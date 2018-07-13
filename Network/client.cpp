#include "client.h"

Client::Client(const shared_ptr<recursive_mutex> &dbMutex, const shared_ptr<recursive_mutex>& netMutex,
               const shared_ptr<mutex> &pubMutex, const shared_ptr<vector<string>> &pk,
               const shared_ptr<mutex>& testMutex, const shared_ptr<vector<string>> &testpk)
    :networkMutex(netMutex),
    myChain(dbMutex, shared_ptr<vector<Transaction>> (new vector<Transaction>),
            shared_ptr<recursive_mutex> (new recursive_mutex)),
    pkMutex(pubMutex), publicKeys(pk), testModeMutex(testMutex), testModePublicKeys(testpk), miner(nullptr), transaction(nullptr)
{
    qRegisterMetaType<Block>("Block");
    peerManager = new PeerManager(this);
    peerManager->setServerPort(server.serverPort());
    peerManager->startBroadcasting();
    threadStopped = true;
    transactionThreadStopped = true;
    createKeypair();

    QObject::connect(peerManager, SIGNAL(newConnection(Connection*)),
                     this, SLOT(newConnection(Connection*)));
    QObject::connect(&server, SIGNAL(newConnection(Connection*)),
                     this, SLOT(newConnection(Connection*)));

    QObject::connect(this, SIGNAL(sendBlockSignal(Block*)),this, SLOT(sendBlock(Block*)));
    QObject::connect(this, SIGNAL(sendTransactionSignal(QString)),this, SLOT(sendTransaction(QString)));


    QTimer::singleShot(4*1000,this, SLOT(getChainFromNetwork()));
    //QTimer::singleShot(4*1000,this, SLOT(getTestingNetworkParticipants()));

}

Client::~Client()
{
    transactionThreadRun = false;
    threadRun = false;
    if (miner->joinable())
    {
        miner->join();
    }
    if(transaction->joinable())
    {
        transaction->join();
    }
}
//const string &public_key
void Client::startTest()
{
    int randomPkInt, sendVal, randomTimeWait, intTime;
    string hash, randompk;
    int currentBlock = myChain.getLatestBlockIndex();
    int balance = myChain.getBalance(getPublicBkey());
    while (transactionThreadRun)
    {
        if (currentBlock < myChain.getLatestBlockIndex())
        {
            balance = myChain.getBalance(getPublicBkey()) - myChain.getMySendTransactionValueFromMempool();
        }
        srand (time (NULL));
        intTime = time(NULL);
        if (balance > 0)
        {
            sendVal = (rand () % balance + 1);
            testModeMutex->lock();
            if (testModePublicKeys->size() > 0)
            {
                balance -= sendVal;
                randomPkInt = (rand()%(testModePublicKeys->size()));
                randompk = testModePublicKeys->at(randomPkInt);
                hash = signMsg(randompk, getPublicBkey(), sendVal);
                myChain.newTransaction(getPublicBkey(),randompk,hash,sendVal,intTime);
                QString transactionString = QString::fromStdString(getPublicBkey()) + "," + QString::fromStdString(randompk) + "," + QString::fromStdString(hash) + "," + QString::number(sendVal) + "," + QString::number(intTime);

                emit sendTransactionSignal(transactionString);
            }
            testModeMutex->unlock();

        }

        randomTimeWait = (rand () % (ROUND_TIME) + (ROUND_TIME / 3));

        usleep(randomTimeWait * 1000000);



    }
    cout << "thread terminated" << endl;
    transactionThreadStopped = true;
    emit startTransactionSignal();
}
void Client::startTransactionThread()
{
    if (transactionThreadStopped)
    {
        if (transaction != nullptr)
        {
            transaction->join();
            delete transaction;
            emit startTransactionSignal();
        }
        transactionThreadRun = true;
        cout << "starting the miner thread" << endl;
        transaction = new std::thread(&Client::startTest, this);
        transactionThreadStopped = false;
    } else {
        cout << "thread is already running" << endl;
    }
}

void Client::stopTransactionThread()
{
    transactionThreadRun = false;
    emit stopTransactionSignal();
}

/**
 * @brief Client::handleTransactionCommandWithParams
 * splits the received string
 * executes a local transaction creation
 * sends the Qstring to all known connected peers
 * @param message string with transactionparams to be send
 */
void Client::handleTransactionCommandWithParams(const QString &message)
{
    cout << "sending transaction" << endl;
    QStringList paramsList = message.split(",");

    // local execution
    time_t timestamp = time(NULL);
    int intTime = timestamp;
    string hash = signMsg(paramsList.at(0).toStdString(), getPublicBkey(), paramsList.at(1).toInt());
    myChain.newTransaction(getPublicBkey(),
                           paramsList.at(0).toStdString(),
                           hash,
                           paramsList.at(1).toInt(), timestamp);

    QString transactionString = QString::fromStdString(getPublicBkey()) + "," + paramsList.at(0) +
            "," + QString::fromStdString(hash) + "," + paramsList.at(1) + "," + QString::number(intTime);

    QList<Connection *> connections = peers.values();
    foreach (Connection *connection, connections)
    {
        connection->sendTransaction(transactionString);
    }
}
/**
 * @brief Client::handleBlockCommand
 * executes a local block creation
 * gets the latest block and splits its components
 * creates a string with the block components
 * sends the string to all connected peers
 */
void Client::handleBlockCommand()
{
    setlocale(LC_NUMERIC, "en_US.UTF-8");
    time_t oldTime = myChain.getLatestBlock().getTimestamp();
    Block prevBlock = myChain.getLatestBlock();
    Block* newBlock = myChain.buildNewBlock(getPublicBkey(), prevBlock);
    int numOfRunsWithSameIndex = 0;
    int previousIndex;
    while (threadRun)
    {
        previousIndex = newBlock->getIndex();
        //check if the time is up. If so create new Block
        if (time(0) > oldTime + ROUND_TIME)
        {
            prevBlock = myChain.getLatestBlock();
            newBlock = myChain.buildNewBlock(getPublicBkey(), prevBlock);
            oldTime = myChain.getLatestBlock().getTimestamp();
            cout << "creating new Block with index " << newBlock->getIndex() << endl;

        }
        //avoid getting stuck with wrong chain
        if (previousIndex == newBlock->getIndex())
        {
            numOfRunsWithSameIndex++;
            if (numOfRunsWithSameIndex >= 5)
            {
                executeverifyBlockchain();
                numOfRunsWithSameIndex = 0;
            }
        }
        else
        {
            numOfRunsWithSameIndex = 0;
        }

        myChain.ProofOfLuck(*newBlock);
	cout << "hash from the new block: " << newBlock->getHash() <<endl;
        int forkID = 0;
        emit updateLNSignal(newBlock->getLn());
        if (myChain.isLuckierBlock(*newBlock))
        {

            int requestedBlock = myChain.handleBlock(*newBlock, &forkID);
            if (requestedBlock > 0)
            {
                myChain.handleBlock(prevBlock, &forkID);
            }
            if (requestedBlock == 0)
            {
                Block* sendingBlock = new Block(*newBlock);
                //memcpy(sendingBlock, newBlock, sizeof(Block));

                emit sendBlockSignal(sendingBlock);
            }
        }
        else
        {
            cout << "not lucky " << newBlock->getLn() << endl;
        }

    }
    cout << "thread terminated" << endl;
    threadStopped = true;
    emit startMiningSignal();
}

void Client::startMiningThread()
{
    if (threadStopped)
    {
        if (miner != nullptr)
        {
            miner->join();
            delete miner;

        }
        threadRun = true;
        cout << "starting the miner thread" << endl;
        miner = new std::thread(&Client::handleBlockCommand, this);
        threadStopped = false;
    } else {
        cout << "thread is already running" << endl;
    }
}

void Client::stopMiningThread()
{
      threadRun = false;
      emit stopMiningSignal();
}


/**
 * @brief Client::handleReceivedBlock
 * handle Block received from the network
 * TODO: appendBlock needs to be replaced by handleBlock
 * @param receivedBlock to append
 */
void Client::handleReceivedBlock(forkBlock receivedBlockStruct)
{
//    int forkID = 0;
    Block* receivedBlock = receivedBlockStruct.block;
    int forkID = receivedBlockStruct.forkID;
    Connection *connection = qobject_cast<Connection *>(sender());
    cout << "starting receive procedure with block " << receivedBlock->getIndex() << endl;

    networkMutex->lock();
    int requestedBlock = myChain.handleBlock(*receivedBlock, &forkID);
    networkMutex->unlock();
//    cout << "requestedBlock return: " << requestedBlock << " forkID: " << forkID << endl;
    if (requestedBlock > 0)
    {

        cout << "request chain from network" << endl;
        connection->sendBlockRequest(receivedBlock->getIndex() - 1, forkID);
    }
    else if (requestedBlock == -2) //invalid Block received, the sending peer should check his database
    {
        connection->sendCheckBlockchain();
    }
    else
    {
        if (requestedBlock == 0)
            emit printChainSignal();
    }
}

void Client::handleReceivedPublicKey(string receivedPublicKey)
{
    vector<string> publicKeysTemp = getPublicKeys();

    for (unsigned int i = 0; i < publicKeysTemp.size(); i++)
    {
        if (publicKeysTemp.at(i).compare(receivedPublicKey) == 0)
        {
            return; // already exists
        }
    }

    publicKeysTemp.push_back(receivedPublicKey);
    setPublicKeys(publicKeysTemp);
    emit updateReceiverComboBox();
}

/**
 * @brief Client::addReceivedTransaction
 * creates a new transaction
 * @param sender of the transaction
 * @param receiver of the transaction
 * @param hash of the transaction
 * @param value of the transaction
 */
void Client::addReceivedTransaction(string sender, string receiver, string hash, int value, int intTime)
{
    time_t timestamp = intTime;
    cout << "transaction received" << endl;
    myChain.newTransaction(sender,receiver,hash,value, timestamp);
}
/**
 * @brief Client::executeProofOfLuck
 * local proofOfWork execution
 */
void Client::executeProofOfLuck()
{
    //myChain.ProofOfLuck(getPublicBkey());
}

/**
 * @brief Client::executeBalancePrinting
 * prints the balance of a public key
 * @param pk public key of the balance to be printed
 */
int Client::executeBalancePrinting(string pk)
{
//    cout << "Balance of " << pk << ": " << myChain.getBalance(pk) << endl;
//    cout << endl;
    return myChain.getBalance(pk);
}

QString Client::executePrintChain(bool detailed)
{
    return myChain.printChain(detailed);
}

vector<string> Client::getAllParticipants()
{
//    vector<string> participants = myChain.getAllParticipants();
//    cout << participants.size() << endl;
    return myChain.getAllParticipants();
}

/**
 * @brief Client::executePrintMempool
 * prints the current mempool
 */
void Client::executePrintMempool()
{
    myChain.printMempool();
}

void Client::executePrintKeys()
{
    vector<string> knownPublicKeys = getPublicKeys();
    cout << "This are the currently known Recipient(s):" << endl;
    for (unsigned int i = 0; i < knownPublicKeys.size(); i++)
    {
        cout << "Receiver " << i + 1 << ": " << knownPublicKeys.at(i)  << endl;
    }
}

bool Client::executeverifyBlockchain()
{
    myChain.checkDatabase();
    return true;
}

void Client::executeHistoryPrinting()
{
    myChain.getMyTransactions();
}

int Client::executeGetLatestBlockIndex()
{
    return myChain.getLatestBlockIndex();
}

/**
 * @brief Client::address
 * returns ip addreses
 * @return array with addresses
 */
QString Client::address() const
{
//    qDebug() << Q_FUNC_INFO;
    return peerManager->getAddress();
}


/**
 * @brief Client::hasConnection
 * checks if the connection already exists in this scope
 * @param senderIp to the senderPort
 * @param senderPort to the senderip
 * @return true if connection already exists, false otherwise
 */
bool Client::hasConnection(const QHostAddress &senderIp, int senderPort) const
{
//    qDebug() << Q_FUNC_INFO;
    if (senderPort == -1)
        return peers.contains(senderIp);

    if (!peers.contains(senderIp))
        return false;

    QList<Connection *> connections = peers.values(senderIp);
    foreach (Connection *connection, connections) {
        if (connection->peerPort() == senderPort)
            return true;
    }

    return false;
}
/**
 * @brief Client::newConnectionmemcpymemcpy
 * connects signals to the newConnection
 * @param connection pointer to the created connection
 */
void Client::newConnection(Connection *connection)
{
    cout << "Network: Connected to a peer." << endl;
    connect(connection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(connection, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(connection, SIGNAL(readyForUse()), this, SLOT(readyForUse()));
    connect(connection, SIGNAL(addReceivedTransaction(string,string,string,int,int)), this, SLOT(addReceivedTransaction(string,string,string,int,int)));
    connect(connection, SIGNAL(handleReceivedBlock(forkBlock)), this, SLOT(handleReceivedBlock(forkBlock)));
    connect(connection, SIGNAL(sendBlockResponseWithBlock(int, int)), this, SLOT(sendBlockResponseWithBlock(int, int)));

    connect(connection, SIGNAL(handlePublicKey(string)),this, SLOT(handleReceivedPublicKey(string)));
    connect(connection, SIGNAL(removePublicKeyFromTestNetwork(string)), this, SLOT(removePublicKeyFromTestNetwork(string)));
    connect(connection, SIGNAL(addPublicKeyFromTestNetwork(string)), this, SLOT(addPublicKeyFromTestNetwork(string)));
    connect(connection, SIGNAL(sendAllKnownParticipants()), this, SLOT(sendAllKnownTestParticipants()));
    connect(connection, SIGNAL(checkDatabase()), this, SLOT(checkDatabase())); // TODO: INVALID BLOCK CHECKING
}
/**
 * @brief Client::readyForUse
 * signals that the connection is ready for use
 */
void Client::readyForUse()
{
    Connection *connection = qobject_cast<Connection *>(sender());
    if (!connection || hasConnection(connection->peerAddress(),connection->peerPort()))
    {
        return;
    }
    peers.insert(connection->peerAddress(), connection);
    QString peer = connection->address();
    if (!peer.isEmpty())
    {
        cout << "public key for this new address" << connection->publicKeyToAddress << endl;
        emit newPeer(peer);
    }
}
/**
 * @brief Client::disconnected
 * received the disconnected signal and removes the connection
 */
void Client::disconnected()
{
    Connection *connection = qobject_cast<Connection *>(sender());
    if (connection)
        removeConnection(connection);
}
/**
 * @brief Client::connectionError
 * could not connect and removes the connection
 * @param SocketError
 */
void Client::connectionError(QAbstractSocket::SocketError /* socketError */)
{
    if (Connection *connection = qobject_cast<Connection *>(sender()))
        removeConnection(connection);
}
/**
 * @brief Client::removeConnection
 * removes the connection from the peers array
 * @param connection to be removed
 */
void Client::removeConnection(Connection *connection)
{
    if (peers.contains(connection->peerAddress())) {
        peers.remove(connection->peerAddress());
        QString peer = connection->address();
        if (!peer.isEmpty())
        {
//            cout << "remove public key for this address" << connection->publicKeyToAddress << endl;
            removePublicKeyFromTestNetwork(connection->publicKeyToAddress);
            emit peerLeft(peer); // NOT USED
        }
    }
    connection->deleteLater();
}

vector<string> Client::getPublicKeys() const
{
    //vector<string>* temp = (vector<string>*) malloc(sizeof(publicKeys));
    //vector<string>* temp = new vector<string>;

    pkMutex->lock();
   // memcpy(temp, &(*publicKeys->begin()), sizeof(publicKeys));
    //cout << "memcompare " << memcmp(temp, &publicKeys, sizeof(publicKeys)) << endl;
    vector<string> temp = *publicKeys;
    pkMutex->unlock();
    return temp;
}

void Client::setPublicKeys(const vector<string> &value)
{
    pkMutex->lock();
    publicKeys->clear();
    foreach (string temp, value)
    {
        publicKeys->push_back(temp);
    }
    pkMutex->unlock();
}

vector<Transaction> Client::getMyTransactions()
{
    return myChain.getMyTransactions();
}

void Client::addToList()
{
    QList<Connection *> connections = peers.values();
    cout << "addtolist." << endl;

    foreach (Connection *connection, connections)
    {
        connection->sendPublicKeyForTestModeAdd();
    }

}

void Client::removeFromList()
{
    QList<Connection *> connections = peers.values();
    cout << "removefromlist." << endl;

    foreach (Connection *connection, connections)
    {
        connection->sendPublicKeyForTestModeRemove();
    }
}
void Client::checkDatabase()
{
    myChain.checkDatabase();
}

void Client::sendBlockResponseWithBlock(int requestedBlockId, int forkID)
{
    Connection *senderConnection = qobject_cast<Connection*>(sender());
    
    Block requestedBlock = (!requestedBlockId) ? myChain.getLatestBlock() : myChain.getBlock(requestedBlockId, 0);

    QString blockAsQString = HelperFunctions::parseBlockToQString(requestedBlock, forkID);

    senderConnection->sendBlockResponse(blockAsQString);

}

void Client::sendBlock(Block* latestBlock)
{
    QList<Connection *> connections = peers.values();
    cout << "This Block is send: " << latestBlock->getIndex() << endl;
    QString blockAsQString = HelperFunctions::parseBlockToQString(*latestBlock, 0);
    cout << qPrintable(blockAsQString) << endl;
    //networkMutex->lock();

    foreach (Connection *connection, connections)
    {
	
        connection->sendBlock(blockAsQString);

    }

    emit printChainSignal();
    cout << "deleting" << endl;
    delete latestBlock;
    cout << "deleted" << endl;
}


void Client::sendTransaction(QString newTransaction)
{
    QList<Connection *> connections = peers.values();
    foreach (Connection *connection, connections)
    {
        connection->sendTransaction(newTransaction);
    }
}

void Client::printChainSlot()
{
    emit printChainSignal();
}

void Client::removePublicKeyFromTestNetwork(string pbkey)
{
    testModeMutex->lock();
    for (unsigned int i = 0; i < testModePublicKeys->size(); i++)
    {
        if (testModePublicKeys->at(i).compare(pbkey) == 0)
        {
            testModePublicKeys->erase(testModePublicKeys->begin() + i);
        }
    }
    testModeMutex->unlock();

}
void Client::addPublicKeyFromTestNetwork(string pbkey)
{
    testModeMutex->lock();
    testModePublicKeys->push_back(pbkey);
    testModeMutex->unlock();
}

void Client::sendAllKnownTestParticipants()
{
    Connection *senderConnection = qobject_cast<Connection*>(sender());
    testModeMutex->lock();
    for (unsigned int i = 0; i < testModePublicKeys->size(); i++)
    {
        senderConnection->sendPublicKeyForTestModeAdd(testModePublicKeys->at(i));
    }
    testModeMutex->unlock();
}

/**
 * @brief
 * sends a BlockRequest to the first known connection
 *
 */
void Client::getChainFromNetwork()
{
    QList<Connection *> connections = peers.values();
    if (connections.size() > 0)
    {
        connections.at(0)->sendBlockRequest();
    }
}

void Client::getTestingNetworkParticipants()
{
    QList<Connection *> connections = peers.values();
    if (connections.size() > 0)
    {
        connections.at(0)->sendTestingNetworkParticipantsRequest();
    }
}

void Client::requestPublicKeysFromNetwork()
{
    QList<Connection *> connections = peers.values();

    foreach (Connection *connection, connections)
    {
        connection->sendPublicKeyRequest();
    }
}

bool Client::closeDb() 
{
    return myChain.closeDb();
}


/**
 * @brief
 * creates a QStringList by reading a local file
 * @param location of the publickey file
 */
QStringList Client::readPublicKeysFromFile(const QString location)
{
    QFile file(location);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Could not open file: " << file.errorString();
    }

    QTextStream in(&file);

    if(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList publicKeys = line.split(",");
        // TODO: Remove after testing
        for(const auto& publicKey : publicKeys)
        {
            qDebug() << publicKey;
        }
        return publicKeys;

    }
    return QStringList();
}

#include "blockchain.hpp"
/**
 * @brief Blockchain::Blockchain
 * standard copy constructor + initialize chain
 */
Blockchain::Blockchain()
    :db()
{
    initializeChain();
}

/**
 * @brief Blockchain::Blockchain
 * passes the mutex to the database
 * and sets the mutex and shared variable
 * for the mempool
 * @param sharedMutex passed to database
 * @param sharedMempool
 * @param memMutex mutex for the mempool
 */
Blockchain::Blockchain(const shared_ptr<recursive_mutex> &sharedMutex,
                       const shared_ptr<vector<Transaction> > &sharedMempool,
                       const shared_ptr<recursive_mutex>& memMutex)
    :db(sharedMutex, shared_ptr<int> (new int(0))), mempool(sharedMempool), mempoolMutex(memMutex)
{
    cout << "starting init" << endl;
    initializeChain();
}

/**
 * @brief Blockchain::initializeChain
 * initializes the chain.
 * if no database is created yet, a new one will be created and
 * the genesis block will be created
 */
void Blockchain::initializeChain()
{
    setlocale(LC_NUMERIC, "en_US.UTF-8");
    if (!db.blockchainInitialized()) {
//        time_t curTime;
//        time(&curTime);
        string prevHash = "0";
        SHA1 transHash;
        vector<Transaction> tempTrans = {};
        transHash.update("GenesisMiner");
        Transaction tempDat = Transaction("","GenesisMiner", MINER_REWARD, transHash.final(), 1523013300);
        tempTrans.push_back(tempDat);
        MerkleTree merkle = MerkleTree(tempTrans);
        string merkleHash = merkle.getMerkleHash();
        SHA1 checksum;
        checksum.update(prevHash);
        checksum.update(to_string(1.0));
        checksum.update(merkleHash);
        checksum.update(to_string(1523013300));
        Block genesisBlock = Block(prevHash, checksum.final(), merkleHash, 1523013300, tempTrans, 1);
        genesisBlock.setLn(1.0);
        genesisBlock.setIndex(1);
        genesisBlock.setCertificate("tKLoBwrOx/1ROWO77UduTA15mO/vml67tU5ifPv3hzF2URujdSFM8AVOoGSmESIQMoZeDRQaVpRxSOpGOlFfUT9n");
        db.appendBlock(genesisBlock);
    }
//    cout << "init done " << endl;
}

bool Blockchain::closeDb()
{
    return db.closeDb();
}

vector<Transaction> Blockchain::getMyTransactions()
{
    return db.getMyTransactions();
}

int Blockchain::getLatestBlockIndex()
{
    return db.getLastBlockIndex(0);
}

int Blockchain::getMySendTransactionValueFromMempool()
{
    int value = 0;
    string pk = getPublicBkey();
    mempoolMutex->lock();
    for (unsigned int i = 0; i < mempool->size(); i++)
    {
        if (mempool->at(i).getSender().compare(pk) == 0)
        {
            value += mempool->at(i).getValue();
        }
    }
    mempoolMutex->unlock();
    return value;
}

void Blockchain::checkDatabase()
{
    db.iterateOverBlocks(0);
    while (db.nextBlock())
    {
        Block temp = db.getBlock();
        if (!verifyBlock(temp, 0))
        {
            db.cleanUpDBFromIndex(temp.getIndex());
            break;
        }
    }
    db.stopBlockIterator();
}

/**
 * @brief Blockchain::verifyBlock
 * verifies if a block is valid.
 * TODO: more proofing (encryption and so on)
 * @param block that needs to be verified
 * @return true if it is valid, false else
 */
bool Blockchain::verifyBlock(Block block, int forkID)
{

    int minerTransactionValue = 0;
    int in = 0, out = 0, coinbase = 0;
    MerkleTree merkle = MerkleTree(block.getTransaction());
    vector<string> input;
    bool multInput;
    string merkleHash = merkle.getMerkleHash();
    if(block.getMerkleHash().compare(merkleHash) != 0)
    {
        cout << "Wrong merkle hash in block " << block.getIndex() << endl;
        cout << "should be " << merkleHash << " but is " << block.getMerkleHash() << endl;
        cout << block.print();
        return false;
    }

    //checking previous hash
    int previousCompareForkID = 0;
    if (forkID != 0)
    {
        if (db.getFirstForkBlockIndex(forkID) != block.getIndex())
        {
            previousCompareForkID = forkID;
        }
    }
    if (block.getIndex() > 1 &&
        block.getPreviousHash().compare(getBlock(block.getIndex() - 1, previousCompareForkID).getHash()) != 0)
    {
        cout << "wrong previous hash at block " << block.getIndex() << endl;
        cout << " is " << block.getPreviousHash() << " but must be " << getBlock(block.getIndex() - 1, previousCompareForkID).getHash() << endl;
        return false;
    }
    if (block.getTimestamp() <= getBlock(block.getIndex() - 1, previousCompareForkID).getTimestamp()
            || block.getTimestamp() > time(0))
    {
        cout << "wrong timestamp" << endl;
        return false;
    }

    if(!block.verifyHash())
    {
        cout << "Hash " << block.buildHash(block.getLn()) << " of Block " << block.getIndex() << " << not valid!" << endl;
        cout << "It is " << block.getHash() << endl;
        return false;
    }
    for (int i = 0; i < (signed int)block.getTransaction().size(); i++)
    {
        if (db.existsTransaction(block.getTransaction().at(i).getHash(), block.getIndex(), forkID))
        {
            return false;
        }
        if (!block.getTransaction().at(i).verifyTransaction())
        {
            //check if it is the coinbase Tx
            if (!(block.getTransaction().at(i).getInput().size() == 0) || block.getTransaction().at(i).getValue() > MINER_REWARD)
            {
                cout << "Transaction " << block.getTransaction().at(i).print() << " not valid" << endl;
                return false;
            }
            coinbase++;
            minerTransactionValue = block.getTransaction().at(i).getValue();
            //cout << "minerTrans " << minerTransactionValue << endl;
            //check if more then one coinbase tx
            if (coinbase > 1)
            {
                cout << "more then 1 coinbase transaction" << endl;
                return false;
            }
        } else {
            if (getBalance(block.getTransaction().at(i).getSender(), block.getIndex() - 1, forkID) < block.getTransaction().at(i).getValue())
            {
                block.getTransaction().at(i).print();
                cout << "invalid transaction in Block " << block.getIndex() << " in fork " << forkID << ". Sender has not enough Coins" << endl;
                cout << "Has Balance " << getBalance(block.getTransaction().at(i).getSender(), block.getIndex() - 1, forkID) << " sends " << block.getTransaction().at(i).getValue() << endl;
                return false;
            }
            if (block.getTransaction().at(i).getValue() <= 0)
            {
                cout << "invalid Transaction in Block. Value is not positiv" << endl;
                return false;
            }
        }
//        cout << "out " << block.getTransaction().at(i).getValue();
        out += block.getTransaction().at(i).getValue();
    }
    //cout << "in " << in << " out  " << out << endl;
    in = db.getInputSumOfBlock(block.getIndex(), forkID);
    if (in + minerTransactionValue !=  out)
    {
        cout << "Block " << block.print() << " is invalid because: in + reward = " << in + minerTransactionValue << " out = " << out << endl;
        return false;
    }
    //cout << "cert" << endl;

    if (!proofCertificate(block))
    {
        cout << "Block has false certificate" << endl;
        return false;
    }
    return true;
}

void Blockchain::ProofOfLuck(Block& block)
{
    double ln;
    string cert;
    addProof(block.getMerkleHash(), block.getPreviousHash(), &ln, &cert);
    cout.precision(dbl::max_digits10);
    cout << "LN: " << fixed << ln << endl;
    block.setLn(ln);
    block.setCertificate(cert);
    block.makeHash();
        //appendBlock(block);
}

/**
 * @brief Blockchain::isLuckierBlock
 * checks if the given block merged with the main chain
 * is luckier then the mainchain
 * @param block to be checked
 * @return true if luckier, false if mainchain is luckier
 */
bool Blockchain::isLuckierBlock(Block block)
{
    return db.isLuckierChain(block.getIndex(), db.getLastBlockIndex(0), block.getLn(), 0);
}

/**
 * @brief Blockchain::buildNewBlock
 * builds a new block, containing all mempool transactions
 * and the miner transaction to the given public key
 * @param pk receiver of the miner reward
 * @return the block, that was build
 */
Block* Blockchain::buildNewBlock(string pk, Block prev)
{
    Transaction minerReward;
    minerReward.setMinerTransaction(pk, MINER_REWARD);
    vector<Transaction> blockTransactions;
    blockTransactions = putTxInBlock();
    blockTransactions.push_back(minerReward);
    MerkleTree merkle = MerkleTree(blockTransactions);
    Block* tempBlock = new Block(prev.getHash(), "", merkle.getMerkleHash(), time(NULL), blockTransactions, prev.getIndex() + 1);
    return tempBlock;
}

/**
 * TODO: implement
 * @brief proofCertificate
 * @param cert
 * @return
 */
bool proofCertificate(Block block)
{
    cout.precision(dbl::max_digits10);
    cout << "LN: " << fixed << block.getLn() << endl;
    if (block.getPreviousHash() != "" && block.getIndex() != 1) {
        return verifyProof(block.getMerkleHash(), block.getPreviousHash(), block.getLn(), block.getCertificate());
    }
    return TRUE;
}

/**
 * @brief Blockchain::checkTempChain
 * checks if the given fork is valid.
 * for this it checks if every block is valid
 * and if the chain is luckier then then main chain
 * @return true if valid, false else
 */
int Blockchain::checkTempChain(int forkID)
{

    db.iterateOverForkBlocks(forkID);
    double forkLN = 0.0;
    Block temp;
    int firstForkBlockIndex = 0;
    //getting forkLN
    while (db.nextBlock()) {
        temp = db.getBlock();
        if (firstForkBlockIndex == 0)
        {
            firstForkBlockIndex = temp.getIndex();
        }
        if (!verifyBlock(temp, forkID))
        {
            cout << "invalid Block returned from checkTempChain" << endl;
            db.stopForkBlockIterator();
            return -2;
        }
        forkLN += temp.getLn();
    }
    db.stopForkBlockIterator();
    return db.isLuckierChain(firstForkBlockIndex, db.getLastBlockIndex(0), forkLN, forkID) - 1;
}
/**
 * @brief Blockchain::getBlock
 * get the block at a certain index
 * TODO: handle invalid index
 * @param index of the block
 * @return block at index
 */
Block Blockchain::getBlock(int index, int forkID)
{
    return db.getBlock(index, forkID);
}

/**
 * @brief Blockchain::handleBlock
 * handles the process after receiving a block.
 * this function will be called recursive from the receiver function.
 * in case there are missing blocks or different previous blocks
 * the function will return the index of the needed block
 * @param block received block
 * @param index expected index of the block
 * @return 0 if successful, index of the block if some other blocks are needed
 *          -1 if the block is invalid
 */
int Blockchain::handleBlock(Block& block, int* forkID)
{
    int retVal = -1;
    if ((block.getIndex() - 1 <= db.getLastBlockIndex(0)) &&
         block.getPreviousHash().compare(this->getBlock(block.getIndex() - 1, 0).getHash()) == 0)
    {
        cout << "appending block" << endl;
        if (*forkID == 0)
        {
            *forkID = db.createFork();
            cout << "created fork" << *forkID << endl;
        }
        db.addToFork(block, *forkID);
        retVal = checkTempChain(*forkID);
        if (retVal == 0)
        {
            cout << "temp chain valid-> applying fork now with index " << block.getIndex() << endl;
            mempoolUpdate(*forkID);
            db.applyFork(*forkID);
            return 0;
        }
    }
    else
    {
        cout << "missing block" << endl;
        if (proofCertificate(block))
        {
            if (*forkID == 0)
            {
                *forkID = db.createFork();
                cout << "created fork " << *forkID << endl;
            }
            db.addToFork(block, *forkID);
            return block.getIndex() - 1;
        }
        else
        {
cout << "wrong certificate" << endl;
            retVal = -2;    //send Reset DB
        }
    }
    cout << "invalid block " << block.getIndex() << " returned from handleBlock with returnValue " << retVal << endl;
//    block.print(); // invalid Block might return error
    *forkID ? db.deleteFork(*forkID) : false;
    return retVal;  //not a valid chain
}

bool Blockchain::verifyBlockchain()
{

    db.iterateOverBlocks(0);
    while(db.nextBlock())
    {
        if (!verifyBlock(db.getBlock(), 0))
        {
            db.stopBlockIterator();
            return false;
        }
    }
    db.stopBlockIterator();
    return true;
}

vector<string> Blockchain::getAllParticipants()
{
    return db.getAllParticipants(getPublicBkey());
}


/**
 * @brief Blockchain::newTransaction
 * A user sends some coins. We will just save the data here and not verify it
 * @param send  public key from sender
 * @param rec   public key from receiver
 * @param hash  the hash from this transaction. It should been signed with
 *              the private key of the sender
 * @param val   the value that needs to be send
 */
void Blockchain::newTransaction(string send, string rec, string hash, int val, time_t timestamp)
{
    Transaction temp = Transaction(send, rec, val, hash, timestamp);
    //check here if the transaction is in the currentTransactions already
    if(checkDuplicateTransition(hash))
    {
        mempoolMutex->lock();
        mempool->push_back(temp);
        mempoolMutex->unlock();

    }
    //cout << "new Transacion " << endl << temp.print();
}




/**
 * @brief Blockchain::putTxInBlock
 * puts the transactions from the mempool into a block
 * it checks for every transaction if it is valid
 * @return List of valid transactions to be put in a block
 */
vector<Transaction> Blockchain::putTxInBlock()
{
    Transaction temp;
    vector<Transaction> trans;
    vector<Utxo_help> utxo;
    Utxo_change tempChange;
    vector<Utxo_change> changes;
    vector<string> used_transactions;
    vector<string> input;
    int changeNum;      //defines if some changeinputs are used, if not -1, else elementnumber of the changesvector
    int sum;
    mempoolMutex->lock();
    for (unsigned int i = 0; i < mempool->size(); i++)
    {
        changeNum = -1;
        sum = 0;

        if (verifySignature(mempool->at(i).getHash(), mempool->at(i).getSender(), mempool->at(i).getRecipient(), mempool->at(i).getValue(), mempool->at(i).getTimestamp())
                && mempool->at(i).getValue() > 0 && db.getTransactionValueByHash(mempool->at(i).getHash(), 0) == 0) //not in bc already
        {
            for (unsigned int c = 0; c < changes.size(); c++)
            {
                if (changes.at(c).key.compare(mempool->at(i).getSender()) == 0)
                {
                    sum += changes.at(c).change;
                    changeNum = c;
                    break;
                }
            }
            if (sum >= mempool->at(i).getValue())     //using only the change inputs
            {
                changes.at(changeNum).change -= mempool->at(i).getValue();
                temp = mempool->at(i);
                temp.add_Input(changes.at(changeNum).input);
                trans.push_back(temp);
                if (changes.at(changeNum).change == 0)   //delete the change if it is 0
                {
                    changes.erase(changes.begin() + changeNum);
                }
                continue;
            }
            utxo = getUTXO(mempool->at(i).getSender(),db.getLastBlockIndex(0), 0);

            for (unsigned int j = 0; j < utxo.size(); j++)
            {
                for (unsigned int l = 0; l < used_transactions.size(); l++)
                {
                    if (used_transactions.at(l).compare(utxo.at(j).hash) == 0)
                    {
                        goto cnt;
                    }
                }
                sum += utxo.at(j).value;
                cnt:;
                input.push_back(utxo.at(j).hash);
                if(sum >= mempool->at(i).getValue())
                    break;
            }
            if(sum < mempool->at(i).getValue())
            {
                input.clear();
                cout << "sender " << mempool->at(i).getSender() << " has not enough money. sends " << mempool->at(i).getValue() << " has " << sum << endl;
                mempool->erase(mempool->begin()+i);
                i--;
                continue;
            }
            temp = mempool->at(i);
            temp.add_Input(input);

            for (unsigned int i = 0; i < input.size(); i++)
            {
                used_transactions.push_back(input.at(i));
            }
            if (changeNum > -1)
            {
                temp.add_Input(changes.at(changeNum).input);
                changes.erase(changes.begin() + changeNum);
            }
            if (temp.verifyTransaction())
            {
                trans.push_back(temp);
                if(sum - mempool->at(i).getValue() != 0)
                {
                    tempChange.input = input;
                    tempChange.change = sum - mempool->at(i).getValue();
                    tempChange.key = mempool->at(i).getSender();
                    changes.push_back(tempChange);
                }
            } else {
                cout << "couldnt verify the transaction with sender " << mempool->at(i).getSender() << endl;
                mempool->erase(mempool->begin() + i);
            }
            input.clear();
        }
        else
        {
            cout << "cant put into block  " << endl;
            mempool->erase(mempool->begin() + i);
        }
    }
    mempoolMutex->unlock();

    for(unsigned int i = 0; i < changes.size(); i++)
    {
        temp.add_Input(changes.at(i).input);
        temp.setNumOfInputs(changes.at(i).input.size());
        temp.setRecipient(changes.at(i).key);
        temp.setSender(changes.at(i).key);
        temp.setValue(changes.at(i).change);
        time_t curTime;
        time(&curTime);
        char buff[20];
        SHA1 transHash;                             //TODO: There needs to be a special hash, to sign the coinbase tx
        strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&curTime));
        transHash.update(buff);
        transHash.update(temp.getSender());
        temp.setHash(transHash.final());
        trans.push_back(temp);
    }

    return trans;

}

/**
 * @brief Blockchain::getUTXO
 * Get all Unspent Transaction Outputs from the given key.
 * If the key got a transaction, but spent it already, it is not a UTXO
 * @param key the public key of the receiver from UTXO
 * @return a list of UTXOs
 */
vector<Utxo_help> Blockchain::getUTXO(string key, int blockIndex, int forkID)
{
    return db.getUTXO(blockIndex, key, forkID);

}

/**
 * @brief Blockchain::getBalance
 * returns the Balance of the input key
 * @param key public key of a client
 * @return balance
 */
int Blockchain::getBalance(string key, int blockIndex, int forkID)
{

    return db.getBalance(blockIndex, key, forkID);
}

int Blockchain::getBalance(string key, int forkID)
{
    return getBalance(key, db.getLastBlockIndex(0), forkID);
}

/**
 * @brief Blockchain::printLatestBlock
 * prints out the latest block
 */
void Blockchain::printLatestBlock()
{
    getLatestBlock().print();
}

/**
 * @brief Blockchain::printMempool
 * prints the current mempool
 */
void Blockchain::printMempool()
{
    mempoolMutex->lock();
    cout << "Mempool has " << mempool->size() << " transactions:" << endl;
    for (unsigned int i = 0; i < mempool->size(); i++)
    {
        cout << "Nr. " << i << endl;
        cout << mempool->at(i).print();
    }
    mempoolMutex->unlock();

}




/**
 * @brief Blockchain::printChain
 * prints the log of the chain in the following format:
 * block i:
 * hash: Sender -> Recipient : Value
 * ...
 * block j:...
 */
QString Blockchain::printChain(bool detailed)
{
    cout << "LN sum is " << db.getLNSum() << endl;
    return db.getStringFromBlockchain(detailed);
}

/**
 * @brief Blockchain::checkDuplicateTransition
 * checks if a given transactionhash is in the mempool
 * @param hash of the transaction to check
 * @return true if it is not a duplicate, false else
 */
bool Blockchain::checkDuplicateTransition(string hash)
{
    mempoolMutex->lock();
    for (unsigned int i = 0; i < mempool->size(); i++)
    {
        if(mempool->at(i).getHash().compare(hash) == 0)
        {
            mempoolMutex->unlock();
            return false;
        }
    }
    mempoolMutex->unlock();
    return true;
}

/**
 * @brief Blockchain::mempoolUpdate
 * @param forkID id if the fork, that replaces the main chain
 */
void Blockchain::mempoolUpdate(int forkID)
{
    vector<Transaction> transToAdd = db.getTransactionsFromChain(forkID);
    vector<Transaction> transToDelete = db.getTransactionsFromFork(forkID);
    for (unsigned int t = 0; t < transToAdd.size(); t++)
    {
        //check if the transaction is a coinbase transaction
        if (transToAdd.at(t).getInput().size() != 0 || transToAdd.at(t).getSender().compare("") == 0)
        {
            newTransaction(transToAdd.at(t).getSender(), transToAdd.at(t).getRecipient(), transToAdd.at(t).getHash(),
                           transToAdd.at(t).getValue(), transToAdd.at(t).getTimestamp());
        }
    }
    for (unsigned int t = 0; t < transToDelete.size(); t++)
    {
        mempoolMutex->lock();
        for (unsigned int m = 0; m < mempool->size(); m++)
        {
            if (transToDelete.at(t).getHash().compare(mempool->at(m).getHash()) == 0)
            {
                mempool->erase(mempool->begin() + m);
                break;
            }
        }
        mempoolMutex->unlock();
    }
}

void Blockchain::gdb()
{
    cout << "gdb helper" << endl;
}


/**
 * @brief Blockchain::getLatestBlock
 * @return the latest Block
 */
Block Blockchain::getLatestBlock()
{
    Block block = db.getLatestBlock();
    return block;

}

#include "connection.h"


static const int TransferTimeout = 30 * 1000;
static const int PongTimeout = 60 * 1000;
static const int PingInterval = 5 * 1000;

Connection::Connection(QObject *parent)
: QTcpSocket(parent)
{
//    qDebug() << Q_FUNC_INFO;
greetingMessage = tr("undefined");
cliAddress = tr("unknown");
state = WaitingForGreeting;
currentDataType = Undefined;
numBytesForCurrentDataType = -1;
transferTimerId = 0;
forkID = 0;
isGreetingMessageSent = false;
pingTimer.setInterval(PingInterval);

QObject::connect(this, SIGNAL(readyRead()), this, SLOT(processReadyRead()));
QObject::connect(this, SIGNAL(disconnected()), &pingTimer, SLOT(stop()));
QObject::connect(&pingTimer, SIGNAL(timeout()), this, SLOT(sendPing()));
QObject::connect(this, SIGNAL(connected()),
                 this, SLOT(sendGreetingMessage()));


}
/**
* @brief Connection::address
* returns the cliAddress
* @return QString with the address
*/
QString Connection::address() const
{
return cliAddress;
}
/**
* @brief Connection::setGreetingMessage
* sets the greeting message
* @param message greetingMessage
*/
void Connection::setGreetingMessage(const QString &message)
{
greetingMessage = message;
}
/**
* @brief Connection::sendTransaction
* sends a transaction to the network
* @param params to the corresponding transaction
* @return true if sending was successful
*/
bool Connection::sendTransaction(const QString &params)
{
QByteArray data;
data = "TRANSACTION " + QByteArray::number(params.toUtf8().size()) + SeparatorToken + params.toUtf8();
return write(data) == data.size();
}
/**
* @brief Connection::sendBlock
* sends a block to the network
* @param params to the corresponding block
* @return true if sending was successful
*/
bool Connection::sendBlock(const QString &params)
{
QByteArray data;
data = "BLOCK " + QByteArray::number(params.toUtf8().size()) + SeparatorToken + params.toUtf8();
return write(data) == data.size();
}

/**
* @brief Connection::sendBlockchainRequest
* requests block from single connection
* @param id id of the requested block, default is 0 for the latest
* @return true if sending was successful
*/
bool Connection::sendBlockRequest(int blockID, int forkID)
{
QByteArray data;
QString idAsQString = QString::number(blockID) + "," + QString::number(forkID);
data = "BLOCK_REQUEST " + QByteArray::number(idAsQString.toUtf8().size()) + SeparatorToken + idAsQString.toUtf8();
return write(data) == data.size();
}
bool Connection::sendTestingNetworkParticipantsRequest()
{
    QByteArray data = "TEST_NETWORK_PARTICIPANTS_REQUEST ";
    return write(data) == data.size();
}

/**
* @brief Connection::sendBlockchainRequest
* send the requested block to a single connection
* @return true if sending was successful
*/
bool Connection::sendBlockResponse(const QString &params)
{
    QByteArray data;
    data = "BLOCK_RESPONSE " + QByteArray::number(params.toUtf8().size()) + SeparatorToken + params.toUtf8();
    return write(data) == data.size();
}

bool Connection::sendPublicKeyRequest()
{
    QByteArray data;
    QByteArray pbkey = QString::fromStdString(getPublicBkey()).toUtf8();
    data = "PUBLICKEY_REQUEST " + QByteArray::number(pbkey.size()) + SeparatorToken + pbkey;
    return write(data) == data.size();
}
bool Connection::sendCheckBlockchain()
{
    QByteArray data;
    QByteArray defaultMessage = QString::fromStdString("default").toUtf8();
    data = "CHECK_BLOCKCHAIN "+ QByteArray::number(defaultMessage.size()) + SeparatorToken + defaultMessage;
    return write(data) == data.size();
}

bool Connection::sendPublicKeyResponse()
{
    QByteArray data;
    QByteArray pbkey = QString::fromStdString(getPublicBkey()).toUtf8();
    data = "PUBLICKEY_RESPONSE " + QByteArray::number(pbkey.size()) + SeparatorToken + pbkey;
    return write(data) == data.size();
}

bool Connection::sendPublicKeyForTestModeRemove()
{
    QByteArray data;
    QByteArray pbkey = QString::fromStdString(getPublicBkey()).toUtf8();
    data = "PUBLICKEY_REMOVE " + QByteArray::number(pbkey.size()) + SeparatorToken + pbkey;
    return write(data) == data.size();
}

bool Connection::sendPublicKeyForTestModeAdd(string pbkey)
{
    QByteArray data;
    QByteArray pbkeyAsByteArray = QString::fromStdString(pbkey).toUtf8();
    data = "PUBLICKEY_ADD " + QByteArray::number(pbkeyAsByteArray.size()) + SeparatorToken + pbkeyAsByteArray;
    return write(data) == data.size();
}

void Connection::timerEvent(QTimerEvent *timerEvent)
{
    //    qDebug() << Q_FUNC_INFO;
    if (timerEvent->timerId() == transferTimerId) {
        abort();
        killTimer(transferTimerId);
        transferTimerId = 0;
    }
}
/**
* @brief Connection::processReadyRead
* checks the current state and handles incoming network messages
*/
void Connection::processReadyRead()
{
//    qDebug() << Q_FUNC_INFO;
if (state == WaitingForGreeting) {
    if (!readProtocolHeader())
        return;
    if (currentDataType != Greeting) {
        abort();
        return;
    }
    state = ReadingGreeting;
}

if (state == ReadingGreeting) {
    if (!hasEnoughData())
        return;

    buffer = read(numBytesForCurrentDataType);
    if (buffer.size() != numBytesForCurrentDataType) {
        abort();
        return;
    }

    cliAddress = peerAddress().toString() + ':' + QString::number(peerPort());
    currentDataType = Undefined;
    numBytesForCurrentDataType = 0;
    buffer.clear();

    if (!isValid()) {
        abort();
        return;
    }

    if (!isGreetingMessageSent)
        sendGreetingMessage();

    pingTimer.start();
    pongTime.start();
    state = ReadyForUse;
    emit readyForUse();
}

do {
    if (currentDataType == Undefined) {
        if (!readProtocolHeader())
            return;
    }
    if (!hasEnoughData())
        return;
    processData();
} while (bytesAvailable() > 0);
}

void Connection::sendPing()
{
//    qDebug() << Q_FUNC_INFO;
if (pongTime.elapsed() > PongTimeout) {
    abort();
    return;
}

write("PING 1 p");
}

void Connection::sendGreetingMessage()
{
//    qDebug() << Q_FUNC_INFO;
QByteArray greeting = greetingMessage.toUtf8();

QByteArray data = "GREETING " + QByteArray::number(greeting.size()) + SeparatorToken + greeting;

if (write(data) == data.size())
{
    isGreetingMessageSent = true;
}

sendPublicKeyRequest();
}

/**
* @brief Connection::readDataIntoBuffer
* stripes overhead from the buffer and save it
* @param maxSize to the buffer
* @return actual byte size
*/
int Connection::readDataIntoBuffer(int maxSize)
{
//    qDebug() << Q_FUNC_INFO;
if (maxSize > MaxBufferSize)
    return 0;

int numBytesBeforeRead = buffer.size();
if (numBytesBeforeRead == MaxBufferSize) {
    abort();
    return 0;
}

while (bytesAvailable() > 0 && buffer.size() < maxSize) {
    buffer.append(read(1));
    if (buffer.endsWith(SeparatorToken))
        break;
}
return buffer.size() - numBytesBeforeRead;
}

/**
* @brief Connection::dataLengthForCurrentDataType
*
* @return length of the current data type
*/
int Connection::dataLengthForCurrentDataType()
{
//    qDebug() << Q_FUNC_INFO;
if (bytesAvailable() <= 0 || readDataIntoBuffer() <= 0
        || !buffer.endsWith(SeparatorToken))
    return 0;

buffer.chop(1);
int number = buffer.toInt();
buffer.clear();
return number;
}
/**
* @brief Connection::readProtocolHeader
* sets the currentDataType
* and sets the numBytesForCurrentDataType
* @return true if successful
*/
bool Connection::readProtocolHeader()
{
//    qDebug() << Q_FUNC_INFO;
if (transferTimerId) {
    killTimer(transferTimerId);
    transferTimerId = 0;
}

if (readDataIntoBuffer() <= 0) {
    transferTimerId = startTimer(TransferTimeout);
    return false;
}

if (buffer == "PING ") {
    currentDataType = Ping;
} else if (buffer == "PONG ") {
    currentDataType = Pong;
} else if (buffer == "MESSAGE ") {
    currentDataType = PlainText;
} else if (buffer == "GREETING ") {
    currentDataType = Greeting;
} else if (buffer == "BLOCK ") {
    currentDataType = ReceivedBlock;
} else if (buffer == "TRANSACTION ") {
    currentDataType = ReceivedTransaction;
} else if (buffer == "BLOCK_REQUEST ") {
    currentDataType = BlockRequest;
} else if (buffer == "CHECK_BLOCKCHAIN ") {
    currentDataType = CheckBlockchain;
} else if (buffer == "BLOCK_RESPONSE ") {
    currentDataType = BlockResponse;
} else if (buffer == "PUBLICKEY_REQUEST ") {
    currentDataType = PublicKeyRequest;
} else if (buffer == "PUBLICKEY_RESPONSE ") {
    currentDataType = PublicKeyResponse;
} else if (buffer == "PUBLICKEY_ADD ") {
    currentDataType = PublicKeyForTestModeAdd;    
} else if (buffer == "TEST_NETWORK_PARTICIPANTS_REQUEST ") {
    currentDataType = TestNetworkParticipantsRequest;
} else if (buffer == "PUBLICKEY_REMOVE ") {
    currentDataType = PublicKeyForTestModeRemove;
} else {
    currentDataType = Undefined;
    abort();
    return false;
}

buffer.clear();
numBytesForCurrentDataType = dataLengthForCurrentDataType();
return true;
}
/**
* @brief Connection::hasEnoughData
*
* @return false if available data does not match expected data
*/
bool Connection::hasEnoughData()
{
//    qDebug() << Q_FUNC_INFO;
if (transferTimerId) {
    QObject::killTimer(transferTimerId);
    transferTimerId = 0;
}

if (numBytesForCurrentDataType <= 0)
    numBytesForCurrentDataType = dataLengthForCurrentDataType();

if (bytesAvailable() < numBytesForCurrentDataType
        || numBytesForCurrentDataType <= 0) {
    transferTimerId = startTimer(TransferTimeout);
    return false;
}

return true;
}
/**
* @brief Connection::processData
* checks the currentDataType and emits corresponding signals
* resets attributes
*/
void Connection::processData()
{
//    qDebug() << Q_FUNC_INFO;
buffer = read(numBytesForCurrentDataType);
if (buffer.size() != numBytesForCurrentDataType) {
    abort();
    return;
}

switch (currentDataType) {
case PlainText:
    cout <<  qPrintable(buffer) << endl; //TESTING <- dont delete
    break;
case Ping:
    write("PONG 1 p");
    break;
case Pong:
    pongTime.restart();
    break;
case ReceivedBlock:
    {
        QString paramsFromBuffer = QString::fromUtf8(buffer);

        emit handleReceivedBlock(HelperFunctions::parseQStringToBlock(paramsFromBuffer));
    }
    break;
case ReceivedTransaction:
    {
        QString paramsFromBuffer = QString::fromUtf8(buffer);
        QStringList paramsList = paramsFromBuffer.split(",");
        emit addReceivedTransaction(paramsList.at(0).toStdString(),
                                            paramsList.at(1).toStdString(),
                                            paramsList.at(2).toStdString(),
                                            paramsList.at(3).toInt(),
                                            paramsList.at(4).toInt());
    }
    break;
case Greeting:
    {
    //        cout <<  qPrintable(buffer) << endl;
    }
    break;
case CheckBlockchain:
    {
        emit checkDatabase();
    }
    break;
case BlockRequest:
    {
        QString blockIDAndForkIDFromBuffer = QString::fromUtf8(buffer);
        QStringList paramsList = blockIDAndForkIDFromBuffer.split(",");

        emit sendBlockResponseWithBlock(paramsList.at(0).toInt(),paramsList.at(1).toInt());

    }
    break;
case BlockResponse:
    {
//        cout << qPrintable(buffer) << endl;
        QString paramsFromBuffer = QString::fromUtf8(buffer);

        emit handleReceivedBlock(HelperFunctions::parseQStringToBlock(paramsFromBuffer));
    }
    break;
case PublicKeyRequest:
    {
        sendPublicKeyResponse();
    }
    break;
case PublicKeyResponse:
    {
    // Hat die Antwort auf einen Request bekommen und speichert die Erhaltene
        string publicKeyFromBuffer = QString::fromUtf8(buffer).toStdString();
        publicKeyToAddress = publicKeyFromBuffer;
        emit handlePublicKey(publicKeyFromBuffer);
    }
    break;

case PublicKeyForTestModeAdd:
    {
    // Key hinzufügen
        string publicKeyFromBuffer = QString::fromUtf8(buffer).toStdString();
        emit addPublicKeyFromTestNetwork(publicKeyFromBuffer);
    }
    break;
case PublicKeyForTestModeRemove:
    {
    // Key löschen
        string publicKeyFromBuffer = QString::fromUtf8(buffer).toStdString();
        emit removePublicKeyFromTestNetwork(publicKeyFromBuffer);
    }
    break;
case TestNetworkParticipantsRequest:
    {
        emit sendAllKnownParticipants();
    }
default:
    break;
}
    currentDataType = Undefined;
    numBytesForCurrentDataType = 0;
    buffer.clear();
}

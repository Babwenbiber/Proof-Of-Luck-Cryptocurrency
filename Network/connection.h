#ifndef CONNECTION_H
#define CONNECTION_H

#include <vector>
#include <QHostAddress>
#include <QString>
#include <QTcpSocket>
#include <QTime>
#include <QTimer>
#include <iostream>
#include <QtNetwork>
#include "../Chain/blockchain.hpp"
#include "../Chain/transactions.hpp"
#include "../Interface/console.h"
#include "helperfunctions.h"

using namespace std;
using namespace HelperFunctions;

static const int MaxBufferSize = 1024000;
static const char SeparatorToken = ' ';

class Connection : public QTcpSocket
{
    Q_OBJECT

public:
    enum ConnectionState {
        WaitingForGreeting,
        ReadingGreeting,
        ReadyForUse
    };
    enum DataType {
        PlainText, // TODO: Remove after Testing
        Ping,
        Pong,
        Greeting,
        PublicKeyResponse,
        PublicKeyRequest,
        PublicKeyForTestModeAdd,
        PublicKeyForTestModeRemove,
        TestNetworkParticipantsRequest,
        ReceivedBlock,
        ReceivedTransaction,
        BlockRequest,
        BlockResponse,
        CheckBlockchain,
        Undefined
    };

    Connection(QObject *parent = 0);

    QString address() const;
    void setGreetingMessage(const QString &message);
    bool sendBlock(const QString &params);
    bool sendTransaction(const QString &params);
    bool sendBlockRequest(int blockID = 0, int forkID = 0);
    bool sendTestingNetworkParticipantsRequest();
    bool sendBlockResponse(const QString &params);
    bool sendPublicKeyRequest();
    bool sendPublicKeyResponse();
    bool sendPublicKeyForTestModeRemove();
    bool sendCheckBlockchain();
    bool sendPublicKeyForTestModeAdd(string pbkey = getPublicBkey());
    string publicKeyToAddress;

signals:
    void readyForUse();
    void addReceivedTransaction(string,string, string, int, int);
    void handleReceivedBlock(forkBlock);
    void sendBlockResponseWithBlock(int requestedBlockId, int forkID);
    void removePublicKeyFromTestNetwork(string);
    void addPublicKeyFromTestNetwork(string);
    void handlePublicKey(string);
    void sendAllKnownParticipants();
    void checkDatabase();

protected:
    void timerEvent(QTimerEvent *timerEvent) override;

private slots:
    void processReadyRead();
    void sendPing();
    void sendGreetingMessage();


private:
    int readDataIntoBuffer(int maxSize = MaxBufferSize);
    int dataLengthForCurrentDataType();
    bool readProtocolHeader();
    bool hasEnoughData();
    void processData();
    int forkID;
    QString greetingMessage;
    QString cliAddress;

    QTimer pingTimer;
    QTime pongTime;
    QByteArray buffer;
    ConnectionState state;
    DataType currentDataType;
    int numBytesForCurrentDataType;
    int transferTimerId;
    bool isGreetingMessageSent;
    Block createBlockFromString(QString);
};

#endif

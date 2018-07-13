#ifndef CLIENT_H
#define CLIENT_H
#include <time.h>
#include <QAbstractSocket>
#include <QHash>
#include <QHostAddress>
#include <QtNetwork>
#include "connection.h"
#include "peermanager.h"
#include "server.h"
#include "memory"
#include <thread>
#include "../sodiumpp/crypt.h"
#include "../Chain/transactions.hpp"
#include "../Chain/blockchain.hpp"
#include "../Chain/block.hpp"
#include "../sodiumpp/crypt.h"
#include <mutex>
#include "helperfunctions.h"

class PeerManager;
using namespace HelperFunctions;

class Client : public QObject
{
    Q_OBJECT

public:
//    int forkID = 0;
    Client(const shared_ptr<recursive_mutex>& dbMutex, const shared_ptr<recursive_mutex> &netMutex,
           const shared_ptr<mutex> &pubMutex, const shared_ptr<vector<string> > &pk,
           const shared_ptr<mutex> &testMutex,const shared_ptr<vector<string> > &testpk);
    ~Client();
    void handleTransactionCommandWithParams(const QString &message);
    void startTest();//const string &public_key
    void handleBlockCommand();
    void startMiningThread();
    void stopMiningThread();
    void startTransactionThread();
    void stopTransactionThread();
    QString address() const;
    bool hasConnection(const QHostAddress &senderIp, int senderPort = -1) const;
    QStringList readPublicKeysFromFile(const QString location);
    void executeProofOfLuck();
    int executeBalancePrinting(string pk);
    QString executePrintChain(bool detailed);
    vector<string> getAllParticipants();
    void executePrintMempool();
    void executePrintKeys();
    bool executeverifyBlockchain();
    void executeHistoryPrinting();
    int executeGetLatestBlockIndex();
    bool closeDb();
    vector<string> getPublicKeys() const;
    void setPublicKeys(const vector<string> &value);
    vector<Transaction> getMyTransactions();
    void addToList();
    void removeFromList();


signals:
    void newPeer(const QString &peer);
    void peerLeft(const QString &peer);
    void sendBlockSignal(Block *newBlock);
    void sendTransactionSignal(QString newTransaction);
    void printChainSignal();
    void updateReceiverComboBox();
    void stopMiningSignal();
    void startMiningSignal();
    void stopTransactionSignal();
    void startTransactionSignal();
    void updateLNSignal(double ln);

private slots:
    void newConnection(Connection *connection);
    void addReceivedTransaction(string,string,string,int,int);
    void handleReceivedBlock(forkBlock receivedBlockStruct);
    void handleReceivedPublicKey(string receivedPublicKey);
    void connectionError(QAbstractSocket::SocketError socketError);
    void disconnected();
    void readyForUse();
    void getChainFromNetwork();
    void getTestingNetworkParticipants();
    void requestPublicKeysFromNetwork();
    void sendBlockResponseWithBlock(int requestedBlockId, int forkID);
    void sendBlock(Block* latestBlock);
    void sendTransaction(QString newTransaction);
    void printChainSlot();
    void removePublicKeyFromTestNetwork(string);
    void addPublicKeyFromTestNetwork(string);
    void sendAllKnownTestParticipants();
    void checkDatabase();

private:
    void removeConnection(Connection *connection);
    bool threadRun;
    bool threadStopped;
    bool transactionThreadRun;
    bool transactionThreadStopped;
    shared_ptr<recursive_mutex> networkMutex;
    Blockchain myChain;
    shared_ptr<mutex> pkMutex;
    shared_ptr<vector<string>> publicKeys;
    shared_ptr<mutex> testModeMutex;
    shared_ptr<vector<string>> testModePublicKeys;
    std::thread *miner;
    std::thread *transaction;
    PeerManager *peerManager;
    Server server;
    bool hasCurrentBlockchain;
    int expectedBlockID;
    QMultiHash<QHostAddress, Connection *> peers;
};

#endif

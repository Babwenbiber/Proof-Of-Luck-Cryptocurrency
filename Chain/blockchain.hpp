#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <iostream>
#include <vector>
#include <memory>
#include <unistd.h>
#include <limits>
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include "transactions.hpp"
//#include "libs/sha1.hpp"
#include "../sodiumpp/crypt.h"
#include "merkletree.hpp"
#include "block.hpp"
#include "../Database/database.hpp"
#include "../helperfunctions.h"
#include "../Enclave/App.h"
using namespace std;
using namespace HelperFunctions;
typedef std::numeric_limits< double > dbl;
//#define DATABASE "database.db"



class Blockchain
{
public:
    Blockchain();
    Blockchain(const shared_ptr<recursive_mutex> &shared_mutex,
               const shared_ptr<vector<Transaction> >&sharedMempool,
               const shared_ptr<recursive_mutex> &memMutex);
    void initializeChain();
    void newTransaction(string send, string rec, string hash, int val, time_t timestamp);
    void ProofOfLuck(Block&);
    Block getLatestBlock();
    int getBalance(string, int BlockIndex, int forkID);
    int getBalance(string, int forkID = 0);
    bool verifyBlock(Block, int);
    QString printChain(bool detailed);
    void printLatestBlock();
    void printMempool();
    void printHistory(string);
    Block getBlock(int index, int forkID);
    void appendBlock(Block);
    string getId() const;
    void setId(const string& value);
    bool isLuckierBlock(Block);
    Block* buildNewBlock(string key, Block prev);
    int handleBlock(Block&, int*);
    bool verifyBlockchain();
    vector<string> getAllParticipants();
    bool closeDb();
    vector<Transaction> getMyTransactions();
    int getLatestBlockIndex();
    int getMySendTransactionValueFromMempool();
    void checkDatabase();

private:
    vector<Transaction> putTxInBlock();
    vector<Utxo_help> getUTXO(string, int, int forkID);
    int checkTempChain(int);
    bool checkDuplicateTransition(string);
    void mempoolUpdate(int forkID);
    void rollbackDB(int blockIndex, int forkID);
    void gdb();
    Database db;
    shared_ptr<vector<Transaction>> mempool;
    shared_ptr<recursive_mutex> mempoolMutex;
    const int MINER_REWARD = 50;
    const int TEMP_CLEAN_NUM = 10;
};
bool proofCertificate(Block);

#endif // BLOCKCHAIN_H

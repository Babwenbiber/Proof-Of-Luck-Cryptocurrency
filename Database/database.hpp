#ifndef DATABASE_H
#define DATABASE_H
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <time.h>
#include <memory>
#include <stdio.h>
#include <QString>
#include <thread>
#include "../Chain/transactions.hpp"
#include "../Chain/merkletree.hpp"
#include "../Chain/block.hpp"
#include "../libs/sqlite3.h"
#include "../sodiumpp/include/sodiumpp/base64.h"
#include "../helperfunctions.h"
#undef FunctionName

using namespace std;
using namespace HelperFunctions;

class Database
{

public:
    Database();
    Database(const shared_ptr<recursive_mutex>& sharedMutex, const shared_ptr<int>& queries);
	bool blockchainInitialized();
	bool appendBlock(Block);
	bool replaceBlock(Block);
    int createFork();
    bool addToFork(Block, int);
    bool applyFork(int);
    bool iterateOverBlocks(int);
    bool iterateOverForkBlocks(int);
	bool nextBlock();
	Block getBlock();
    Block getBlock(int blockID, int forkID);
	void stopBlockIterator();
    void stopForkBlockIterator();
    int getLastBlockIndex(int);
    Block getLatestBlock();
    int getFirstForkBlockIndex(int forkID = 0);
    bool closeDb();
    int getBalance(int block, string pk, int forkID);
    vector<Utxo_help> getUTXO(int block, string pk, int forkID);
    int getTransactionValueByHash(string hash, int forkID);
    bool isLuckierChain(int start, int end, double ln, int forkID);
    vector<string> getAllParticipants(string pk);
    vector<Transaction> getMyTransactions();
    vector<Transaction> getTransactionsFromChain(int forkID);
    vector<Transaction> getTransactionsFromFork(int forkID);
    QString getStringFromBlockchain(bool detailed);
    bool deleteFork(int);
    double getLNSum();
    bool cleanUpDBFromIndex(int index);
    bool existsTransaction(string hash, int index, int forkID);
    int getInputSumOfBlock(int index, int forkID);


private:
	sqlite3 *db;
    sqlite3_stmt *blockIterator;
    sqlite3_stmt *forkIterator;
    bool forkSpecified;
    bool forkReached;
    bool forkReady;
    bool onlyFork;
    bool firstForkBlockReturned;
    int forkId;
    shared_ptr<int> activeQuerys = 0;
	bool openDb();
    bool initializeTables();
	bool executeSql(string);
	bool executeQuery(string, sqlite3_stmt**);
    bool nextRow(sqlite3_stmt*&);
    vector<string> getRow(sqlite3_stmt*&, int);
    int finalizeQuery(sqlite3_stmt**);
    bool removeTransactions(string);
    bool removeInput(string);
	bool saveTransaction(Transaction, string);
	bool saveTransactionForFork(Transaction, string, int);
	bool saveInput(vector<string>, int, string);
    bool saveInputForFork(vector<string>, int, int, string);
	vector<Transaction> loadTransactions(string);
	vector<Transaction> loadTransactionsForFork(string, int);
	vector<string> loadInput(int);
    vector<string> loadInputForFork(int, int);
    std::shared_ptr<recursive_mutex> dbMutex;
    Block createBlockFromRow(bool);
    void gdb();
};

#endif //DATABASE_H

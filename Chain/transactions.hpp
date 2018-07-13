#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <iostream>
#include "../sodiumpp/crypt.h"
using namespace std;

class Transaction {
    public:

        Transaction(string, string, int, string, time_t);
        Transaction();
        void add_Input(vector<string>);
        string print();
        bool verifyTransaction();
        string getSender() const;
        void setSender(const string& value);
        void setMinerTransaction(string, const int);
        string getRecipient() const;
        void setRecipient(const string& value);

        int getValue() const;
        void setValue(int value);

        string getHash() const;
        void setHash(const string& value);

        vector<string> getInput() const;
        void setInput(const vector<string>& value);

        int getNumOfInputs() const;
        void setNumOfInputs(int value);

        time_t getTimestamp() const;
        void setTimestamp(const time_t& value);


private:
        string sender;
        string recipient;
        int value;
        string hash;
        vector<string> input;      //the list of utxo's (as hashes)
        int numOfInputs;
        time_t timestamp;
};


vector <Transaction> sortTransactions(vector<Transaction> trans);

#endif // TRANSACTIONS_H

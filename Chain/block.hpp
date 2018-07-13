#ifndef BLOCK_H
#define BLOCK_H
#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>

#include "transactions.hpp"
#include "../libs/sha1.hpp"

using namespace std;

class Block
{
public:
    Block();


    Block(string, string, string, time_t, vector<Transaction>, int);
    bool validateNonce(int);
    bool verifyHash();
    string buildHash(int);
    string buildHash(double);
    void makeHash();
    void makeHash(double);
    string print();

    string getHash() const;
    void setHash(const string& value);

    string getPreviousHash() const;
    void setPreviousHash(const string& value);

    string getMerkleHash() const;
    void setMerkleHash(const string& value);

    vector<Transaction> getTransaction() const;
    void setTransaction(const vector<Transaction>& value);

    time_t getTimestamp() const;
    void setTimestamp(const time_t& value);

    int getIndex() const;
    void setIndex(int value);

    int getNumTrans() const;
    void setNumTrans(int value);

    string getCertificate() const;
    void setCertificate(const string& value);

    double getLn() const;

    void setLn(double value);

private:
    string hash;
    string previousHash;
    string merkleHash;
    double ln;
    vector<Transaction> transaction;
    time_t timestamp;
    int index;
    int numTrans;
    string certificate;
};

#endif // BLOCK_H

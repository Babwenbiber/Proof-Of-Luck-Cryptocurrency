#include "block.hpp"

/**
 * @brief Block::Block
 * constructor. Pass everything but the ln and the hash
 * @param prev  hash of the previous
 * @param merkle merkle hash
 * @param trans vector of the transactions to be put in the block
 * @param time current timestamp
 * @param index blocknumber
 */
Block::Block(string prev, string hash, string merkle, time_t timestamp, vector<Transaction> trans, int index)
{
    this->timestamp = timestamp;
    this->previousHash = prev;
    this->hash = hash;
    this->merkleHash = merkle;
    this->transaction = trans;
    this->index = index;
    this->numTrans = trans.size();
}

Block::Block()
{

}

/**
 * @brief Block::getHash
 * hashes the given ln + merkle hash + previous hash.
 * @param ln to be hashed
 * @return the hash
 */
string Block::buildHash(double ln)
{
    this->ln = ln;
    SHA1 checksum;
    checksum.update(this->previousHash);
    checksum.update(to_string(ln));
    checksum.update(merkleHash);
    checksum.update(to_string(this->timestamp));
    return checksum.final();
}

bool Block::verifyHash()
{
    if(this->buildHash(this->ln).compare(this->hash) != 0)
    {
        return false;
    }
    return true;
}


/**
 * @brief Block::setHash
 * saves the hash in the block already
 * also saves timestamp
 */
void Block::makeHash()
{
    this->timestamp = time(0);
    this->hash = buildHash(this->ln);
}


/**
 * @brief Block::print
 * prints out a block, calls transaction print
 */
string Block::print()
{
    string uiBlockString = "";
//    cout << "Block \t" << this->index << " has " << this->transaction.size() << " transactions:" << endl;
    uiBlockString.append("Block \t\t" + to_string(this->index) + " has " + to_string(this->transaction.size()) + " transactions:\n");
//    cout << "Blockhash \t" << this->getHash() << endl;
    uiBlockString.append("Blockhash \t" + this->getHash() + "\n");
//    cout << "Merklehash \t" << this->getMerkleHash() << endl;
    uiBlockString.append("Merklehash \t" + this->getMerkleHash() + "\n");
//    cout << "Lucky Number \t" << this->getLn() << endl << endl;
    uiBlockString.append("Lucky Number \t" + to_string(this->getLn()) + "\n\n");
//    cout << uiBlockString;
    for(unsigned int i = 0; i < this->transaction.size(); i++)
    {

        uiBlockString.append(this->transaction.at(i).print());
    }

    return uiBlockString;
}


string Block::getHash() const
{
    return hash;
}

void Block::setHash(const string& value)
{
    hash = value;
}

string Block::getPreviousHash() const
{
    return previousHash;
}

void Block::setPreviousHash(const string& value)
{
    previousHash = value;
}

string Block::getMerkleHash() const
{
    return merkleHash;
}

void Block::setMerkleHash(const string& value)
{
    merkleHash = value;
}

vector<Transaction> Block::getTransaction() const
{
    return transaction;
}

void Block::setTransaction(const vector<Transaction>& value)
{
    transaction = value;
}

time_t Block::getTimestamp() const
{
    return timestamp;
}

void Block::setTimestamp(const time_t& value)
{
    timestamp = value;
}

int Block::getIndex() const
{
    return index;
}

void Block::setIndex(int value)
{
    index = value;
}

int Block::getNumTrans() const
{
    return numTrans;
}

void Block::setNumTrans(int value)
{
    numTrans = value;
}

string Block::getCertificate() const
{
    return certificate;
}

void Block::setCertificate(const string& value)
{
    certificate = value;
}

double Block::getLn() const
{
    return ln;
}

void Block::setLn(double value)
{
    ln = value;
}

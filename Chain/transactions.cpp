#include "transactions.hpp"

/**
 * @brief Transaction::Transaction
 * constructor. pass arguments to initialize it
 * @param sender of transaction
 * @param receiver of transaction
 * @param value to be sent
 * @param hash of the transaction
 */
Transaction::Transaction(string sender, string receiver, int value, string hash, time_t timestamp)
{
    this->sender = sender;
    this->recipient = receiver;
    this->value = value;
    this->hash = hash;
    this->input = {};
    this->timestamp = timestamp;
    this->numOfInputs = 0;
}

/**
 * @brief Transaction::setMinerTransaction
 * sets the transaction into a miner transaction
 * @param key of the miner
 * @param MINER_REWARD to pass in as value for the transaction
 */
void Transaction::setMinerTransaction(string key, const int MINER_REWARD)
{
    time_t now = time(NULL);
    SHA1 transHash;
    transHash.update(to_string(now));
    transHash.update(key);
    this->sender = "";
    this->recipient = key;
    this->value = MINER_REWARD;
    this->hash = transHash.final();
    this->input = {};
    this->timestamp = now;
    this->numOfInputs = 0;
}


/**
 * @brief Transaction::Transaction
 * This constructur is used to create empty transactions
 */
Transaction::Transaction()
{
    this->hash = "";
}

string Transaction::print()
{
    string uiTransactionString = "";
//    cout << "  transaction: " << hash << endl;
    uiTransactionString.append("  transaction: " + hash + "\n");
//    cout << "\t sender: " << sender  << endl;
    uiTransactionString.append("\t sender: " + sender  + "\n");
//    cout << "\t recipient: " << recipient << endl;
    uiTransactionString.append("\t recipient: " + recipient + "\n");
//    cout << "\t value: " << value << endl;
    uiTransactionString.append("\t value: " + to_string(value) + "\n");

    char buff[20];
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&timestamp));
//    cout << "\t time: "<< buff << endl;
    uiTransactionString.append("\t time: "+ string(buff) + "\n");
//    cout << "\t inputs: " << endl;
    uiTransactionString.append("\t" + to_string(this->getInput().size()) + " inputs: \n");
//    cout << uiTransactionString;
    for (unsigned int i = 0; i < this->getInput().size(); i++)
    {
//        cout << "\t \t" << this->getInput().at(i) << endl;
        uiTransactionString.append("\t \t" + this->getInput().at(i) + "\n");
    }
//    cout << endl;
    uiTransactionString += "\n";

    return uiTransactionString;
}

/**
 * @brief Transaction::add_Input
 * adds an input to the transaction
 * @param hash of the transaction, which is used as input
 */
void Transaction::add_Input(vector<string> input)
{
    this->input = input;
    this->numOfInputs += input.size();
}

/**
 * @brief Transaction::verifytransaction
 * verifies the transaction
 * @return true if valid, false else
 */
bool Transaction::verifyTransaction()
{
    vector<string> temp;
    //check for double input
    for (unsigned int i = 0; i < this->input.size(); i++)
    {
        for (unsigned int t = 0; t < temp.size(); t++)
        {
            if (temp.at(t).compare(this->input.at(i)) == 0)
            {
                cout << "double input" << endl;
                return false;
            }
        }
        temp.push_back(this->input.at(i));
    }
    //check if signature is right
    //wrong signatures are allowed if sender = receiver

    if (!verifySignature(this->hash, this->sender, this->recipient, this->value, this->getTimestamp()) && (this->sender.compare(this->recipient) != 0))
    {
        //cout << "wrong signature in transaction" << endl;
        return false;
    }
    return true;
}

string Transaction::getSender() const
{
    return sender;
}

void Transaction::setSender(const string& value)
{
    sender = value;
}

string Transaction::getRecipient() const
{
    return recipient;
}

void Transaction::setRecipient(const string& value)
{
    recipient = value;
}

int Transaction::getValue() const
{
    return value;
}

void Transaction::setValue(int vlue)
{
    value = vlue;
}

string Transaction::getHash() const
{
    return hash;
}

void Transaction::setHash(const string& value)
{
    hash = value;
}

vector<string> Transaction::getInput() const
{
    return input;
}

void Transaction::setInput(const vector<string>& value)
{
    input = value;
}

int Transaction::getNumOfInputs() const
{
    return numOfInputs;
}

void Transaction::setNumOfInputs(int value)
{
    numOfInputs = value;
}

time_t Transaction::getTimestamp() const
{
    return timestamp;
}

void Transaction::setTimestamp(const time_t& value)
{
    timestamp = value;
}

/**
 * @brief sortTransactions
 * sorts the transactions lexicographic.
 * this is needed to make the order of the transactions in the merkle tree clear!
 * @param trans transactions to be sorted
 * @return a sorted vector of transactions
 */
vector<Transaction> sortTransactions(vector<Transaction> trans)
{
    Transaction temp;
    int j;
    for(int i = 1; i < (signed int)trans.size(); i++)
    {
        temp = trans.at(i);
        j = i;

        while (j > 0 && trans.at(j - 1).getHash().compare(temp.getHash()) > 0) //temp is lexicographicly-lower
        {
            trans.at(j) = trans.at(j - 1);
            j--;
        }
        trans.at(j) = temp;
    }
    return trans;
}

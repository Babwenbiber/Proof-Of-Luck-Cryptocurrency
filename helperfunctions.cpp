#include "helperfunctions.h"


QString HelperFunctions::parseBlockToQString(Block block, int forkID)
{
    string blockAsString = "";
    blockAsString += block.getPreviousHash() + ","
            + block.getMerkleHash() + ",";

    // Vectors to String
    string vectorAsString;

    for (unsigned int i = 0; block.getTransaction().size() > i; i++)
    {
        Transaction temp = block.getTransaction().at(i);

        vectorAsString += temp.getSender() + "_"
                + temp.getRecipient() + "_"
                + std::to_string(temp.getValue()) + "_"
                + temp.getHash() + "_"
                + std::to_string(temp.getTimestamp()) + "_";

        for (unsigned int j = 0; temp.getInput().size() > j; j++)
        {
            vectorAsString += temp.getInput().at(j);
            vectorAsString += "-";
        }
        vectorAsString += ";";

    }
    blockAsString += vectorAsString + ",";

    blockAsString += block.getHash() + ",";
    blockAsString += to_string(block.getIndex()) + ",";
    double getLn = block.getLn();
    string LN =            base64_encode((unsigned char*)&getLn, sizeof(double));
    blockAsString += LN + ",";
    blockAsString += to_string(block.getTimestamp()) + ",";
    blockAsString += block.getCertificate() + ",";
    blockAsString += to_string(forkID) + ",";

    return QString::fromStdString(blockAsString);
}

HelperFunctions::forkBlock HelperFunctions::parseQStringToBlock(QString block)
{
    QStringList paramsList = block.split(",");
    vector<Transaction> transactionFromString = {};
    QStringList tempTransList = paramsList.at(2).split(";");

    for (int i = 0; tempTransList.size() - 1 > i; i++)
    {
        QStringList transAsString = tempTransList.at(i).split("_");

        Transaction temp = Transaction(transAsString.at(0).toStdString(),
                                       transAsString.at(1).toStdString(),
                                       transAsString.at(2).toDouble(),
                                       transAsString.at(3).toStdString(),
                                       transAsString.at(4).toInt());

        vector<string> inputsFromString = {};
        if (transAsString.size() >= 6)
        {
            QStringList inputsAsString = transAsString.at(5).split("-");
            for (int j = 0; inputsAsString.size() - 1 > j; j++)
            {
                inputsFromString.push_back(inputsAsString.at(j).toStdString());
            }
        }
        temp.setInput(inputsFromString);
        //temp.print();
        transactionFromString.push_back(temp);
    }

    //prev,merkle, vector,index
    Block* receivedBlock = new Block();
    receivedBlock->setPreviousHash(paramsList.at(0).toStdString());
    receivedBlock->setMerkleHash(paramsList.at(1).toStdString());

    receivedBlock->setTransaction(transactionFromString);
    receivedBlock->setHash(paramsList.at(3).toStdString());
    double ln;
    vector<unsigned char> ln_copy;
    ln_copy = base64_decode(paramsList.at(5).toStdString());
    memcpy(&ln, ln_copy.data(), sizeof(double));
    receivedBlock->setLn(ln);
    time_t time;
    forkBlock retVal;

    try {
        time = paramsList.at(6).toInt();
        receivedBlock->setIndex((paramsList.at(4)).toInt());
        retVal.forkID = paramsList.at(8).toInt();

    }
    catch (...)
    {
        time = 0;
        receivedBlock->setIndex(2);
        retVal.forkID = 0;
        cout << endl << "*************caught an invalid sent Block**************" << endl << endl;
    }
    receivedBlock->setTimestamp(time);
    receivedBlock->setCertificate(paramsList.at(7).toStdString());
    retVal.block = receivedBlock;
    //cout << "****RECEIVED BLOCK*********" << endl;
    //cout << receivedBlock->print();
    return retVal;
}

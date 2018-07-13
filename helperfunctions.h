#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H
#include "Chain/block.hpp"
#include <QString>
#include <QDataStream>


namespace HelperFunctions
{
    struct forkBlock {
        int forkID;
        Block* block;
    };

    QString parseBlockToQString(Block block, int forkID);
    forkBlock parseQStringToBlock(QString block);
    const int ROUND_TIME = 30;  //in seconds
    struct Utxo_help {
        string hash;
        int value;
    };

    struct Utxo_change {
        string key; //pub key from sender
        int change; //change value
        vector<string> input;   //a list of values, that were used as input in this round
    };


}

#endif // HELPERFUNCTIONS_H

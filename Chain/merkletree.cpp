#include "merkletree.hpp"
/**
 * @brief MerkleTree::MerkleTree
 * The merkle tree is a binary tree, that contains successiv hashes of the transactions,
 * that will be put in the block
 * it is usefull to verify transactions
 * @param trans the transactions, that will be put in the block
 */
MerkleTree::MerkleTree(vector<Transaction> trans)
{
    trans = sortTransactions(trans);
    height = ceil(log2 (trans.size())) + 1; //logarithmic height;
    if (trans.size() == 0)
    {
        cout << "!!!!!! 0 transactions: " << height << endl;
    }
    numOfLattice = trans.size();
    root = buildTree(trans);
}
/**
 * @brief MerkleTree::buildTree
 * gets called from the constructor
 * @param trans the lattice transactions
 * @return returns the root of the tree
 */
Node MerkleTree::buildTree(vector<Transaction> trans)
{
    Node arr[height][numOfLattice];
    int x = ceil(trans.size()), i, j;
    //initialize the first hash;
    for (i = 0; i < numOfLattice; i++)
    {
        for(j = 0; j < height; j++)
        {
            arr[j][i].hash = "";
        }
    }
    for (i = 0; i < numOfLattice; i++)
    {
        arr[0][i].hash = trans.at(i).getHash();
    }
    for (i = 1; i < height; i++)
    {
        for(j = 0; j < x; j++)
        {
            SHA1 checksum;
            arr[i][j / 2].left = &arr[i - 1][j];
            j++;
            if(arr[i-1][j].hash == "")
            {
                checksum.update(arr[i][j / 2].left->hash);
                arr[i][j/2].hash= checksum.final();

                break;
            }

            arr[i][j / 2].right = &arr[i - 1][j];

            checksum.update(arr[i][j / 2].left->hash);
            checksum.update(arr[i][j / 2].right->hash);
            arr[i][j / 2].hash= checksum.final();
        }
        x = ceil(x / 2);
    }

    //Now transform into a tree with Node struct
    return arr[height - 1][0];
}

Node MerkleTree::getRoot() const
{
    return root;
}

void MerkleTree::setRoot(const Node& value)
{
    root = value;
}

int MerkleTree::getHeight() const
{
    return height;
}

void MerkleTree::setHeight(int value)
{
    height = value;
}

int MerkleTree::getNumOfLattice() const
{
    return numOfLattice;
}

void MerkleTree::setNumOfLattice(int value)
{
    numOfLattice = value;
}
/**
 * @brief MerkleTree::getMerkleHash
 * @return returns the hashroot of the tree
 */
string MerkleTree::getMerkleHash() {
    return root.hash;
}

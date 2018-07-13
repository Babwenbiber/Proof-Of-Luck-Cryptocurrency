#ifndef MERKLETREE_H
#define MERKLETREE_H
#include "transactions.hpp"
#include "../libs/sha1.hpp"
#include <vector>
#include <math.h>       /* log2 */

struct Node {
    string hash;
    Node* left;
    Node* right;
};

class MerkleTree
{
public:
    MerkleTree(vector<Transaction>);
    string getMerkleHash();
    int getNumOfLattice() const;
    void setNumOfLattice(int value);

    int getHeight() const;
    void setHeight(int value);

    Node getRoot() const;
    void setRoot(const Node& value);

private:
    Node buildTree(vector<Transaction>);
    Node root;
    int height;
    int numOfLattice;
};

#endif // MERKLETREE_H

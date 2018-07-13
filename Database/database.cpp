#include "database.hpp"
#include <clocale>
#define DATABASE "database.db"

/**
 * @brief Database::Database()
 * constructor
 */

Database::Database()
{
    openDb();
    initializeTables();
}

Database::Database(const shared_ptr<recursive_mutex> &sharedMutex, const shared_ptr<int> &queries)
    :dbMutex(sharedMutex), activeQuerys(queries)
{
    openDb();
    initializeTables();
}

/**
 * @brief Database::blockchainInitialized
 * checks if a blockchain is saved,
 * specifically if a genesis block exists
 * @return true if blockchain exists
 */
bool Database::blockchainInitialized() 
{
    bool isInitialized = false;
    sqlite3_stmt *result;

    string sql = "SELECT * FROM BLOCKCHAIN";

    if(!executeQuery(sql, &result)) 
    {
        return false;
    }

    isInitialized = nextRow(result);

    finalizeQuery(&result);

    return isInitialized;
}

/**
 * @brief Blockchain::appendBlock
 * appends block to the persisted blockchain;
 * the index will be automatically determined by
 * the database as the next available index
 * @param block to save
 * @return true if successful
 */
bool Database::appendBlock(Block block)
{
    setlocale(LC_NUMERIC, "en_US.UTF-8");
    string HASH =          block.getHash();
    string PREVIOUS_HASH = block.getPreviousHash();
    string MERKLE_HASH =   block.getMerkleHash();
    double getLn = block.getLn();
    string LN =            base64_encode((unsigned char*)&getLn, sizeof(double));
    string TIMESTAMP =     to_string(block.getTimestamp());
    string NUM_TRANS =     to_string(block.getNumTrans());
    string CERTIFICATE =   block.getCertificate();
    string sql = string("INSERT INTO BLOCKCHAIN(HASH, PREVIOUS_HASH, MERKLE_HASH, LN, TIMESTAMP, NUM_TRANS, CERTIFICATE) ")
                        + "VALUES ("
                            + "'" + HASH + "'" + ","            
                            + "'" + PREVIOUS_HASH + "'" + ","  
                            + "'" + MERKLE_HASH + "'" + ","    
                            + "'" + LN + "',"
                            + TIMESTAMP + ","
                            + NUM_TRANS + ","
                            + "'" + CERTIFICATE + "'"
                        + ");";

   // cout << "sql: " << sql << endl;

    if(!executeSql(sql))
    {
        return false;
    }
    for (int i = 0; i < (signed int)block.getTransaction().size(); i++)
    {
        saveTransaction(block.getTransaction().at(i), to_string(block.getIndex()));
    }

    return true;
}

bool Database::replaceBlock(Block block)
{
    string INDEX =         to_string(block.getIndex());
    string HASH =          block.getHash();
    string PREVIOUS_HASH = block.getPreviousHash();
    string MERKLE_HASH =   block.getMerkleHash();
    double getLn = block.getLn();
    string LN =            base64_encode((unsigned char*)&getLn, sizeof(double));
    string TIMESTAMP =     to_string(block.getTimestamp());
    string NUM_TRANS =     to_string(block.getNumTrans());
    string CERTIFICATE =   block.getCertificate();

    string sql = string("INSERT OR REPLACE INTO BLOCKCHAIN(BLOCK_INDEX, HASH, PREVIOUS_HASH, MERKLE_HASH, LN, TIMESTAMP, NUM_TRANS, CERTIFICATE) ")
                        + "VALUES ("
                            + INDEX + ","
                            + "'" + HASH + "'" + ","            
                            + "'" + PREVIOUS_HASH + "'" + ","  
                            + "'" + MERKLE_HASH + "'" + ","    
                            + "'" + LN + "',"
                            + TIMESTAMP + ","
                            + NUM_TRANS + ","
                            + "'" + CERTIFICATE + "'"
                        + ");";

    if(!executeSql(sql)) 
    {
        return false;
    }
    if (!removeTransactions(to_string(block.getIndex())))
    {
        cout << "couldn't remove transactions" << endl;
        return false;
    }
    if (!removeInput(to_string(block.getIndex())))
    {
        cout << "couldn't remove input" << endl;
        return false;
    }

    for (int i = 0; i < (signed int)block.getTransaction().size(); i++)
    {
        if (!saveTransaction(block.getTransaction().at(i), to_string(block.getIndex())))
        {
            cout << "couldn't save transactions" << endl;
            return false;
        }
    }

    return true;
}


/**
 * @brief Blockchain::createFork
 * creates a table for a new fork
 * @return forkId or -1 in case of failure
 */
int Database::createFork()
{
    string sql = "INSERT INTO FORKS(ID) VALUES(NULL);";

    if(!executeSql(sql)) 
    {
        gdb();
        cout << __FUNCTION__ << endl;
        return -1;
    }

    int forkId = sqlite3_last_insert_rowid(db);

    sql = "CREATE TABLE IF NOT EXISTS FORK" + to_string(forkId) + "("
            + "BLOCK_INDEX    INTEGER   PRIMARY KEY,"
            + "HASH           TEXT      NOT NULL,"                  
            + "PREVIOUS_HASH  TEXT      NOT NULL,"                  
            + "MERKLE_HASH    TEXT      NOT NULL,"                 
            + "LN             DOUBLE    NOT NULL,"
            + "TIMESTAMP      DATETIME  NOT NULL,"                  
            + "NUM_TRANS      INTEGER   NOT NULL,"
            + "CERTIFICATE    TEXT      NOT NULL);"

            + "CREATE TABLE IF NOT EXISTS FORK" + to_string(forkId) + "_TRANSACTIONS("
            + "ID             INTEGER   PRIMARY KEY,"
            + "HASH           TEXT      NOT NULL,"
            + "BLOCK          TEXT      NOT NULL,"
            + "SENDER         TEXT      NOT NULL,"
            + "RECIPIENT      TEXT      NOT NULL,"
            + "VALUE          INTEGER   NOT NULL,"
            + "NUM_OF_INPUTS  INTEGER   NOT NULL,"
            + "TIMESTAMP      DATETIME  NOT NULL);"

            + "CREATE TABLE IF NOT EXISTS FORK" + to_string(forkId) + "_INPUT("
            + "ID             INTEGER   PRIMARY KEY,"
            + "HASH           TEXT      NOT NULL,"
            + "TRANS          INTEGER   NOT NULL,"
            + "BLOCK          TEXT      NOT NULL);";

    if(!executeSql(sql)) 
    {
        return -1;
    }
    //cout << "forkID returned  " << forkId << endl;
    return forkId;
}


/**
 * @brief Blockchain::addToFork
 * adds a block to a fork
 * @param block block to insert
 * @param forkId id of the fork
 * @return true if successful
 */
bool Database::addToFork(Block block, int forkId)
{
    if (forkId < 0)
    {
        return false;
    }

    string INDEX =         to_string(block.getIndex());
    string HASH =          block.getHash();
    string PREVIOUS_HASH = block.getPreviousHash();
    string MERKLE_HASH =   block.getMerkleHash();
    double getLn = block.getLn();
    string LN =            base64_encode((unsigned char*)&getLn, sizeof(double));
    string TIMESTAMP =     to_string(block.getTimestamp());
    string NUM_TRANS =     to_string(block.getNumTrans());
    string CERTIFICATE =   block.getCertificate();

    string sql = "INSERT INTO FORK" + to_string(forkId) + "(BLOCK_INDEX, HASH, PREVIOUS_HASH, MERKLE_HASH, LN, TIMESTAMP, NUM_TRANS, CERTIFICATE) "
                        + "VALUES ("
                            + INDEX + ","
                            + "'" + HASH + "'" + ","            
                            + "'" + PREVIOUS_HASH + "'" + ","  
                            + "'" + MERKLE_HASH + "'" + ","    
                            + "'" + LN + "',"
                            + TIMESTAMP + ","
                            + NUM_TRANS + ","
                            + "'" + CERTIFICATE + "'"
                            + ");";

    if(!executeSql(sql)) 
    {
        return false;
    }

    for (int i = 0; i < (signed int)block.getTransaction().size(); i++)
    {
        saveTransactionForFork(block.getTransaction().at(i), to_string(block.getIndex()), forkId);
    }

    return true;
}

/**
 * @brief Blockchain::applyFork
 * saves all blocks of a fork to the main blockchain
 * and deletes the fork
 * @param forkId id of the fork to apply
 * @return true if successful
 */
bool Database::applyFork(int forkId)
{
    sqlite3_stmt *result;
    vector<unsigned char> ln_copy;
    string sql = "SELECT * FROM FORK" + to_string(forkId) + ";";

    if(!executeQuery(sql, &result))
    {
        return false;
    }
    //cout << "got data selected:  " << sql << endl;

    vector<Block> fork;
    while(nextRow(result))
    {
        vector<string> row = getRow(result, 8);

        int index = stoi(row.at(0));
        string hash = row.at(1);
        string prev = row.at(2);
        string merkle = row.at(3);
        double ln;
        ln_copy = base64_decode(row.at(4));
        memcpy(&ln, ln_copy.data(), sizeof(double));

        time_t timestamp = stoi(row.at(5));
        string certificate = row.at(7);
        Block block(prev, hash, merkle, timestamp, loadTransactionsForFork(to_string(index), forkId), index);

        block.setHash(hash);
        block.setLn(ln);
        block.setTimestamp(timestamp);
        block.setCertificate(certificate);

        fork.push_back(block);
    }

    finalizeQuery(&result);

    for(unsigned int i = 0; i < fork.size(); i++)
    {   

        if(!replaceBlock(fork.at(i))) 
        {
            return false;
        }
    }


    if(!deleteFork(forkId)) 
    {
        return false;
    }

    return true;
}

/**
 * @brief Blockchain::deleteFork
 * deletes a fork
 * @param forkId id of the fork to delete
 * @return true if successful
 */
bool Database::deleteFork(int forkId) 
{

    string sql = "DROP TABLE FORK" + to_string(forkId) + ";"
                + "DROP TABLE FORK" + to_string(forkId) + "_TRANSACTIONS;"
                + "DROP TABLE FORK" + to_string(forkId) + "_INPUT;"
    ;
    if(!executeSql(sql)) {
        return false;
    }
    return true;
}

double Database::getLNSum()
{
    string sql = "SELECT LN FROM BLOCKCHAIN;";
    sqlite3_stmt *result;
    double ln, lnSum = 0.0;
    vector<unsigned char> ln_copy;

    if(!executeQuery(sql, &result))
    {
        return false;
    }

    while (nextRow(result))
    {
        ln_copy = base64_decode(getRow(result, 1).at(0));
        memcpy(&ln, ln_copy.data(), sizeof(double));
        lnSum += ln;
    }
    finalizeQuery(&result);
    return lnSum;
}

bool Database::cleanUpDBFromIndex(int index)
{
    string sql = "DELETE FROM BLOCKCHAIN WHERE CAST(BLOCK_INDEX AS INT) >=" + to_string(index) + ";"
                + "DELETE FROM TRANSACTIONS WHERE CAST(BLOCK AS INT) >=" + to_string(index) + ";"
                + "DELETE FROM INPUT WHERE CAST(BLOCK AS INT) >=" + to_string(index) + ";"
    ;
    return executeSql(sql);
}

bool Database::existsTransaction(string hash, int index, int forkID)
{
    string sql;
    sqlite3_stmt *result;
    if (forkID == 0)
    {
        sql = string("select exists (select * from transactions where hash like '") + hash + "'" +
                     " and cast(block as int) < " + to_string(index) + ");";
    }
    else
    {
        sql = string("select exists (select * from transactions where hash like '") + hash + "'" +
                     " and cast(block as int) < cast((select min(cast(block_index as int)) from fork" + to_string(forkID) + ") as int)" +
                     " union select * from fork" + to_string(forkID) + "_transactions where hash like '" + hash + "'" +
                     " and cast(block as int) <" + to_string(index) + ");";
    }
    if(!executeQuery(sql, &result))
    {
        gdb();
        return true;
    }
    if (nextRow(result))
    {
        int retVal;
        retVal = stoi(getRow(result,1).at(0));
        finalizeQuery(&result);
        return retVal;
    }
    finalizeQuery(&result);
    return true;

}

int Database::getInputSumOfBlock(int index, int forkID)
{
    string sql;
    sqlite3_stmt *result;
    if (forkID == 0)
    {
        sql = string("select coalesce(sum(value), 0) from (select distinct t.hash, t.value from input") +
                     " join transactions as t on t.hash like input.hash where input.block = " + to_string(index) + ");";
    }
    else
    {
        sql = string("with tr as (select * from transactions where cast(block as int) < (select min(cast(block_index as int)) from fork") + to_string(forkID) + ")" +
                     "          union select * from fork" + to_string(forkID) + "_transactions)" +
                     " select coalesce(sum(value), 0) from (select distinct t.hash, t.value from fork" + to_string(forkID) + "_input as i" +
                     " join tr as t on t.hash like i.hash where i.block = " + to_string(index) + ");";
    }
    //cout << "***********" << sql << endl;
    if(!executeQuery(sql, &result))
    {
        gdb();
        return true;
    }
    if (nextRow(result))
    {
        int retVal;
        retVal = stoi(getRow(result,1).at(0));
        finalizeQuery(&result);
        return retVal;
    }
    finalizeQuery(&result);
    return true;

}

/**
 * @brief Blockchain::iterateOverBlocks
 * starts an iteration over the blocks saved in the database
 * @param forkId id of the fork; 0 for main chain
 * @return true if successful
 */


bool Database::iterateOverBlocks(int forkId)
{
    /*if (!dbMutex->try_lock())
        gdb();*/
    dbMutex->lock();
	forkReached = false;
	forkSpecified = false;
    forkReady = false;
    onlyFork = false;
    string sql = "SELECT * FROM BLOCKCHAIN;";

    if (!executeQuery(sql, &blockIterator)) 
    {
        return false;
    }


    if(forkId != 0)
    {
        this->forkId = forkId;
    	forkSpecified = true;
        firstForkBlockReturned = false;

    	sqlite3_stmt *result;

        sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='FORK" + to_string(forkId) + "';";

	    if (!executeQuery(sql, &result)) 
	    {
	        return false;
	    }

	    bool forkExists = nextRow(result);
	    finalizeQuery(&result);

	    if(forkExists)
        {
		    sql = "SELECT * FROM FORK" + to_string(forkId) + ";";

		    if (!executeQuery(sql, &forkIterator)) 
		    {
		        return false;
		    }

		    bool hasBlock = nextRow(forkIterator);

		    if(!hasBlock)
		    {
		    	finalizeQuery(&forkIterator);
		    	forkSpecified = false;
		    }
	    } else {
	    	forkSpecified = false;
	    }
    }

    return true;
}

bool Database::iterateOverForkBlocks(int forkID)
{
    dbMutex->lock();
    forkReached = true;
    forkSpecified = true;
    forkReady = true;
    firstForkBlockReturned = false;
    onlyFork = true;
    this->forkId = forkID;
    sqlite3_stmt *result;
    string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='FORK" + to_string(forkID) + "';";
    if (!executeQuery(sql, &result))
    {
        return false;
    }
    bool forkExists = nextRow(result);
    finalizeQuery(&result);

    if(forkExists)
    {
        sql = "SELECT * FROM FORK" + to_string(forkID) + ";";

        if (!executeQuery(sql, &forkIterator))
        {
            return false;
        }

        bool hasBlock = nextRow(forkIterator);

        if(!hasBlock)
        {
            finalizeQuery(&forkIterator);
        }
    }

    return true;

}

/**
 * @brief Blockchain::nextBlock
 * moves the iterator to the next block;
 * if no next block exists, this function stops the iterator
 * @return true if next Block exists
 */
bool Database::nextBlock()
{
    bool nextBlock = false;
    //gdb();
    if(forkReady)
    {
        forkReached = true;
        if (!onlyFork)
        {
            finalizeQuery(&blockIterator);
        }

        if (firstForkBlockReturned) {
    		nextBlock = nextRow(forkIterator);
    	} else {
    		nextBlock = true;
            firstForkBlockReturned = true;
    	}
	    if(!nextBlock) 
	    {
            nextBlock = false;
	        finalizeQuery(&forkIterator);
	    }
    } else {
    	nextBlock = nextRow(blockIterator);

	    if(!nextBlock) 
	    {
	        finalizeQuery(&blockIterator);
	    }
    }

    return nextBlock;
}


Block Database::createBlockFromRow(bool fork)
{
    vector<unsigned char> ln_copy;
    vector<string> row;
    if (fork)
    {
        row = getRow(forkIterator, 8);
    } else {
        row = getRow(blockIterator, 8);
    }
    int index = stoi(row.at(0));
    string hash = row.at(1);
    string prev = row.at(2);
    string merkle = row.at(3);
    double ln;
    ln_copy = base64_decode(row.at(4));
    memcpy(&ln, ln_copy.data(), sizeof(double));
    time_t timestamp = stoi(row.at(5));
    string certificate = row.at(7);
    vector<Transaction> trans;
    if (fork)
    {
        trans = loadTransactionsForFork(to_string(index), forkId);
    } else {
        trans = loadTransactions(to_string(index));
    }
    Block block(prev, hash, merkle, timestamp, trans, index);
    block.setLn(ln);
    block.setTimestamp(timestamp);
    block.setCertificate(certificate);

    return block;
}

void Database::gdb()
{
    cout << "in" << endl;
}

/**
 * @brief Blockchain::getBlock
 * loads the block that the iterator currently points to
 * @return next block
 */
Block Database::getBlock()
{
    if(forkSpecified)
    {
        Block forkBlock = createBlockFromRow(true);
        /*if (getLastBlockIndex() == 0)
        {
            int test;
            cin >> test;
        }*/
        if(!forkReached)
        {
            Block mainChainBlock = createBlockFromRow(false);
            if(mainChainBlock.getIndex() >= forkBlock.getIndex() - 1)
            {
                forkReady = true;
            }

            return mainChainBlock;
        }

        return forkBlock;
    }
    Block block = createBlockFromRow(false);
    return block;
}

Block Database::getBlock(int blockID, int forkID)
{
    sqlite3_stmt *result;
    vector<unsigned char> ln_copy;
    string sql;

    if (forkID != 0 && getFirstForkBlockIndex(forkID) <= blockID)
    {
        sql = "SELECT * FROM FORK" + to_string(forkID) + " WHERE BLOCK_INDEX=" + to_string(blockID) + ";";
    }
    else
    {
        sql = "SELECT * FROM BLOCKCHAIN WHERE BLOCK_INDEX=" + to_string(blockID) + ";";
    }

    if(!executeQuery(sql, &result))
    {
        cout << "exec failed in getBlock(int)" << endl;
        return Block();
    }

    if(nextRow(result))
    {
        vector<string> row;

        row = getRow(result, 8);

        int index = stoi(row.at(0));
        string hash = row.at(1);
        string prev = row.at(2);
        string merkle = row.at(3);
        double ln;
        ln_copy = base64_decode(row.at(4));
        memcpy(&ln, ln_copy.data(), sizeof(double));
        time_t timestamp = stoi(row.at(5));
        string  certificate = row.at(7);
        vector<Transaction> trans;

        trans = loadTransactions(to_string(index));

        Block block(prev, hash, merkle, timestamp, trans, index);
        block.setLn(ln);
        block.setTimestamp(timestamp);
        block.setCertificate(certificate);
        finalizeQuery(&result);

        return block;
    }
    finalizeQuery(&result);

    return Block();
}

/**
 * @brief Database::stopBlockIterator
 * stops the block iterator;
 * usage only necessay if the iterator
 * isn't used to iterate over all blocks
 */
void Database::stopBlockIterator() 
{
    finalizeQuery(&blockIterator);
    if (forkSpecified)
    {
        finalizeQuery(&forkIterator);
    }
    dbMutex->unlock();
}

void Database::stopForkBlockIterator()
{
    finalizeQuery(&forkIterator);
    dbMutex->unlock();
}

/**
 * @brief Blockchain::getLastBlockIndex
 * gets the index of the last Block
 * @return last block index
 */
int Database::getLastBlockIndex(int forkID)
{
    int lastBlockIndex = 0;
    sqlite3_stmt *result;
    string sql;
    if (forkID == 0)
    {
        sql = "SELECT COALESCE(MAX(BLOCK_INDEX),0) FROM BLOCKCHAIN;";
    }
    else {
        sql = "SELECT COALESCE(MAX(BLOCK_INDEX),0) FROM FORK" + to_string(forkID) + ";";
    }

    if(!executeQuery(sql, &result)) 
    {
        cout << "exec failed in getlast" << endl;
        return lastBlockIndex;
    }

    if(nextRow(result)) 
    {
        vector<string> row = getRow(result, 1);

        lastBlockIndex = stoi(row.at(0));
    }

    finalizeQuery(&result);

    return lastBlockIndex;
}

Block Database::getLatestBlock()
{
    vector<unsigned char> ln_copy;
    sqlite3_stmt *result;
    string sql = string("SELECT * FROM  BLOCKCHAIN WHERE BLOCK_INDEX=") + to_string(getLastBlockIndex(0)) + ";";
    //cout << sql << endl;
    if(!executeQuery(sql, &result))
    {
        cout << "exec failed in getBlock(int)" << endl;
        return Block();
    }

    if(nextRow(result))
    {
        vector<string> row;

        row = getRow(result, 8);

        int index = stoi(row.at(0));
        string hash = row.at(1);
        string prev = row.at(2);
        string merkle = row.at(3);
        double ln;
        ln_copy = base64_decode(row.at(4));
        memcpy(&ln, ln_copy.data(), sizeof(double));
        time_t timestamp = stoi(row.at(5));
        string certificate = row.at(7);
        vector<Transaction> trans;

        trans = loadTransactions(to_string(index));

        Block block(prev, hash, merkle, timestamp, trans, index);
        block.setLn(ln);
        block.setTimestamp(timestamp);
        block.setCertificate(certificate);
        finalizeQuery(&result);
        return block;
    }
    finalizeQuery(&result);
    return Block();
}

int Database::getFirstForkBlockIndex(int forkID)
{
    int blockIndex = 0;
    sqlite3_stmt *result;

    string sql = "SELECT MIN(BLOCK_INDEX) FROM FORK" + to_string(forkID) + ";";

    if(!executeQuery(sql, &result))
    {
        cout << "exec failed in getlast" << endl;
        return blockIndex;
    }

    if(nextRow(result))
    {
        vector<string> row = getRow(result, 1);

        blockIndex = stoi(row.at(0));
    }

    finalizeQuery(&result);

    return blockIndex;
}

/**
 * @brief Database::openDb
 * opens the database
 * @return true if successful
 */
bool Database::openDb() 
{
    int rc;

	rc = sqlite3_open(DATABASE, &db);

    if (rc)
    {
        cout << "couldnt open. rc = " << rc << endl;
        return false;
    }

    return true;
}

/**
 * @brief Database::closeDb
 * closes the database
 */
bool Database::closeDb()
{
    int rc;
    
	rc = sqlite3_close(db);
    if(rc)
    {
        // cout << "close state is " << rc << endl;
        return false;
    }

    
    return true;
}

int Database::getBalance(int block, string pk, int forkID)
{
    sqlite3_stmt *result;
    string sql;
    if (forkID == 0)
    {
        sql = string("with inc as(SELECT COALESCE(sum( VALUE ),0) as received") +
                            " FROM TRANSACTIONS" +
                            " WHERE CAST(BLOCK AS INT) <= " + to_string(block) +
                                " AND RECIPIENT LIKE '" + pk +
                                "' AND SENDER NOT LIKE RECIPIENT)," +
                      "outp as (SELECT COALESCE(sum( VALUE ),0) as spent" +
                             " FROM TRANSACTIONS" +
                             " WHERE CAST(BLOCK AS INT) <= " + to_string(block) +
                             " AND SENDER LIKE '" + pk +
                             "' AND SENDER NOT LIKE RECIPIENT)" +
                 " select inc.received - outp.spent from inc, outp;";
    }
    else
    {
        sql = string("with tr as (select sender, block, recipient, value from transactions") +
                     "      where cast(block as int) < (select min (cast(block as int) ) as minID from  fork" + to_string(forkID) + "_transactions) union" +
                     "      select sender, block, recipient, value from fork" + to_string(forkID) + "_transactions" +
                     "      where cast(block as int) >= (select min (cast(block as int) ) as minID from  fork" + to_string(forkID) + "_transactions))," +
                     " inc as (SELECT COALESCE(sum( t.VALUE ),0) as received  FROM tr AS t" +
                     "      WHERE CAST(t.BLOCK AS INT) <= " + to_string(block) +
                     "      AND t.RECIPIENT LIKE '" + pk + "'" +
                     "      AND t.SENDER NOT LIKE  t.recipient)," +
                     "      outp as (SELECT COALESCE(sum( t.VALUE ),0) as spent  FROM tr AS t" +
                     "      WHERE CAST(t.BLOCK AS INT) <= " + to_string(block) +
                     "      AND t.sender LIKE '" + pk + "'" +
                     "      AND t.RECIPIENT NOT LIKE t.sender)" +

                     " select inc.received - outp.spent from inc, outp;";
    }
    //cout << "***********" << endl << sql << endl;
    if(!executeQuery(sql, &result))
    {
        return false;
    }

    if (nextRow(result))
    {
        int retVal = stoi(getRow(result, 1).at(0));
        finalizeQuery(&result);
	if (retVal < 0)
		gdb();
        return retVal;
    }
    gdb();
    cout << __FUNCTION__ << endl;

    return -1;

}

vector<Utxo_help> Database::getUTXO(int block, string pk, int forkID)
{
    vector<Utxo_help> retValue;
    //string bcTable = "BLOCKCHAIN";
    string trTable = "TRANSACTIONS";
    string inTable = "INPUT";
    if (forkID != 0)
    {
       // bcTable = "FORK" + forkID;
        trTable = "FORK" + to_string(forkID) + "_TRANSACTIONS";
        inTable = "FORK" + to_string(forkID) + "_INPUT";
    }

    sqlite3_stmt *result;
    string sql;
    if (forkID == 0)
    {
        sql = string("SELECT distinct t.Hash, t.Value ") +
                        "FROM " + trTable + " AS t" +
                 " WHERE CAST(t.BLOCK AS INT) <= " + to_string(block) +
                     " AND t.RECIPIENT LIKE '" + pk +
                     "' AND NOT EXISTS " +
                         " ( SELECT  i.HASH FROM " + inTable + " AS i" +
                         " where t.HASH LIKE i.HASH" +
                         " and CAST(i.BLOCK AS INT) <= " + to_string(block) + ");";
    }
    else
    {
        sql = string("with inp as(select hash, block from input union") +
                     " select hash, block from fork" + to_string(forkID) + "_input)," +
                     " tr as (select hash, block, recipient, value from transactions union" +
                     " select hash, block, recipient, value from fork" + to_string(forkID) + "_transactions)" +
                     " SELECT distinct t.Hash, t.Value " +
                     " FROM tr AS t" +
                     " WHERE CAST(t.BLOCK AS INT) <= " + to_string(block) +
                         " AND t.RECIPIENT LIKE '" + pk +
                         "' AND NOT EXISTS " +
                             " ( SELECT  i.HASH FROM inp AS i" +
                             " where t.HASH LIKE i.HASH" +
                             " and CAST(i.BLOCK AS INT) <= " + to_string(block) + ");";
    }
    //cout << "***********" << endl << sql << endl;
    if(!executeQuery(sql, &result))
    {
        gdb();
        cout << __FUNCTION__ << endl;
        return retValue;
    }
    Utxo_help temp;
    while (nextRow(result))
    {
        temp.hash = getRow(result, 2).at(0);
        temp.value = stoi(getRow(result, 2).at(1));
        retValue.push_back(temp);
    }
    finalizeQuery(&result);
    return retValue;
}

int Database::getTransactionValueByHash(string hash, int forkID)
{
    sqlite3_stmt *result;
    string sql;

    if (forkID == 0)
    {
        sql = string("SELECT VALUE FROM TRANSACTIONS WHERE HASH LIKE '") + hash + "';";
    } else {
        sql = string("SELECT VALUE FROM TRANSACTIONS WHERE HASH LIKE '") + hash + "' UNION" +
        " SELECT VALUE FROM FORK" + to_string(forkID) + "_TRANSACTIONS WHERE HASH LIKE '" + hash + "';";
    }
    if(!executeQuery(sql, &result))
    {
        return 0;
    }
    int retVal[2] = {0, 0};
    int i =0;
    while (nextRow(result))
    {
        retVal[i] = stoi(getRow(result, 1).at(0));
        i++;
    }
    finalizeQuery(&result);
    if (i > 1 && retVal[0] != retVal[1])
        gdb();

    return retVal[0];
}

bool Database::isLuckierChain(int start, int end, double ln, int forkID)
{
    sqlite3_stmt *result;
    string sql;
    double tempLN, sumLN = 0;
    vector<unsigned char> ln_copy;
    sql = string("SELECT b.LN FROM BLOCKCHAIN as b") +
                 " WHERE b.BLOCK_INDEX BETWEEN " + to_string(start) + " AND " + to_string(end) + ";";
    if(!executeQuery(sql, &result))
    {
        return 0;
    }
    while (nextRow(result))
    {
      ln_copy = base64_decode(getRow(result, 1).at(0));
      memcpy(&tempLN, ln_copy.data(), sizeof(double));
      sumLN += tempLN;
    }

    finalizeQuery(&result);
    cout << "mainchainLN: " << sumLN << " forkLN " << ln << endl;
    if (sumLN == ln)
    {
        if (forkID == 0)
        {
            return 0;
        }
        cout << "Lucky numbers where the same, proofing for lexicographically order now." << endl;
        sql = string("select b.hash < f.hash from blockchain as b, fork") + to_string(forkID) + " as f where b.block_index = "
                + to_string(start) + " and f.block_index = " + to_string(start) + ";";
        if(!executeQuery(sql, &result))
        {
            return 0;
        }
        if (nextRow(result))
        {
            int retVal = stoi(getRow(result, 1).at(0));
            finalizeQuery(&result);

            return retVal;
        }
    }
    return sumLN<ln;
}

vector<string> Database::getAllParticipants(string pk)
{
    vector<string> parts;
    parts.push_back(getPublicBkey());
    sqlite3_stmt *result;
    string sql;

    sql = string("select distinct sender  from (select sender from transactions") +
                 " union select recipient as sender from transactions)" +
                 " where sender not like 'GenesisMiner' AND sender NOT LIKE '' AND sender NOT LIKE '" + pk + "';";
    if(!executeQuery(sql, &result))
    {
        return parts;
    }
    while (nextRow(result))
    {
        parts.push_back(getRow(result, 1).at(0));
    }
    finalizeQuery(&result);
    return parts;
}

vector<Transaction> Database::getMyTransactions()
{
    vector<Transaction> transactions;
    transactions = {};
    sqlite3_stmt *result;

    string sql = "SELECT * FROM TRANSACTIONS WHERE sender = '" + getPublicBkey() + "' AND recipient NOT LIKE sender ORDER BY BLOCK DESC;";
    if(!executeQuery(sql, &result))
    {
        //gdb();
        cout << "exec failed" << endl;
        return transactions;
    }
    while(nextRow(result))
    {
        vector<string> row = getRow(result, 8);

        int id = stoi(row.at(0));
        string hash = row.at(1);
        string sender = row.at(3);
        string recipient = row.at(4);
        int value = stoi(row.at(5));
        int numOfInputs = stoi(row.at(6));
        time_t timestamp = stoi(row.at(7));

        Transaction transaction(sender, recipient, value, hash, timestamp);
        transaction.setNumOfInputs(numOfInputs);
        transaction.setInput(loadInput(id));

        transactions.push_back(transaction);
    }
    finalizeQuery(&result);
    return transactions;
}

vector<Transaction> Database::getTransactionsFromChain(int forkID)
{
    vector<Transaction> transactions;
    transactions = {};
    sqlite3_stmt *result;

    string sql = string("SELECT * FROM TRANSACTIONS WHERE CAST(BLOCK AS INT) >= ") +
                        "(SELECT MIN(CAST(BLOCK_INDEX AS INT)) FROM FORK" + to_string(forkID) + ");";
    if(!executeQuery(sql, &result))
    {
        //gdb();
        cout << "exec failed" << endl;
        return transactions;
    }
    while(nextRow(result))
    {
        vector<string> row = getRow(result, 8);

        int id = stoi(row.at(0));
        string hash = row.at(1);
        string sender = row.at(3);
        string recipient = row.at(4);
        int value = stoi(row.at(5));
        int numOfInputs = stoi(row.at(6));
        time_t timestamp = stoi(row.at(7));

        Transaction transaction(sender, recipient, value, hash, timestamp);
        transaction.setNumOfInputs(numOfInputs);
        transaction.setInput(loadInput(id));
        //gdb();
        transactions.push_back(transaction);
    }
    finalizeQuery(&result);
    return transactions;
}

vector<Transaction> Database::getTransactionsFromFork(int forkID)
{
    vector<Transaction> transactions;
    transactions = {};
    sqlite3_stmt *result;

    string sql = "SELECT * FROM FORK" + to_string(forkID) + "_TRANSACTIONS;";
    if(!executeQuery(sql, &result))
    {
        //gdb();
        cout << "exec failed" << endl;
        return transactions;
    }
    while(nextRow(result))
    {
        vector<string> row = getRow(result, 8);

        int id = stoi(row.at(0));
        string hash = row.at(1);
        string sender = row.at(3);
        string recipient = row.at(4);
        int value = stoi(row.at(5));
        int numOfInputs = stoi(row.at(6));
        time_t timestamp = stoi(row.at(7));

        Transaction transaction(sender, recipient, value, hash, timestamp);
        transaction.setNumOfInputs(numOfInputs);
        transaction.setInput(loadInput(id));

        transactions.push_back(transaction);
    }
    finalizeQuery(&result);
    return transactions;
}

QString Database::getStringFromBlockchain(bool detailed)
{
    QString bc, transactions;
    sqlite3_stmt *result;

    string sql = string("SELECT b.*,t.*,coalesce(i.id, -1), coalesce(i.hash, -1), coalesce(i.trans, -1), coalesce(i.block, -1)") +
                 " FROM (select * from BLOCKCHAIN order by block_index desc limit 10) as b" +
                 " JOIN TRANSACTIONS as t ON b.BLOCK_INDEX LIKE t.BLOCK left JOIN INPUT AS i ON t.ID = i.TRANS" +
                 " order by b.block_index desc;";
    if(!executeQuery(sql, &result))
    {
        //gdb();
        cout << "exec failed" << endl;
        return "";
    }
    int curBlockIndex = 0;
    int curTransIndex = 0;


    vector<string> row;

    int b_id;
    string b_hash;
    string b_prev;
    string b_merkle;
    double b_ln;
    vector<unsigned char> ln_copy;
    time_t b_timestamp;
    int b_numOfTrans;
    string b_certificate;
    char buff[20];

    string t_hash;
    string t_sender;
    string t_recipient;
    string t_value;
    string t_numOfInputs;
    time_t t_timestamp;

    string input;

    while(nextRow(result))
    {
        row.clear();
        row = getRow(result, 20);

        input = row.at(17);

        if (curBlockIndex == stoi(row.at(0)))
        {
            if (curTransIndex == stoi(row.at(8)))
            {
                if (detailed)
                {
                    transactions.append(QString::fromStdString("\t\t\t " + input + "\n"));
                }
            }
            else {
                curTransIndex = stoi(row.at(8));
                t_hash = row.at(9);
                t_sender = row.at(11);
                t_recipient = row.at(12);
                t_value = row.at(13);
                t_numOfInputs = stoi(row.at(14));
                t_timestamp = stoi(row.at(15));

                if (detailed)
                {
                    transactions.append(QString::fromStdString("\n\t\tTransaction: \t" + t_hash +"\n"));
                }
                transactions.append(QString::fromStdString("\t\tSender: \t" + t_sender + "\n"));
                transactions.append(QString::fromStdString("\t\tRecipient: \t" + t_recipient + "\n"));
                transactions.append(QString::fromStdString("\t\tValue: \t\t" + t_value + "\n"));
                if (detailed)
                {
                    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t_timestamp));
                    transactions.append(QString::fromStdString("\t\tTime: \t\t" + string(buff) + "\n"));
                    transactions.append(QString::fromStdString("\t\tInputs:\n"));
                    if (input.compare("-1") != 0)
                        transactions.append(QString::fromStdString("\t\t\t " + input + "\n"));
                }
                if (t_sender.compare("") == 0)
                {
                    bc.append(QString::fromStdString("Miner: \t\t" + t_recipient + "\n"));
//                    bc.append("\n");
                    if (detailed)
                    {
                        bc.append(transactions);
                        transactions.clear();
                    }
                }
            }
        }
        else
        {
            curBlockIndex = stoi(row.at(0));


            b_id = stoi(row.at(0));
            b_hash = row.at(1);
            b_prev = row.at(2);
            b_merkle = row.at(3);
            ln_copy = base64_decode(row.at(4));
            memcpy(&b_ln, ln_copy.data(), sizeof(double));
            b_timestamp = stoi(row.at(5));
            b_numOfTrans = stoi(row.at(6));
            b_certificate = row.at(7);

            bc.append(QString::fromStdString("-----------------------------------------------\n"));
            bc.append(QString::fromStdString("Block: \t\t" + to_string(b_id) + " has " +to_string(b_numOfTrans) + " transactions:\n"));
            bc.append(QString::fromStdString("Blockhash: \t" + b_hash + "\n"));
            bc.append(QString::fromStdString("Merklehash: \t" + b_merkle + "\n"));
            bc.append(QString::fromStdString("Lucky Number: \t" + to_string(b_ln) + "\n"));
            bc.append(QString::fromStdString("Certificate: \t" + b_certificate + "\n"));
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&b_timestamp));
            bc.append(QString::fromStdString("Time: \t\t"+ string(buff) + "\n"));
//            bc.append(QString::fromStdString("Miner: \t\t" + t_recipient + "\n"));

            if (curTransIndex == stoi(row.at(8)))
            {
                if (detailed)
                {
                    bc.append(QString::fromStdString("\t\t\t " + input + "\n"));
                }
            }
            else {
                curTransIndex = stoi(row.at(8));
                t_hash = row.at(9);
                t_sender = row.at(11);
                t_recipient = row.at(12);
                t_value = row.at(13);
                t_numOfInputs = stoi(row.at(14));
                t_timestamp = stoi(row.at(15));

                if (detailed)
                {
                    transactions.append(QString::fromStdString("\n\t\tTransaction: \t" + t_hash +"\n"));
                }
                transactions.append(QString::fromStdString("\t\tSender: \t" + t_sender + "\n"));
                transactions.append(QString::fromStdString("\t\tRecipient: \t" + t_recipient + "\n"));
                transactions.append(QString::fromStdString("\t\tValue: \t\t" + t_value + "\n"));
                if (detailed)
                {
                    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t_timestamp));
                    transactions.append(QString::fromStdString("\t\tTime: \t\t" + string(buff) + "\n"));
                    transactions.append(QString::fromStdString("\t\tInputs:\n"));
                    if (input.compare("-1") != 0)
                        transactions.append(QString::fromStdString("\t\t\t " + input + "\n"));
                }
                if (t_sender.compare("") == 0)
                {
                    bc.append(QString::fromStdString("Miner: \t\t" + t_recipient + "\n"));
//                    bc.append("\n");
                    if (detailed)
                    {
                        bc.append(transactions);
                        transactions.clear();
                    }
                }
            }
        }

    }
    finalizeQuery(&result);
    return bc;
}
/*
QString Database::getStringFromBlockchain(bool detailed)
{
    QString bc, transactions;
    sqlite3_stmt *result;

    string sql = string("SELECT b.*,t.*,coalesce(i.id, -1), coalesce(i.hash, -1), coalesce(i.trans, -1), coalesce(i.block, -1)") +
                 " FROM (select * from BLOCKCHAIN order by block_index desc limit 10) as b" +
                 " JOIN TRANSACTIONS as t ON b.BLOCK_INDEX LIKE t.BLOCK left JOIN INPUT AS i ON t.ID = i.TRANS" +
                 " order by b.block_index desc;";
    if(!executeQuery(sql, &result))
    {
        //gdb();
        cout << "exec failed" << endl;
        return "";
    }
    int curBlockIndex = 0;
    int curTransIndex = 0;


    vector<string> row;

    int b_id;
    string b_hash;
    string b_prev;
    string b_merkle;
    double b_ln;
    vector<unsigned char> ln_copy;
    time_t b_timestamp;
    int b_numOfTrans;
    string b_certificate;
    char buff[20];

    string t_hash;
    string t_sender;
    string t_recipient;
    string t_value;
    string t_numOfInputs;
    time_t t_timestamp;

    string input;

    while(nextRow(result))
    {
        row.clear();
        row = getRow(result, 20);

        input = row.at(17);

        if (curBlockIndex == stoi(row.at(0)))
        {
            if (curTransIndex == stoi(row.at(8)))
            {
                if (detailed)
                {
                    bc.append(QString::fromStdString("\t\t\t " + input + "\n"));
                }
            }
            else {
                curTransIndex = stoi(row.at(8));
                t_hash = row.at(9);
                t_sender = row.at(11);
                t_recipient = row.at(12);
                t_value = row.at(13);
                t_numOfInputs = stoi(row.at(14));
                t_timestamp = stoi(row.at(15));
                bc.append("\n");
                if (detailed)
                {
                    transactions.append(QString::fromStdString("\t\tTransaction: \t" + t_hash +"\n"));
                }
                transactions.append(QString::fromStdString("\t\tSender: \t" + t_sender + "\n"));
                transactions.append(QString::fromStdString("\t\tRecipient: \t" + t_recipient + "\n"));
                transactions.append(QString::fromStdString("\t\tValue: \t\t" + t_value + "\n"));
                if (detailed)
                {
                    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t_timestamp));
                    transactions.append(QString::fromStdString("\t\tTime: \t\t" + string(buff) + "\n"));
                    transactions.append(QString::fromStdString("\t\tInputs:\n"));
                    if (input.compare("-1") != 0)
                        transactions.append(QString::fromStdString("\t\t\t " + input + "\n"));
                }
                if (t_sender.compare("") == 0)
                {
                    bc.append(QString::fromStdString("Miner: \t\t" + t_recipient + "\n"));
                    bc.append("\n");
                    if (detailed)
                    {
                        bc.append(transactions);
                        transactions.clear();
                    }
                }
                else
                {
                    bc.append("\n");
                    bc.append(transactions);
                    transactions.clear();
                }
            }
        }
        else
        {
            curBlockIndex = stoi(row.at(0));


            b_id = stoi(row.at(0));
            b_hash = row.at(1);
            b_prev = row.at(2);
            b_merkle = row.at(3);
            ln_copy = base64_decode(row.at(4));
            memcpy(&b_ln, ln_copy.data(), sizeof(double));
            b_timestamp = stoi(row.at(5));
            b_numOfTrans = stoi(row.at(6));
            b_certificate = row.at(7);

            bc.append(QString::fromStdString("-----------------------------------------------\n"));
            bc.append(QString::fromStdString("\nBlock \t" + to_string(b_id) + " has " +to_string(b_numOfTrans) + " transactions:\n"));
            bc.append(QString::fromStdString("Blockhash: \t" + b_hash + "\n"));
            bc.append(QString::fromStdString("Merklehash \t" + b_merkle + "\n"));
            bc.append(QString::fromStdString("Lucky Number: \t" + to_string(b_ln) + "\n"));
            bc.append(QString::fromStdString("Certificate: \t" + b_certificate + "\n"));
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&b_timestamp));
            bc.append(QString::fromStdString("Time: \t\t"+ string(buff) + "\n"));


            if (curTransIndex == stoi(row.at(8)))
            {
                if (detailed)
                {
                    bc.append(QString::fromStdString("\t\t\t " + input + "\n"));
                }
            }
            else {
                curTransIndex = stoi(row.at(8));
                t_hash = row.at(9);
                t_sender = row.at(11);
                t_recipient = row.at(12);
                t_value = row.at(13);
                t_numOfInputs = stoi(row.at(14));
                t_timestamp = stoi(row.at(15));

                if (detailed)
                {
                    transactions.append(QString::fromStdString("\t\tTransaction: \t" + t_hash +"\n"));
                }
                transactions.append(QString::fromStdString("\t\tSender: \t" + t_sender + "\n"));
                transactions.append(QString::fromStdString("\t\tRecipient: \t" + t_recipient + "\n"));
                transactions.append(QString::fromStdString("\t\tValue: \t\t" + t_value + "\n"));
                if (detailed)
                {
                    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t_timestamp));
                    transactions.append(QString::fromStdString("\t\tTime: \t\t" + string(buff) + "\n"));
                    transactions.append(QString::fromStdString("\t\tInputs:\n"));
                    if (input.compare("-1") != 0)
                        transactions.append(QString::fromStdString("\t\t\t " + input + "\n"));
                }
                if (t_sender.compare("") == 0)
                {
                    bc.append(QString::fromStdString("Miner: \t\t" + t_recipient + "\n"));
                    bc.append("\n");
                    if (detailed)
                    {
                        bc.append(transactions);
                        transactions.clear();
                    }
                }
                else
                {
                    bc.append("\n");
                    bc.append(transactions);
                    transactions.clear();
                }
            }
        }

    }
    finalizeQuery(&result);
    return bc;
}

 */
/**
 * @brief Blockchain::initializeTables
 * creates the necessary tables for blocks, transactions and input
 * if not already existant
 * @return true if successful
 */
bool Database::initializeTables()
{

    string sql = 
            string("CREATE TABLE IF NOT EXISTS BLOCKCHAIN(")
            + "BLOCK_INDEX    INTEGER   PRIMARY KEY,"
            + "HASH           TEXT      NOT NULL,"                  
            + "PREVIOUS_HASH  TEXT      NOT NULL,"                  
            + "MERKLE_HASH    TEXT      NOT NULL,"                 
            + "LN             DOUBLE    NOT NULL,"
            + "TIMESTAMP      DATETIME  NOT NULL,"                  
            + "NUM_TRANS      INTEGER   NOT NULL,"
            + "CERTIFICATE    TEXT      NOT NULL);"

            + "CREATE TABLE IF NOT EXISTS TRANSACTIONS("
            + "ID             INTEGER   PRIMARY KEY,"
            + "HASH           TEXT      NOT NULL,"
            + "BLOCK          TEXT      NOT NULL,"
            + "SENDER         TEXT      NOT NULL,"
            + "RECIPIENT      TEXT      NOT NULL,"
            + "VALUE          INTEGER   NOT NULL,"
            + "NUM_OF_INPUTS  INTEGER   NOT NULL,"
            + "TIMESTAMP      DATETIME  NOT NULL);"

            + "CREATE TABLE IF NOT EXISTS INPUT("
            + "ID             INTEGER   PRIMARY KEY,"
            + "HASH           TEXT      NOT NULL,"
            + "TRANS          INTEGER   NOT NULL,"
            + "BLOCK          TEXT      NOT NULL);"

            + "CREATE TABLE IF NOT EXISTS FORKS("
            + "ID             INTEGER   PRIMARY KEY);";

    return executeSql(sql);
}

/**
 * @brief Database::executeSql
 * executes a sql statemnt
 * @param sqlString the sql statement in string format
 * @return true if successful
 */
bool Database::executeSql(string sqlString) 
{
    int rc;
    char *zErrMsg = 0;
    char const *sql = sqlString.c_str();
    dbMutex->lock();
    rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    dbMutex->unlock();
    if(rc != SQLITE_OK)
    {
        gdb();
        cout << "exec fail" << endl;
        cout << sqlString << endl;
        cout << "SQL error: " << zErrMsg << endl;
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}

/**
 * @brief Database::executeQuery
 * executes a sql query
 * @param sqlString the sql quert in string format
 * @param result to a handle for the result
 * @return true if successful
 */
bool Database::executeQuery(string sqlString, sqlite3_stmt **result) 
{
    int rc;
    char const *sql = sqlString.c_str();
    *activeQuerys += 1;
    dbMutex->lock();
    rc = sqlite3_prepare_v2(db, sql, -1, result, NULL);
    dbMutex->unlock();
    if (rc != SQLITE_OK) 
    {
        *activeQuerys -= 1;
        gdb(); cout << "Failed to fetch data: " << sqlite3_errstr(rc) << endl;
        cout << sqlString << endl;
        return false;
    }

    return true;
}

/**
 * @brief Database::nextRow
 * advances the iterator of a query result to the next row
 * @param result pointer to the handle of the result
 * @return true if next row exists
 */
bool Database::nextRow(sqlite3_stmt* &result)
{
    return sqlite3_step(result) == SQLITE_ROW;
}

/**
 * @brief Database::getRow
 * creates a vector of the values of each column of the
 * row that the iterator,
 * which belongs to the provided result handle,
 * points to.
 * @param result handle of the result
 * @param numColumns number of columns for which to retrieve the values
 * @return vector with the column values as strings
 */
vector<string> Database::getRow(sqlite3_stmt* &result, int numColumns)
{
    vector<string> row = {};

    for (int i = 0; i < numColumns; i++) 
    {
        string value = string(reinterpret_cast<const char*>(sqlite3_column_text(result, i)));
        row.push_back(value);
    }
    return row;
}

/**
 * @brief Database::finalizeQuery
 * finalizes an iterator over provided result;
 * needs to be called for each created iterator,
 * when no longer needed
 * @param result handle of the query result
 */
int Database::finalizeQuery(sqlite3_stmt **result)
{
    int rc = 100;
	if(*result != NULL) {
        rc = sqlite3_finalize(*result);
        *result = NULL;
        *activeQuerys-= 1;
    }
    return rc;
}

bool Database::removeTransactions(string index)
{
    string sql = "DELETE FROM TRANSACTIONS WHERE BLOCK = " + index + ";";

    return executeSql(sql);
}

bool Database::removeInput(string index)
{
    string sql = "DELETE FROM INPUT WHERE BLOCK = " + index + ";";

    if(!executeSql(sql))
    {
        return false;
    }

    return true;
}

/**
 * @brief Database::saveTransaction
 * saves a transaction of a block
 * @param transaction transaction to be saved
 * @param block index of the block that the transaction belongs to
 * @return true if successful
 */
bool Database::saveTransaction(Transaction transaction, string block) 
{
    string HASH =          transaction.getHash();
    string BLOCK =         block;
    string SENDER =        transaction.getSender();
    string RECIPIENT =     transaction.getRecipient();
    string VALUE =         to_string(transaction.getValue());
    string NUM_OF_INPUTS = to_string(transaction.getNumOfInputs());
    string TIMESTAMP =     to_string(transaction.getTimestamp());
    string sql = string("INSERT OR REPLACE INTO TRANSACTIONS(HASH, BLOCK, SENDER, RECIPIENT, VALUE, NUM_OF_INPUTS, TIMESTAMP) ")
                        + "VALUES ("
                            + "'" + HASH + "'" + ","            
                            + "'" + BLOCK + "'" + ","  
                            + "'" + SENDER + "'" + "," 
                            + "'" + RECIPIENT + "'" + ","       
                            + VALUE + ","           
                            + NUM_OF_INPUTS + ","
                            + TIMESTAMP
                        + ");";
    //cout << "***********" << endl << sql << endl;
    if(!executeSql(sql)) 
    {
        return false;
    }
    int transactionId = sqlite3_last_insert_rowid(db);
    return saveInput(transaction.getInput(), transactionId, block);
}

/**
 * @brief Database::saveTransactionForFork
 * saves a transaction of a block of a fork
 * @param transaction transaction to be saved
 * @param block index of the block that the transaction belongs to
 * @param forkId id of the fork
 * @return true if successful
 */
bool Database::saveTransactionForFork(Transaction transaction, string block, int forkId)
{
    string HASH =          transaction.getHash();
    string BLOCK =         block;
    string SENDER =        transaction.getSender();
    string RECIPIENT =     transaction.getRecipient();
    string VALUE =         to_string(transaction.getValue());
    string NUM_OF_INPUTS = to_string(transaction.getNumOfInputs());
    string TIMESTAMP =     to_string(transaction.getTimestamp());

    string sql = "INSERT INTO FORK" + to_string(forkId) + "_TRANSACTIONS(HASH, BLOCK, SENDER, RECIPIENT, VALUE, NUM_OF_INPUTS, TIMESTAMP) "
                        + "VALUES ("
                            + "'" + HASH + "'" + ","            
                            + "'" + BLOCK + "'" + ","  
                            + "'" + SENDER + "'" + "," 
                            + "'" + RECIPIENT + "'" + ","       
                            + VALUE + ","           
                            + NUM_OF_INPUTS + ","
                            + TIMESTAMP
                        + ");";

    if(!executeSql(sql)) 
    {
        return false;
    }

    int transactionId = sqlite3_last_insert_rowid(db);
    return saveInputForFork(transaction.getInput(), transactionId, forkId, block);
}

/**
 * @brief Database::saveInput
 * saves input of a transaction
 * @param input input to be saved
 * @param transaction id of the transaction that the input belongs to
 * @return true if successful
 */
bool Database::saveInput(vector<string> input, int transaction, string block) 
{
    string sql = "";

    for (unsigned int i = 0; i < input.size(); i++)
    {
        sql += string("INSERT INTO INPUT(HASH, TRANS, BLOCK) ")
                        + "VALUES ("
                            + "'" + input.at(i) + "'" + ","
                            + to_string(transaction) + ","
                            + "'" + block + "'"
                        + ");";
    }


    return executeSql(sql);
}

/**
 * @brief Database::saveInputForFork
 * saves input of a transaction of a block of a fork
 * @param input input to be saved
 * @param transaction id of the transaction that the input belongs to
 * @param forkId id of the fork
 * @return true if successful
 */
bool Database::saveInputForFork(vector<string> input, int transaction, int forkId, string block)
{
    string sql = "";

    for (unsigned int i = 0; i < input.size(); i++)
    {
        sql += "INSERT INTO FORK" + to_string(forkId) + "_INPUT(HASH, TRANS, BLOCK) "
                        + "VALUES ("
                            + "'" + input.at(i) + "'" + ","
                            + to_string(transaction) + ","
                              + "'" + block + "'"
                        + ");";
    }

    return executeSql(sql);
}

/**
 * @brief Blockchain::loadTransactions
 * loads the transactions for a block
 * @param block index of the block for which to load the transactions
 * @return transactions as vector of type Transaction
 */
vector<Transaction> Database::loadTransactions(string block) 
{
    vector<Transaction> transactions;
    transactions = {};
    sqlite3_stmt *result;

    string sql = "SELECT * FROM TRANSACTIONS WHERE BLOCK = " + block + ";";
    if(!executeQuery(sql, &result)) 
    {
        //gdb();
        cout << "exec failed" << endl;
        return transactions;
    }
    while(nextRow(result))
    {
        vector<string> row = getRow(result, 8);

        int id = stoi(row.at(0));
        string hash = row.at(1);
        string sender = row.at(3);
        string recipient = row.at(4);
        int value = stoi(row.at(5));
        int numOfInputs = stoi(row.at(6));
        time_t timestamp = stoi(row.at(7));

        Transaction transaction(sender, recipient, value, hash, timestamp);
        transaction.setNumOfInputs(numOfInputs);
        transaction.setInput(loadInput(id));

        transactions.push_back(transaction);
    }
    finalizeQuery(&result);
    return transactions;
}

/**
 * @brief Blockchain::loadTransactionsForFork
 * loads the transactions for a block of a fork
 * @param block index of the block for which to load the transactions
 * @param forkId id of the fork
 * @return transactions as vector of type Transaction
 */
vector<Transaction> Database::loadTransactionsForFork(string block, int forkId) 
{
    vector<Transaction> transactions;
    transactions = {};
    sqlite3_stmt *result;

    string sql = "SELECT * FROM FORK" + to_string(forkId) + "_TRANSACTIONS WHERE BLOCK = '" + block + "';";

    if(!executeQuery(sql, &result)) 
    {
        cout << "exec failed int forkloadTrans" << endl;
        return transactions;
    }

    while(nextRow(result))
    {
        vector<string> row = getRow(result, 8);

        int id = stoi(row.at(0));
        string hash = row.at(1);
        string sender = row.at(3);
        string recipient = row.at(4);
        int value = stoi(row.at(5));
        int numOfInputs = stoi(row.at(6));
        time_t timestamp = stoi(row.at(7));

        Transaction transaction(sender, recipient, value, hash, timestamp);
        transaction.setNumOfInputs(numOfInputs);
        transaction.setInput(loadInputForFork(id, forkId));

        transactions.push_back(transaction);
    }

    finalizeQuery(&result);
    return transactions;
}

/**
 * @brief Blockchain::loadInput
 * loads the input for a transaction
 * @param transaction id of the transaction
 * @return input as vector of type string
 */
vector<string> Database::loadInput(int transaction) 
{
    vector<string> input = {};
    sqlite3_stmt *result;

    string sql = "SELECT * FROM INPUT WHERE TRANS = '" + to_string(transaction) + "';";

    if(!executeQuery(sql, &result)) 
    {
        return input;
    }

    while(nextRow(result))
    {
        vector<string> row = getRow(result, 2);

        string hash = row.at(1);

        input.push_back(hash);
    }

    finalizeQuery(&result);
    return input;
}

/**
 * @brief Blockchain::loadInputForFork
 * loads the input for a transaction of a block of a fork
 * @param transaction id of the transaction
 * @param forkId id of the fork
 * @return input as vector of type string
 */
vector<string> Database::loadInputForFork(int transaction, int forkId) 
{
    vector<string> input = {};
    sqlite3_stmt *result;

    string sql = "SELECT * FROM FORK" + to_string(forkId) + "_INPUT WHERE TRANS = '" + to_string(transaction) + "';";

    if(!executeQuery(sql, &result)) 
    {
        return input;
    }

    while(nextRow(result))
    {
        vector<string> row = getRow(result, 3);

        string hash = row.at(1);

        input.push_back(hash);
    }

    finalizeQuery(&result);
    return input;
}

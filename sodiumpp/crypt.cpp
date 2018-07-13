#include "crypt.h"

/**
 * @brief create_keypair
 * creates a private and public keypair
 * only creates a new pair if one of the files
 * in keys is missing
 */
void createKeypair() {
    ifstream p(PUBLIC_KEY_PATH);
    ifstream s(PRIVATE_KEY_PATH);
    if (!p.good() || !s.good())
    {
        unsigned char pk[crypto_sign_PUBLICKEYBYTES];
        unsigned char sk[crypto_sign_SECRETKEYBYTES];
        crypto_sign_keypair(pk, sk);
        string pk_to_64 = base64_encode(pk, crypto_sign_PUBLICKEYBYTES);
        string sk_to_64 = base64_encode(sk, crypto_sign_SECRETKEYBYTES);
        ofstream file;
        file.open (PUBLIC_KEY_PATH);
        file << pk_to_64;
        file.close();
        file.open (PRIVATE_KEY_PATH);
        file << sk_to_64;
        file.close();
    }
}

/**
 * @brief get_public_bkey
 * @return the public key in base64 format
 */
string getPublicBkey()
{
    string line;
    ifstream file;
    file.open(PUBLIC_KEY_PATH);
    getline (file,line);
    file.close();
    return line;
}

/**
 * @brief get_private_bkey
 * @return the private key in base64 format
 */
string getPrivateBkey()
{
    string line;
    ifstream file;
    file.open(PRIVATE_KEY_PATH);
    getline (file,line);
    file.close();
    return line;
}

/**
 * @brief get_public_ukey
 * transforms a given base64 public key into a unsigned char array
 * this method does not return the key, but uses the given ukey pointer
 * @param ukey pointer to the variable which gets set as the key
 * @param len length of the key, usually crypto_sign_PUBLICKEYLENGTH
 */
void getPublicUkey(unsigned char* ukey, int len)
{
    string line = getPublicBkey();
    vector<unsigned char> b64_to_str;

    b64_to_str = base64_decode(line);
    unsigned char* converted = &b64_to_str[0];
    for (int i = 0; i < len; i++)
        ukey[i] = converted[i];
}

/**
 * @brief get_private_ukey
 * transforms a given base64 private key into a unsigned char array
 * this method does not return the key, but uses the given ukey pointer
 * @param ukey pointer to the variable which gets set as the key
 * @param len length of the key, usually crypto_sign_SECRETKEYLENGTH
 */
void getPrivateUkey(unsigned char* ukey, int len)
{
    string line = getPrivateBkey();
    vector<unsigned char> b64_to_str;

    b64_to_str = base64_decode(line);
    unsigned char* converted = &b64_to_str[0];
    for (int i = 0; i < len; i++)
        ukey[i] = converted[i];
}

/**
 * @brief sign_msg
 * signs a transaction with the private key in the keys folder
 * this method uses a timestamp in the signature as well, to make
 * all transaction hashes look different
 * to shorten the message, it gets hashed b4 signing
 * @param rec receiver of the transaction
 * @param send sender of the Tx
 * @param val value if the Transaction
 * @return base64 string of the signature, which will be uses as a hash
 */
string signMsg(string rec, string send, int val)
{
    SHA1 transHash;
    transHash.update(rec);
    transHash.update(send);
    transHash.update(to_string(val));
    transHash.update(to_string(time(0)));
    string orig_msg = transHash.final();
    unsigned long long MESSAGE_LEN = orig_msg.length();
    unsigned char msg[MESSAGE_LEN];
    for (unsigned int i = 0; i < MESSAGE_LEN; i++){
        msg[i] = (unsigned char)(orig_msg[i]);
    }
    unsigned char signed_message[crypto_sign_BYTES + MESSAGE_LEN];
    unsigned long long signed_message_len;
    unsigned char sk[crypto_sign_SECRETKEYBYTES];
    getPrivateUkey(sk, crypto_sign_SECRETKEYBYTES);
    crypto_sign(signed_message, &signed_message_len,
        msg, MESSAGE_LEN, sk);
    return base64_encode(signed_message, signed_message_len);
}

/**
 * @brief msg_base_to_uchar
 * transforms a given base64 string to an unsigned char*
 * USAGE: dont use the return value, rather use the pointer to save the variablee
 * @param msg base64 message
 * @param signed_message unsigned char pointer which will get set
 * @param len pointer to the length. will be set as well
 * @return the unsiged char pointer
 */
unsigned char* msgBaseToUchar(string msg, unsigned char* signed_message, int* len)
{
    vector<unsigned char> temp;
    temp = base64_decode(msg);
    *len = temp.size() - 1;
    memcpy(signed_message, temp.data(), *len);
    return signed_message;
}

/**
 * @brief verify_signature
 * verifies if a given string (base64) was signed by the given (secret) key
 * @param msg base64 string of the message
 * @param b_pk public key of the sender
 * @return true if signature is valid, false else
 */
bool verifySignature(string msg, string b_pk, string rec, int val, time_t timestamp)
{
    unsigned char* pk = (unsigned char*)malloc(crypto_sign_PUBLICKEYBYTES);
    int temp;
    if (b_pk.compare("") == 0)
        return false;
    pk = msgBaseToUchar(b_pk, pk, &temp);
    unsigned char* signed_message = (unsigned char*)malloc(sizeof(unsigned char*)* 2000);
    int signed_message_len;
    msgBaseToUchar(msg, signed_message, &signed_message_len);
    unsigned char* unsigned_message = (unsigned char*)malloc(2000);
    unsigned long long unsigned_message_len;
    if (crypto_sign_open(unsigned_message, &unsigned_message_len,
                         signed_message, signed_message_len, pk) != 0)
    {
        return false;
    }
    SHA1 transHash;
    transHash.update(rec);
    transHash.update(b_pk);
    transHash.update(to_string(val));
    transHash.update(to_string(timestamp));
    string sent_msg = transHash.final();
    //check if the value and the receiver fits
    for (unsigned int i = 0; i < sent_msg.length(); i++)
    {
        if (sent_msg[i] != unsigned_message[i])
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief print_key
 * prints the own public key
 */
void printKey()
{
    cout << getPublicBkey() << endl;
}

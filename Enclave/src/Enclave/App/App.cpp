/* Copyright (C) 2011-2017 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdbool.h>
#include <assert.h>

#include <unistd.h>
#include <pwd.h>

#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"
#include "base64.h"
#include <sgx_tcrypto.h>

using namespace std;

/*Global EID shared by multiple threads*/
sgx_enclave_id_t global_eid = 0;

extern "C" {

    void *memset_s(void *s, int c, size_t n) {
        return memset(s, c, n);
    }

    sgx_status_t SGXAPI sgx_read_rand(uint8_t *buf, size_t size) {
        *buf = 0;
    }
}

/*Check error conditions for loading enclave*/
void print_error_message(sgx_status_t ret) {

    size_t idx = 0;
    size_t ttl = sizeof(sgx_errlist)/sizeof(sgx_errlist[0]);

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }

    if (idx == ttl) {
		printf("Error: Unexpected error occurred.\n");
	}
}

/* Initialize the enclave:
 * call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void) {

    sgx_launch_token_t token = {0};
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    int updated = 0;

    /*
	    call sgx_create_enclave to initialize an enclave instance
      Debug Support: set 2nd parameter to 1
	  */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

int SGX_CDECL main(int argc, char *argv[]) {

    (void)(argc);
    (void)(argv);
//int addProve(string merkle, string prev, double *ln, string *cert) {

    blockHeader *header = (blockHeader *) malloc(sizeof(blockHeader));
    string merkle = "c7ea7a645ab4975cae7955a5f274cdedb5a1874a";
    string prev = "c7ea7a645ab4975cae7955a5f274cdedb5a1874a";
    memcpy(header->merkleTree, merkle.c_str(), 40);
    memcpy(header->prevBlock, prev.c_str(), 40);

    sgx_status_t status;

    /*Initialize the enclave*/
    if(initialize_enclave() < 0){
        return -1;
    }

    //Get lucky number
    uint8_t *certificate = (uint8_t *) malloc(sizeof(sgx_ec256_signature_t));

    ecall_get_proof(global_eid, &status, header, sizeof(blockHeader), certificate);
    if (status != SGX_SUCCESS) {
        print_error_message(status);
    } else {
        printf("(App) Headers Lucky number is: %f\n", header->luckyNumber);
    }

    bool a;
    string cert = base64_encode((const unsigned char*) certificate, sizeof(sgx_ec256_signature_t));
    a = verifyProof("c7ea7a645ab4975cae7955a5f274cdedb5a1874a", "c7ea7a645ab4975cae7955a5f274cdedb5a1874a", header->luckyNumber, cert);
    if (a == FALSE) {
      printf("ERROR!!\n");
      return -1;
    }
    /*Destroy the enclave*/
    sgx_destroy_enclave(global_eid);
    printf("Info: Enclave successfully returned.\n");
    return 0;
}

bool verifyProof(string merkle, string prevBlock, double luckyNumber, string certificate) {

    blockHeader *header = (blockHeader *) malloc(sizeof(blockHeader));
    memcpy(header->merkleTree, merkle.c_str(), 40);
    memcpy(header->prevBlock, prevBlock.c_str(), 40);
    header->luckyNumber = luckyNumber;

    uint8_t result;
    sgx_ecc_state_handle_t eccHandle;
    sgx_ec256_public_t *publicKey = (sgx_ec256_public_t *) malloc(sizeof(sgx_ec256_public_t));
    sgx_sha256_hash_t *hash = (sgx_sha256_hash_t *) malloc(sizeof(sgx_sha256_hash_t));

    sgx_ec256_signature_t *signature = (sgx_ec256_signature_t *) malloc(sizeof(sgx_ec256_signature_t));
    vector<unsigned char> data = base64_decode(certificate);
    /*if (data.size() != sizeof(sgx_ec256_signature_t)) {
      return SGX_ERROR_UNEXPECTED;
    }*/
    memcpy(signature, data.data(), sizeof(sgx_ec256_signature_t));

    sgx_status_t status = sgx_sha256_msg((uint8_t *) header, sizeof(blockHeader), hash);
    if (status != SGX_SUCCESS) {
        return FALSE;
    }

    status = sgx_ecc256_open_context(&eccHandle);
    if (status != SGX_SUCCESS) {
        return FALSE;
    }

    FILE *f = fopen("keys.txt", "r");
    if (f == NULL) {
        return FALSE;
    }

    char c;
    c = fgetc(f);
    while (c != EOF) {
        string publicKeyBase64 = "";
        while (c != '\n' && c != EOF) {
            publicKeyBase64 += c;
            c = fgetc(f);
        }
        c = fgetc(f);

        vector<unsigned char> publicKeyVector = base64_decode(publicKeyBase64);
        memcpy(publicKey, publicKeyVector.data(), sizeof(sgx_ec256_public_t));

        status = sgx_ecdsa_verify((uint8_t *) hash, sizeof(sgx_sha256_hash_t), publicKey, signature, &result, eccHandle);
        if (status != SGX_SUCCESS) {
            return FALSE;
        }
        if (result == SGX_EC_VALID) {
            status = sgx_ecc256_close_context(eccHandle);
            if (status != SGX_SUCCESS) {
                return FALSE;
            }
            return TRUE;
        }
    }
    fclose(f);
    status = sgx_ecc256_close_context(eccHandle);
    if (status != SGX_SUCCESS) {
        return FALSE;
    }
    return FALSE;
}

/*OCall print function*/
void ocall_print_string(const char *str) {
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */
    printf("%s", str);
}

/*OCall sleep function */
void ocall_sleep(unsigned int time) {

    printf("(APP) Waiting time %d\n", time);
    sleep(time);
}

/*OCall writes data to a file*/
int ocall_save_in_file(const char *filename, uint8_t *data, uint32_t sealedDataSize) {

    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        return -1;
    }
    fwrite(data, sealedDataSize, 1, f);
    fclose(f);
    return 1;
}

/*Ocall writes the public key to a .txt file*/
int ocall_save_public_key(uint8_t *publicKey, uint32_t dataSize) {

    string publicKeyBase64;
    char* data = (char *) malloc(dataSize);
    memcpy(data, publicKey, dataSize);
    //base64_encode the public key
    publicKeyBase64 = base64_encode((const unsigned char*) data, dataSize);
    /*
      Writes the public key to a .txt file for sharing the key with the network
    */
    FILE *f = fopen("publicKey.txt", "w");
    if (f == NULL) {
        return -1;
    }
    fprintf(f, "%s\n", publicKeyBase64.c_str());
    fclose(f);
    /*
      Writes the public key to a .txt file so that it can be used for verifying
      blocks. Keys from other network users must be log to this file by hand.
    */
    f = fopen("keys.txt", "a");
    if (f == NULL) {
        return -1;
    }
    fprintf(f, "%s\n", publicKeyBase64.c_str());
    fclose(f);
    return 1;
}

/*Ocall reads data from a file*/
struct fileData ocall_read_from_file(const char *filename) {

    fileData temp;
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
      temp.size = 0;
      return temp;
    }
    fseek(f, 0L, SEEK_END);
    temp.size = sizeof(uint8_t) * ftell(f);
    rewind(f);
    temp.data = malloc(temp.size);
    fread(temp.data, temp.size, 1, f);
    fclose(f);
    return temp;
}

//the follwing code was copied from the internet
std::string base64_encode(unsigned char const* buf, unsigned int bufLen) {
    size_t ret_size = bufLen+2;

    ret_size = 4*ret_size/3;

    std::string ret;
    ret.reserve(ret_size);

    for (unsigned int i=0; i<ret_size/4; ++i)
    {
        size_t index = i*3;
        unsigned char b3[3];
        b3[0] = buf[index+0];
        b3[1] = buf[index+1];
        b3[2] = buf[index+2];

        ret.push_back(to_base64[ ((b3[0] & 0xfc) >> 2) ]);
        ret.push_back(to_base64[ ((b3[0] & 0x03) << 4) + ((b3[1] & 0xf0) >> 4) ]);
        ret.push_back(to_base64[ ((b3[1] & 0x0f) << 2) + ((b3[2] & 0xc0) >> 6) ]);
        ret.push_back(to_base64[ ((b3[2] & 0x3f)) ]);
    }

    return ret;
}

//the follwing code was copied from the internet
std::vector<unsigned char> base64_decode(std::string encoded_string) {
    size_t encoded_size = encoded_string.size();
    std::vector<unsigned char> ret;
    ret.reserve(3*encoded_size/4);

    for (size_t i=0; i<encoded_size; i += 4)
    {
        unsigned char b4[4];
        b4[0] = from_base64[encoded_string[i+0]];
        b4[1] = from_base64[encoded_string[i+1]];
        b4[2] = from_base64[encoded_string[i+2]];
        b4[3] = from_base64[encoded_string[i+3]];

        unsigned char b3[3];
        b3[0] = ((b4[0] & 0x3f) << 2) + ((b4[1] & 0x30) >> 4);
        b3[1] = ((b4[1] & 0x0f) << 4) + ((b4[2] & 0x3c) >> 2);
        b3[2] = ((b4[2] & 0x03) << 6) + ((b4[3] & 0x3f));

        ret.push_back(b3[0]);
        ret.push_back(b3[1]);
        ret.push_back(b3[2]);
    }

    return ret;
}

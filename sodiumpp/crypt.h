// Copyright (c) 2014, Ruben De Visscher
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef CRYPT_H
#define CRYPT_H
#include <sodiumpp/sodiumpp.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sodiumpp/base64.h>
#include "libs/sha1.hpp"
using namespace sodiumpp;
using namespace std;

const string PUBLIC_KEY_PATH = "../keys/public_key.txt";
const string PRIVATE_KEY_PATH = "../keys/private_key.txt";

void createKeypair();
void getPublicUkey(unsigned char* key, int len);
void getPrivateUkey(unsigned char* key, int len);
string getPublicBkey();
string getPrivateBkey();
unsigned char* msgBaseToUchar(string msg, unsigned char* signed_message, int* len);
void printKey();
string signMsg(string rec, string send, int val);
bool verifySignature(string msg, string pk, string rec, int val, time_t timestamp);
#endif

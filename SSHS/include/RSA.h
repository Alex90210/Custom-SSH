#ifndef RSA_KEY_H
#define RSA_KEY_H

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>
#include "json_functions.h"

#define RSA_KEY_LEN 1024
#define RSA_KEY_SIZE (RSA_KEY_LEN / 8)

EVP_PKEY *generate_rsa_key();
std::string key_to_pem(EVP_PKEY *pkey, bool is_private);
EVP_PKEY* loadPrivateKeyFromJSON(const std::string& jsonFilePath);
std::string decryptWithPrivateKey(EVP_PKEY* privateKey, const char* encryptedData, size_t encryptedDataLen);

#endif
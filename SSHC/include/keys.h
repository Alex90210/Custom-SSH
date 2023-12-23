#ifndef KEYS_H
#define KEYS_H

#include "../include/string_operations.h"
#include "../include/json_functions.h"
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <iostream>

constexpr int KEY_LENGTH_BITS = 128;
constexpr int KEY_LENGTH_BYTES = KEY_LENGTH_BITS / 8;

bool generate_aes_key(unsigned char (&aesKey)[KEY_LENGTH_BYTES]);
void store_aes_key_b64(const std::string& base_64_encoded_key, const std::string& username);
bool receive_and_store_s_public_key(int socket, const std::string& jsonFilePath);
EVP_PKEY* loadPublicKeyFromJSON(const std::string& jsonFilePath);
std::string encryptWithPublicKey(EVP_PKEY* publicKey, const std::string& message);

#endif
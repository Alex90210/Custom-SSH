#ifndef KEYS_H
#define KEYS_H

#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include "utils.h"

using json = nlohmann::json;

constexpr int KEY_LENGTH_BITS = 256;
constexpr int KEY_LENGTH_BYTES = KEY_LENGTH_BITS / 8;

bool generate_aes_key(unsigned char (&aesKey)[KEY_LENGTH_BYTES]);
void store_aes_key_b64(const std::string& base_64_encoded_key, const std::string& username);
bool receive_and_store_s_public_key(int socket, const std::string& json_path);
EVP_PKEY* load_public_key_JSON(const std::string& json_path);
std::string encrypt_with_public_key(EVP_PKEY* public_key, const std::string& message);

#endif
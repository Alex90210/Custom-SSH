#ifndef AES_H
#define AES_H

#include <fstream>

#include "utils.h"
#include <openssl/evp.h>

std::string get_aes_key_from_json(const std::string& json_file_path, const std::string& username);
std::string aes_encrypt(const std::string& plaintext, const std::string& key);
std::string aes_decrypt(const std::string& ciphertext, const std::string& key);

#endif
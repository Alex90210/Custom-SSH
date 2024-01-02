#ifndef AES_H
#define AES_H

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

using json = nlohmann::json;

std::string get_aes_key_from_json(const std::string& json_path, const std::string& username);
std::string aes_encrypt(const std::string& plaintext, const std::string& key);
std::string aes_decrypt(const std::string& ciphertext, const std::string& key);

#endif
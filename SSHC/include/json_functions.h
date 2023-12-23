#ifndef JSON_FUNCTIONS_H
#define JSON_FUNCTIONS_H

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

json read_json(const std::string& filename);
bool authenticate_user(const json& j, const std::string& username, const std::string& password);

#endif
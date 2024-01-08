#ifndef JSON_FUNCTIONS_H
#define JSON_FUNCTIONS_H

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
using json = nlohmann::json;

void update_user_key(const std::string& filename, const std::string& username, const std::string& new_key);
void write_keys_to_json(const std::string &filename,
                        const std::string &private_key, const std::string &public_key);
json read_json(const std::string& filename);

#endif
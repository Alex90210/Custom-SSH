#ifndef JSON_FUNCTIONS_H
#define JSON_FUNCTIONS_H

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
using json = nlohmann::json;

void update_user_status(json& users, const std::string& username, const std::string& status);
void save_json(const json& j, const std::string& file_path);
json read_json(const std::string& filename);

#endif
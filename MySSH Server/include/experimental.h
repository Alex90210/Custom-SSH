#ifndef EXPERIMENTAL_H
#define EXPERIMENTAL_H

#include "server.h"

void update_user_status(json& users, const std::string& username, const std::string& status);
void save_json(const json& j, const std::string& file_path);
bool is_user_active(const std::string& username, const std::string& file_path);
std::vector<char> add_len_header2(const std::string& buffer);
void printHex(const char* data, int len);

#endif
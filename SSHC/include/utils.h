#ifndef CLIENT_FUNCTIONS_H
#define CLIENT_FUNCTIONS_H

#include <cstring>
#include <cctype>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <cstring>
#include <arpa/inet.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void print_AES_key(const std::string& filename);
void print_hex(const char* data, int len);
void print_message(const char* full_message, int total_len);
char* add_len_header(const char* buffer);
json read_json(const std::string& filename);
bool authenticate_user(const json& j, const std::string& username, const std::string& password);

#endif
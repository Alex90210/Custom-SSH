#ifndef CLIENT_FUNCTIONS_H
#define CLIENT_FUNCTIONS_H

#include <cstring>
#include <cctype>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void printAESKeys(const std::string& filename);
void printHex(const char* data, int len);
void printMessage(const char* fullMessage, int totalLen);
bool check_logged(const char* input_string, int size_of_string);
char* add_len_header(const char* buffer);
json read_json(const std::string& filename);
bool authenticate_user(const json& j, const std::string& username, const std::string& password);

#endif
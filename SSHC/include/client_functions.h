#ifndef CLIENT_FUNCTIONS_H
#define CLIENT_FUNCTIONS_H

#include "../include/json_functions.h"

void printAESKeys(const std::string& filename);
void printHex(const char* data, int len);
void printMessage(const char* fullMessage, int totalLen);
void command_loop(const int& sd, std::string& username);

#endif
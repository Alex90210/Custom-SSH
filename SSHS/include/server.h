#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include "client_handler.h"
#include "AES.h"
#include "base64.h"
#include "exec_command.h"

void verify_command(const char* fullMessage, struct thData tdL,
                    const std::string& username, std::string& path);
std::string receive_AES_key(int client_socket);
char* add_len_header(const char* buffer);
bool sendPublicKey(int cl, const std::string& jsonFilePath);
bool changeDirectory(const std::string& path);
std::string executeCommand(const std::string& command);

#endif
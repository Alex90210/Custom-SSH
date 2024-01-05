#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include "client_handler.h"
#include "AES.h"
#include "base64.h"
#include "exec_command.h"

void process_command(const char* client_msg, struct thData tdL,
                     const std::string& username, std::string& path);
std::string receive_AES_key(int client_socket);
bool send_public_key(int cl, const std::string& jsonFilePath);
char* add_len_header(const char* buffer);

#endif
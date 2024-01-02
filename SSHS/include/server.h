#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H

#include "client_handler.h"
#include "AES.h"
#include "base64.h"
#include "exec_command.h"

void process_command(const char* client_msg, struct thData tdL,
                     const std::string& username, std::string& path);
std::string receive_AES_key(int client_socket);
char* add_len_header(const char* buffer);
std::vector<char> add_len_header2(const std::string& buffer);
bool send_public_key(int cl, const std::string& jsonFilePath);
bool change_dir(const std::string& path);
std::string exec_command(const std::string& command);
void update_user_status(json& users, const std::string& username, const std::string& status);
json read_json(const std::string& filename);
void save_json(const json& j, const std::string& file_path);

#endif
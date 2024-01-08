#ifndef CLIENT_H
#define CLIENT_H

#include "AES.h"
#include "base64.h"
#include "utils.h"

void command_loop(const int& sd, std::string& username);

#endif
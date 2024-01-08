#ifndef WRITE_READ_H
#define WRITE_READ_H

#include <cstdio>
#include <unistd.h>
#include <string>

void read_msg_len(int cd, int& msg_len);
std::string read_msg (int sd, int message_len);

#endif
#ifndef READ_WRITE_H
#define READ_WRITE_H

#include <cstdio>
#include <unistd.h>
#include <string>

void read_msg_len(int sd, int& msg_len);
std::string read_msg (int sd, int message_len);

#endif

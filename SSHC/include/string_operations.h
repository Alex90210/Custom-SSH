#ifndef STRING_OPERATIONS_H
#define STRING_OPERATIONS_H

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


bool check_logged(const char* input_string, int size_of_string);
char* add_len_header(const char* buffer);

#endif
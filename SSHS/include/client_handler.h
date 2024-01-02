#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <pthread.h>
#include <string>
#include <iostream>
#include "server.h"
#include "../include/client_handler.h"
#include "../include/server.h"
#include "../include/AES.h"
#include "../include/base64.h"

typedef struct thData {
    int idThread;
    int cl;
} thData;

void *treat(void * arg);
void answer_client(void *arg);
#endif
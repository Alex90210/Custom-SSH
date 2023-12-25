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

typedef struct thData {
    int idThread;
    int cl; //descriptorul intors de accept
} thData;

// am eliminat static de aici, ar putea aparea probleme in viitor
void *treat(void * arg);
void raspunde(void *arg);

#endif
#include "../include/read_write.h"

void read_msg_len(int sd, int& msg_len) {
    ssize_t bytes_read = read(sd, &msg_len, sizeof(int));
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            printf("The server closed the connection.");
        } else {
            perror("Error reading message length header");
        }
        close(sd);
    }
}

std::string read_msg (int sd, int message_len) {
    ssize_t bytesRead = 0;
    char* server_output = new char[message_len + 1];
    while (bytesRead < message_len) {
        ssize_t result = read(sd, server_output + bytesRead, message_len - bytesRead);
        if (result <= 0) {
            if (result == 0) {
                printf("Client closed the connection.\n");
            } else {
                perror("Error reading message");
            }
            delete[] server_output;
            perror("Error reading message");
            return nullptr;
        }
        bytesRead += result;
    }

    server_output[message_len] = '\0';
    if (bytesRead > 0 && server_output[bytesRead - 1] == '\n') {
        server_output[bytesRead - 1] = '\0';
    }

    std::string server_msg = server_output;
    delete[] server_output;

    return server_msg;
}
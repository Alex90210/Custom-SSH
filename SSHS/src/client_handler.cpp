#include "../include/client_handler.h"

const std::string json_path {"../server_data.json"};

void* treat(void * arg) {
    struct thData tdL;
    tdL= *((struct thData*)arg);
    printf ("[Thread]- %d - Waiting for the message...\n", tdL.idThread);
    fflush (stdout);
    pthread_detach(pthread_self());

    // Instantly sending the public key
    printf("[Thread]- %d - Sending public key...\n", tdL.idThread);
    fflush(stdout);

    if (!send_public_key(tdL.cl, json_path)) {
        std::cerr << "Failed to send public key" << std::endl;
        close(tdL.cl);
        return NULL;
    }

    answer_client((struct thData*)arg);
    close ((intptr_t)arg);

    return(nullptr);
};

void printHex(const char* data, int len) {
    for (int i = 0; i < len; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(data[i]) & 0xff) << " ";
    }
    std::cout << std::dec << std::endl; // Switch back to decimal format
}

void answer_client(void *arg) {

    int thread_id;
    struct thData tdL;
    tdL= *((struct thData*)arg);
    std::string current_path = "/home/alex";

    json users = read_json(json_path);
    std::string user = receive_AES_key(tdL.cl);
    /*if (user == "User already active.") {
        // Kill both processes and return error to both processes
        close(tdL.cl);
        update_user_status(users, user, "offline");
        save_json(users, "../server_data.json");
    }*/

    while (1) {

        int message_len {};
        ssize_t bytes_read = read(tdL.cl, &message_len, sizeof(int));
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                printf("[Thread %d] Client closed the connection.\n", tdL.idThread);
                /*update_user_status(users, user, "offline");
                save_json(users, "../server_data.json");*/
            } else {
                perror("[Thread] Error reading message length header");
            }
            close(tdL.cl);
            return;
        }

        char* client_input = new char[message_len + 1];

        ssize_t bytesRead = 0;
        while (bytesRead < message_len) {
            ssize_t result = read(tdL.cl, client_input + bytesRead, message_len - bytesRead);
            if (result <= 0) {
                if (result == 0) {
                    printf("Client closed the connection.\n");
                } else {
                    perror("Error reading message");
                }
                free(client_input);
                return;
            }
            bytesRead += result;
        }

        client_input[message_len] = '\0';
        if (bytesRead > 0 && client_input[bytesRead - 1] == '\n') {
            client_input[bytesRead - 1] = '\0';
        }

        message_len = strlen(client_input);
        printf("Length of message: %d\n", message_len);
        printf("[Thread %d]Received message (b64 encoded): %s\n", tdL.idThread, client_input);
        printf("[Thread %d]Processing the command and sending it back...%d\n",
               tdL.idThread, thread_id);

        process_command(client_input, tdL, user, current_path);
        delete[] client_input;
    }
}
#include "../include/client_handler.h"

const std::string json_path {"../server_data.json"};

void* treat(void* arg) {
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

        ssize_t cl_input_bytes_read = 0;
        while (cl_input_bytes_read < message_len) {
            ssize_t result = read(tdL.cl, client_input + cl_input_bytes_read, message_len - cl_input_bytes_read);
            if (result <= 0) {
                if (result == 0) {
                    printf("Client closed the connection.\n");
                } else {
                    perror("Error reading message");
                }
                free(client_input);
                return;
            }
            cl_input_bytes_read += result;
        }

        client_input[message_len] = '\0';
        if (cl_input_bytes_read > 0 && client_input[cl_input_bytes_read - 1] == '\n') {
            client_input[cl_input_bytes_read - 1] = '\0';
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
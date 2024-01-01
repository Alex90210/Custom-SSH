#include "../include/client_handler.h"
#include "../include/server.h"
#include "../include/AES.h"
#include "../include/base64.h"


// am eliminat static de aici, ar putea aparea probleme in viitor
void *treat(void * arg) {
    struct thData tdL;
    tdL= *((struct thData*)arg);
    printf ("[Thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush (stdout);
    pthread_detach(pthread_self());

    // INSTANTLY SENDING THE RSA PUBLIC KEY

    printf("[Thread] - %d - Sending public key...\n", tdL.idThread);
    fflush(stdout);

    if (!sendPublicKey(tdL.cl, "../server_data.json")) {
        std::cerr << "Failed to send public key" << std::endl;
        close(tdL.cl);
        return NULL;
    }

    // -------------------------------------

    answer_client((struct thData*)arg);
    close ((intptr_t)arg);
    return(NULL);
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

    std::string user = receive_AES_key(tdL.cl);
    // std::string current_path = get_current_directory();
    std::string current_path = "/home/alex";
    while (1) {
        int message_len {};
        ssize_t bytes_read = read(tdL.cl, &message_len, sizeof(int));
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                printf("[Thread %d] Client closed the connection.\n", tdL.idThread);
            } else {
                perror("[Thread] Error reading message length header");
            }
            close(tdL.cl);
            return;
        }

        // Allocate memory for the full message and an extra byte for the null terminator
        // Change malloc with new
        char *client_input = (char *)malloc(message_len + 1);
        if (client_input == NULL) {
            perror("Error allocating memory for full message");
            return;
        }

        // Read the actual message
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
        free(client_input);
    }
}
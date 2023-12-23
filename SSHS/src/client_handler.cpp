#include "../include/client_handler.h"
#include "../include/server_functions.h"
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

    raspunde((struct thData*)arg);
    close ((intptr_t)arg);
    return(NULL);
};

void printHex(const char* data, int len) {
    for (int i = 0; i < len; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(data[i]) & 0xff) << " ";
    }
    std::cout << std::dec << std::endl; // Switch back to decimal format
}

void raspunde(void *arg) {

    int nr;
    struct thData tdL;
    tdL= *((struct thData*)arg);

    char message_r[1000];

    std::string user = receive_AES_key(tdL.cl);

    while (1) {

        char buf[1000];
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
        char *fullMessage = (char *)malloc(message_len + 1); // +1 for null terminator
        if (fullMessage == NULL) {
            perror("Error allocating memory for full message");
            return;
        }

        // Read the actual message
        ssize_t bytesRead = 0;
        while (bytesRead < message_len) {
            ssize_t result = read(tdL.cl, fullMessage + bytesRead, message_len - bytesRead);
            if (result <= 0) {
                if (result == 0) {
                    printf("Client closed the connection.\n");
                } else {
                    perror("Error reading message");
                }
                free(fullMessage);
                return;
            }
            bytesRead += result;
        }

        fullMessage[message_len] = '\0';

        if (bytesRead > 0 && fullMessage[bytesRead - 1] == '\n') {
            fullMessage[bytesRead - 1] = '\0';
        }

        std::string string_cipther = fullMessage;
        // DECODE THE MESSAGE
        std::string b64_aes_key = get_aes_key_from_json("../server_data.json", user);
        std::cout << "BASE 64 KEY: " << b64_aes_key << std::endl;

        // Decode the AES key
        std::string aes_key = base64_decode(b64_aes_key);
        std::cout << "KEY: " << aes_key << std::endl;

        // Assuming fullMessage is the Base64-encoded ciphertext received from the client
        std::cout << "BASE 64 CIPHERTEXT: " << string_cipther << std::endl;
        std::string encryptedMessage = base64_decode(string_cipther);
        std::cout << "CIPHERTEXT: " << encryptedMessage << std::endl;

        /*// Decrypt the message
        std::string plaintext = aes_decrypt(encryptedMessage, aes_key);
        std::cout << "DECRYPTED TEXT: " << plaintext << std::endl;*/

        message_len = strlen(fullMessage);
        printf("SIZE_OF_MESSAGE: %d\n", message_len);
        printf("[Thread %d] Received message: %s\n", tdL.idThread, encryptedMessage.c_str());

        printf("[Thread %d]Trimitem mesajul inapoi...%d\n", tdL.idThread, nr);

        verify_command(fullMessage, tdL, user);
        free(fullMessage);
    }
}
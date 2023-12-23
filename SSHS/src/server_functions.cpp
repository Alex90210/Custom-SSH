#include "../include/server_functions.h"
#include "../include/json_functions.h"
#include "../include/rsa_key.h"

void verify_command(const char* fullMessage, struct thData tdL, const std::string& username) {
    if (strcmp(fullMessage, "login") == 0) {
        char message_s[] = "Comanda 'Login' a fost recunoscuta.";

        if (write(tdL.cl, &message_s, strlen(message_s) + 1) <= 0) {
            printf("[Thread %d] ", tdL.idThread);
            perror("[Thread]Eroare la write() catre client.\n");
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

    } else if (strcmp(fullMessage, "create account") == 0) {

        char message_s[] = "Comanda 'Create Account' a fost recunoscuta.";
        if (write(tdL.cl, &message_s, strlen(message_s) + 1) <= 0) {
            printf("[Thread %d] ", tdL.idThread);
            perror("[Thread]Eroare la write() catre client.\n");
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

    } else if (strcmp(fullMessage, "show public key") == 0) {

        char message_s[] = "Comanda 'Show Public Key' a fost recunoscuta.";
        if (write(tdL.cl, &message_s, strlen(message_s) + 1) <= 0) {
            printf("[Thread %d] ", tdL.idThread);
            perror("[Thread]Eroare la write() catre client.\n");
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

    } else if (strcmp(fullMessage, "show private key") == 0) {

        char message_s[] = "Comanda 'Show Private Key' a fost recunoscuta.";
        if (write(tdL.cl, &message_s, strlen(message_s) + 1) <= 0) {
            printf("[Thread %d] ", tdL.idThread);
            perror("[Thread]Eroare la write() catre client.\n");
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);

    } else {

        // const char *baseMessage = "Comanda nerecunoscuta: ";
        // int baseMessageLen = strlen(baseMessage);
        /*fullMessage = "mesaj DE LA SERVER";
        std::string b64_aes_key = get_aes_key_from_json("../server_data.json", username);
        std::string aes_key = base64_decode(b64_aes_key);

        std::string cripted = aes_encrypt(fullMessage, aes_key);
        cripted = base64_encode(reinterpret_cast<const unsigned char *>(cripted.c_str()), cripted.length());*/

        int fullMessageLen = strlen(fullMessage);

        // Allocate memory for the response
        char *responseMessage = (char *) malloc(fullMessageLen + 1); // +1 for null terminator
        if (responseMessage == NULL) {
            perror("Error allocating memory for response message");
            // Handle the error (e.g., return, close connection, etc.)
        }

        // Initialize the response message
        responseMessage[0] = '\0'; // Add this line to initialize the memory

        // Build the response message
        // If you want to use baseMessage as well, uncomment and use strcpy and strcat as follows:
        // strcpy(responseMessage, baseMessage);
        strcat(responseMessage, fullMessage);

        if (write(tdL.cl, responseMessage, strlen(responseMessage)) <= 0) {
            printf("[Thread %d] ", tdL.idThread);
            perror("[Thread]Eroare la write() catre client.\n");
        } else {
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
        }

        free(responseMessage);

    }
}

std::string receive_AES_key(int client_socket) {
    int username_len, key_len;

    read(client_socket, &username_len, sizeof(int));
    read(client_socket, &key_len, sizeof(int));

    // Allocate memory for the username and key
    // char* decrypted_username = new char[username_len + 1]; // not necesary because I am using outlen
    // char* decrypted_aes_key = new char[key_len + 1]; // not necesary because I am using outlen
    char* encrypted_username = new char[RSA_KEY_SIZE + 1];
    char* encrypted_aes_key = new char[RSA_KEY_SIZE + 1];

    // Read the username and key
    read(client_socket, encrypted_username, RSA_KEY_SIZE);
    encrypted_username[RSA_KEY_SIZE] = '\0';

    read(client_socket, encrypted_aes_key, RSA_KEY_SIZE);
    encrypted_aes_key[RSA_KEY_SIZE] = '\0';

    // DECRYPT THE USERNAME AND THE ASE KEY

    // Load the server's private key
    EVP_PKEY* privateKey = loadPrivateKeyFromJSON("../server_data.json");
    if (!privateKey) {
        std::cerr << "Error loading private key." << std::endl;
    }

    std::string decryptedUsername = decryptWithPrivateKey(privateKey, encrypted_username, RSA_KEY_SIZE);
    std::string decryptedAesKey = decryptWithPrivateKey(privateKey, encrypted_aes_key, RSA_KEY_SIZE);

    decryptedUsername.resize(username_len);
    decryptedAesKey.resize(key_len);

    update_user_key("../server_data.json", std::string(decryptedUsername),
                    std::string(decryptedAesKey));

    // delete[] decrypted_username;
    // delete[] decrypted_aes_key;
    delete[] encrypted_username;
    delete[] encrypted_aes_key;

    return decryptedUsername;
}

char* add_len_header(const char* buffer) {

    int string_len = strlen(buffer);
    char *full_message = (char *)malloc(sizeof(int) + string_len);
    if (full_message == NULL) {
        perror("Error allocating memory for full message");
    }

    memcpy(full_message, &string_len, sizeof(int));
    memcpy(full_message + sizeof(int), buffer, string_len);

    return full_message;
}

bool sendPublicKey(int cl, const std::string& jsonFilePath) {
    std::ifstream ifs(jsonFilePath);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonFilePath << std::endl;
        return false;
    }

    json j;
    ifs >> j;
    ifs.close();

    std::string publicKey = j["server keys"][0]["public key"];
    if (publicKey.empty()) {
        std::cerr << "Public key not found in JSON file" << std::endl;
        return false;
    }

    char* full_message = add_len_header(publicKey.c_str());
    if (full_message == NULL) {
        return false;
    }

    int total_len = sizeof(int) + publicKey.size();
    if (write(cl, full_message, total_len) < 0) {
        std::cerr << "Error sending public key" << std::endl;
        free(full_message);
        return false;
    }

    free(full_message);
    return true;
}
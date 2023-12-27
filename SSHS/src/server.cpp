#include "../include/server.h"
#include "../include/json_functions.h"
#include "../include/RSA.h"

void process_command(const char* fullMessage, struct thData tdL,
                     const std::string& username, std::string& path) {

    // The header containing the message size should be received in the previous function
    // You could get its size from a parameter
    int full_message_len = strlen(fullMessage);
    char *response_message = new char[full_message_len + 1];
    response_message[0] = '\0';
    strcat(response_message, fullMessage);
    printf("[Server]Received from client: %s\n", response_message);

    // Getting the key
    std::string b64_aes_key = get_aes_key_from_json("../server_data.json", username);
    std::string aes_key = base64_decode(b64_aes_key);

    // Decrypt the message
    std::string ciphertext = base64_decode(response_message);
    std::string decrypted_ciphertext = aes_decrypt(ciphertext, aes_key);
    std::cout << "[Server]The decrypted message is: " << decrypted_ciphertext << std::endl;

    // Interpret the command
    std::string command_output;
    command_output = interpret_command(decrypted_ciphertext, path);

    // Encrypt the message
    std::string encrypted = aes_encrypt(command_output, aes_key);
    encrypted = base64_encode(encrypted);

    // Send the message.
    char* full_message = add_len_header(encrypted.c_str());
    int total_length = encrypted.length() + sizeof(int);

    // Write the message with the length header to the client
    if (write(tdL.cl, full_message, total_length) <= 0) {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread] Error at writing the message to the client.\n");
    } else {
        printf("[Thread %d] The message was sent successfully.\n", tdL.idThread);
    }

    delete[] response_message;
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

    update_user_key("/home/alex/Desktop/SSH/SSHS/server_data.json", std::string(decryptedUsername),
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

std::vector<char> add_len_header2(const std::string& buffer) {
    int string_len = buffer.size();

    // Convert the length to a binary representation
    std::vector<char> length_bytes(sizeof(int));
    std::memcpy(length_bytes.data(), &string_len, sizeof(int));

    // Create a vector to hold the length bytes followed by the string data
    std::vector<char> full_message;
    full_message.reserve(sizeof(int) + string_len);

    // Append length bytes and string data
    full_message.insert(full_message.end(), length_bytes.begin(), length_bytes.end());
    full_message.insert(full_message.end(), buffer.begin(), buffer.end());

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
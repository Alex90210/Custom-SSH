#include "../include/server.h"
#include "../include/json_functions.h"
#include "../include/RSA.h"

void process_command(const char* client_msg, struct thData tdL,
                     const std::string& username, std::string& path) {

    int full_message_len = strlen(client_msg);
    char *response_message = new char[full_message_len + 1];
    response_message[0] = '\0';
    strcat(response_message, client_msg);
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

    // Adding the path to the message
    std::string output_with_path = path + "@@@" + command_output;

    // Encrypt the message
    std::string encrypted = aes_encrypt(output_with_path, aes_key);
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

void update_user_status(json& users, const std::string& username, const std::string& status) {
    for (auto& user : users["users"]) {
        if (user["username"] == username) {
            user["status"] = status;
            break;
        }
    }
}

void save_json(const json& j, const std::string& file_path) {
    std::ofstream file(file_path);
    if (file.is_open()) {
        file << j.dump(4); // 4 is for pretty printing
        file.close();
    } else {
        std::cerr << "Could not open file for writing.\n";
    }
}

json read_json(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    json j;
    try {
        ifs >> j;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Parse error in file: " + filename + "; " + e.what());
    }

    return j;
}

bool is_user_active(const std::string& username, const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Unable to open file." << std::endl;
        return false;
    }

    json j;
    file >> j;

    for (const auto& user : j["users"]) {
        if (user["username"] == username && user["status"] == "active") {
            return true;
        }
    }
    return false;
}
std::string receive_AES_key(int client_socket) {
    int username_len, key_len;

    read(client_socket, &username_len, sizeof(int));
    read(client_socket, &key_len, sizeof(int));

    char* encrypted_username = new char[RSA_KEY_SIZE + 1];
    char* encrypted_aes_key = new char[RSA_KEY_SIZE + 1];

    // Read the username and key
    read(client_socket, encrypted_username, RSA_KEY_SIZE);
    encrypted_username[RSA_KEY_SIZE] = '\0';

    read(client_socket, encrypted_aes_key, RSA_KEY_SIZE);
    encrypted_aes_key[RSA_KEY_SIZE] = '\0';

    // Load the server's private key
    EVP_PKEY* privateKey = load_private_key_from_json("../server_data.json");
    if (!privateKey) {
        std::cerr << "Error loading private key." << std::endl;
    }

    std::string decryptedUsername = decryptWithPrivateKey(privateKey, encrypted_username, RSA_KEY_SIZE);
    std::string decryptedAesKey = decryptWithPrivateKey(privateKey, encrypted_aes_key, RSA_KEY_SIZE);

    decryptedUsername.resize(username_len);
    update_user_key("/home/alex/Desktop/SSH/SSHS/server_data.json", std::string(decryptedUsername),
                    std::string(decryptedAesKey));

    // Updating the status of the user
    /*json users = read_json("../server_data.json");
    if (is_user_active(decryptedUsername, "../server_data.json")) {
        return "User already active.";
    }
    else {
        update_user_status(users, decryptedUsername, "active");
        save_json(users, "../server_data.json");
        decryptedAesKey.resize(key_len);
        update_user_key("/home/alex/Desktop/SSH/SSHS/server_data.json", std::string(decryptedUsername),
                        std::string(decryptedAesKey));
    }*/

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

bool send_public_key(int cl, const std::string& jsonFilePath) {
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

    int total_len = sizeof(int) + publicKey.size();
    if (write(cl, full_message, total_len) < 0) {
        std::cerr << "Error sending public key" << std::endl;
        free(full_message);
        return false;
    }

    free(full_message);
    return true;
}
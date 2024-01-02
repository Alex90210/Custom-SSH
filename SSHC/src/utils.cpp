#include "../include/utils.h"
#include "../include/AES.h"
#include "../include/base64.h"

void print_AES_key(const std::string& filename) {
    // Read the JSON file
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open JSON file: " << filename << std::endl;
        return;
    }

    // Parse the JSON content
    nlohmann::json json;
    try {
        ifs >> json;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        return;
    }

    // Extract and print AES keys
    if (json.find("users") != json.end()) {
        for (const auto& user : json["users"]) {
            if (user.find("aes_key") != user.end()) {
                std::string aesKey = user["aes_key"];
                std::cout << "AES Key: " << aesKey << std::endl;
            }
        }
    }
}

void print_hex(const char* data, int len) {
    for (int i = 0; i < len; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(data[i]) & 0xff) << " ";
    }
    std::cout << std::dec << std::endl; // Switch back to decimal format
}

void print_message(const char* full_message, int total_len) {
    int encryptedUsernameLen, encryptedAesKeyLen;
    memcpy(&encryptedUsernameLen, full_message, sizeof(int));
    memcpy(&encryptedAesKeyLen, full_message + sizeof(int), sizeof(int));

    std::cout << "Encrypted Username Length: " << encryptedUsernameLen << std::endl;
    std::cout << "Encrypted AES Key Length: " << encryptedAesKeyLen << std::endl;

    // Print the encrypted username in hex
    std::cout << "Encrypted Username (Hex): ";
    print_hex(full_message + sizeof(int) * 2, encryptedUsernameLen);

    // Print the encrypted AES key in hex
    std::cout << "Encrypted AES Key (Hex): ";
    print_hex(full_message + sizeof(int) * 2 + encryptedUsernameLen, encryptedAesKeyLen);
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

bool authenticate_user(const json& j, const std::string& username, const std::string& password) {
    for (const auto& user : j["users"]) {
        if (user["username"] == username && user["password"] == password) {
            return true;
        }
    }
    return false;
}
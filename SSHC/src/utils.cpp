#include "../include/utils.h"
#include "../include/AES.h"
#include "../include/base64.h"

void printAESKeys(const std::string& filename) {
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

void printHex(const char* data, int len) {
    for (int i = 0; i < len; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(data[i]) & 0xff) << " ";
    }
    std::cout << std::dec << std::endl; // Switch back to decimal format
}

void printMessage(const char* fullMessage, int totalLen) {
    // Print the lengths
    // maybe the username is encrypted using a less complex key
    // 2 different headers are not that bad
    // 2 different keys are needed though

    int encryptedUsernameLen, encryptedAesKeyLen;
    memcpy(&encryptedUsernameLen, fullMessage, sizeof(int));
    memcpy(&encryptedAesKeyLen, fullMessage + sizeof(int), sizeof(int));

    std::cout << "Encrypted Username Length: " << encryptedUsernameLen << std::endl;
    std::cout << "Encrypted AES Key Length: " << encryptedAesKeyLen << std::endl;

    // Print the encrypted username in hex
    std::cout << "Encrypted Username (Hex): ";
    printHex(fullMessage + sizeof(int) * 2, encryptedUsernameLen);

    // Print the encrypted AES key in hex
    std::cout << "Encrypted AES Key (Hex): ";
    printHex(fullMessage + sizeof(int) * 2 + encryptedUsernameLen, encryptedAesKeyLen);
}

bool check_logged(const char* input_string, int size_of_string) {
    if (input_string == nullptr || size_of_string <= 0) {
        return false;
    }

    char* temp = new char[size_of_string];
    int i = 0;
    while (input_string[i] != '\0' && i < size_of_string - 1) {
        temp[i] = std::tolower(static_cast<unsigned char>(input_string[i]));
        i++;
    }
    temp[i] = '\0';

    bool result = strcmp(temp, "login") == 0;

    delete[] temp;
    return result;
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

bool authenticate_user(const json& j, const std::string& username, const std::string& password) {
    for (const auto& user : j["users"]) {
        if (user["username"] == username && user["password"] == password) {
            return true;
        }
    }
    return false;
}
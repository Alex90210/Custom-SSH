#include "../include/experimental.h"

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

void printHex(const char* data, int len) {
    for (int i = 0; i < len; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(data[i]) & 0xff) << " ";
    }
    std::cout << std::dec << std::endl; // Switch back to decimal format
}
#include "../include/json_functions.h"

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
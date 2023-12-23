#include "../include/json_functions.h"

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
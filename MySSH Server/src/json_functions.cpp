#include "../include/json_functions.h"

void update_user_key(const std::string& filename, const std::string& username, const std::string& new_key) {
    std::ifstream ifs(filename);
    nlohmann::json j;
    ifs >> j;

    bool user_found {false};
    for (auto& user : j["users"]) {
        if (user["username"] == username) {
            user["aes_key"] = new_key;
            user_found = true;
            break;
        }
    }

    if (!user_found)
        std::cerr << "[Update key function] User does not exist" << std::endl;
    else
        std::cout << "The ASE key of " << username << " has been update successfully." << std::endl;

    std::ofstream ofs(filename);
    ofs << j.dump(4);
}

void write_keys_to_json(const std::string &filename, const std::string &private_key, const std::string &public_key) {
    std::ifstream ifs(filename);
    json j;
    ifs >> j;
    ifs.close();

    j["server keys"][0]["private key"] = private_key;
    j["server keys"][0]["public key"] = public_key;

    std::ofstream ofs(filename);
    ofs << j.dump(4);
    ofs.close();
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
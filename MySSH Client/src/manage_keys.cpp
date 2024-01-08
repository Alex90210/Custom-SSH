#include "../include/manage_keys.h"

bool generate_aes_key(unsigned char (&aesKey)[KEY_LENGTH_BYTES]) {
    OpenSSL_add_all_algorithms();

    // Generate a random AES key
    if (RAND_bytes(aesKey, KEY_LENGTH_BYTES) != 1) {
        std::cerr << "Error generating AES key." << std::endl;
        return false;
    }

    return true;
}

void store_aes_key_b64(const std::string& base_64_encoded_key, const std::string& username) {
    json users = read_json("../client_data.json");
    bool user_found = false;
    for (auto& user : users["users"]) {
        if (user["username"] == username) {
            user["aes_key"] = base_64_encoded_key;
            user_found = true;
            break;
        }
    }

    if (user_found) {
        std::ofstream file("../client_data.json");
        if (file.is_open()) {
            file << users.dump(4); // Write formatted JSON
            file.close();
        } else {
            std::cerr << "Failed to open file for writing." << std::endl;
        }
    }
}

bool receive_and_store_s_public_key(int socket, const std::string& json_path) {
    // Read the size of the public key
    int key_size;
    if (read(socket, &key_size, sizeof(key_size)) <= 0) {
        std::cerr << "Error reading public key size" << std::endl;
        return false;
    }

    // Prepare a string to hold the public key
    std::string public_key(key_size, '\0');
    if (read(socket, &public_key[0], key_size) <= 0) {
        std::cerr << "Error reading public key" << std::endl;
        return false;
    }

    // Load the client's JSON file
    std::ifstream ifs(json_path);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open JSON file: " << json_path << std::endl;
        return false;
    }
    json j;
    ifs >> j;
    ifs.close();

    // Update the JSON object with the new public key
    j["server key"][0]["public key"] = public_key;

    // Write the updated JSON object back to the file
    std::ofstream ofs(json_path);
    if (!ofs.is_open()) {
        std::cerr << "Failed to open JSON file for writing: " << json_path << std::endl;
        return false;
    }
    ofs << j.dump(4);  // Pretty printing with 4 spaces indentation
    ofs.close();

    return true;
}

EVP_PKEY* load_public_key_JSON(const std::string& json_path) {
    std::ifstream ifs(json_path);
    if (!ifs) {
        std::cerr << "Cannot open JSON file: " << json_path << std::endl;
        return nullptr;
    }

    json j;
    ifs >> j;
    ifs.close();

    std::string pemPublicKey;
    try {
        pemPublicKey = j["server key"][0]["public key"].get<std::string>();
    } catch (const json::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return nullptr;
    }

    if (pemPublicKey.empty()) {
        std::cerr << "Public key not found in JSON file" << std::endl;
        return nullptr;
    }

    BIO* bio = BIO_new_mem_buf(pemPublicKey.c_str(), -1);
    EVP_PKEY* publicKey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!publicKey) {
        std::cerr << "Error loading public key from PEM string" << std::endl;
    }

    return publicKey;
}

std::string encrypt_with_public_key(EVP_PKEY* public_key, const std::string& message) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(public_key, nullptr);
    if (!ctx) {
        std::cerr << "Error creating context for encryption: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        return "";
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        std::cerr << "Error initializing public key encryption: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    // Determine buffer length for the encrypted data
    size_t outlen;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, reinterpret_cast<const unsigned char*>(message.c_str()), message.length()) <= 0) {
        std::cerr << "Error determining buffer length: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    std::string encrypted(outlen, '\0');
    if (EVP_PKEY_encrypt(ctx, reinterpret_cast<unsigned char*>(&encrypted[0]), &outlen, reinterpret_cast<const unsigned char*>(message.c_str()), message.length()) <= 0) {
        std::cerr << "Error during encryption: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    EVP_PKEY_CTX_free(ctx);
    return encrypted;
}

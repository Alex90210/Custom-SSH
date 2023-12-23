#include "../include/keys.h"

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
    bool userFound = false;
    for (auto& user : users["users"]) {
        if (user["username"] == username) {
            user["aes_key"] = base_64_encoded_key;
            userFound = true;
            break;
        }
    }

    if (userFound) {
        std::ofstream file("../client_data.json");
        if (file.is_open()) {
            file << users.dump(4); // Write formatted JSON
            file.close();
        } else {
            std::cerr << "Failed to open file for writing." << std::endl;
        }
    }
}

bool receive_and_store_s_public_key(int socket, const std::string& jsonFilePath) {
    // Read the size of the public key
    int keySize;
    if (read(socket, &keySize, sizeof(keySize)) <= 0) {
        std::cerr << "Error reading public key size" << std::endl;
        return false;
    }

    // Prepare a string to hold the public key
    std::string publicKey(keySize, '\0');
    if (read(socket, &publicKey[0], keySize) <= 0) {
        std::cerr << "Error reading public key" << std::endl;
        return false;
    }

    // Load the client's JSON file
    std::ifstream ifs(jsonFilePath);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonFilePath << std::endl;
        return false;
    }
    json j;
    ifs >> j;
    ifs.close();

    // Update the JSON object with the new public key
    j["server key"][0]["public key"] = publicKey;

    // Write the updated JSON object back to the file
    std::ofstream ofs(jsonFilePath);
    if (!ofs.is_open()) {
        std::cerr << "Failed to open JSON file for writing: " << jsonFilePath << std::endl;
        return false;
    }
    ofs << j.dump(4);  // Pretty printing with 4 spaces indentation
    ofs.close();

    return true;
}

EVP_PKEY* loadPublicKeyFromJSON(const std::string& jsonFilePath) {
    std::ifstream ifs(jsonFilePath);
    if (!ifs) {
        std::cerr << "Cannot open JSON file: " << jsonFilePath << std::endl;
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

std::string encryptWithPublicKey(EVP_PKEY* publicKey, const std::string& message) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(publicKey, nullptr);
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

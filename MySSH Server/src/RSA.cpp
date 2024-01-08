#include <openssl/core_names.h>
#include "../include/RSA.h"

EVP_PKEY* generate_rsa_key() {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        std::cerr << "Error creating context: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        return nullptr;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        std::cerr << "Error initializing keygen: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    // Set the number of bits for the RSA key
    unsigned int rsa_bits = 1024;
    BIGNUM* bn_pub_exp = BN_new();
    if (!bn_pub_exp) {
        std::cerr << "Error creating BIGNUM for public exponent: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }
    BN_set_word(bn_pub_exp, RSA_F4);  // RSA_F4 is a common choice for e (0x10001)

    // Convert the public exponent to binary
    size_t bn_pub_exp_len = BN_num_bytes(bn_pub_exp);
    unsigned char* bn_pub_exp_bin = new unsigned char[bn_pub_exp_len];
    BN_bn2bin(bn_pub_exp, bn_pub_exp_bin);

    OSSL_PARAM params[] = {
            OSSL_PARAM_construct_uint(OSSL_PKEY_PARAM_RSA_BITS, &rsa_bits),
            OSSL_PARAM_construct_BN(OSSL_PKEY_PARAM_RSA_E, bn_pub_exp_bin, bn_pub_exp_len),
            OSSL_PARAM_construct_end()
    };

    if (EVP_PKEY_CTX_set_params(ctx, params) <= 0) {
        std::cerr << "Error setting keygen parameters: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        delete[] bn_pub_exp_bin;
        EVP_PKEY_CTX_free(ctx);
        BN_free(bn_pub_exp);
        return nullptr;
    }

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        std::cerr << "Error generating key: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
    }

    delete[] bn_pub_exp_bin;
    EVP_PKEY_CTX_free(ctx);
    BN_free(bn_pub_exp);
    return pkey;
}

std::string key_to_pem(EVP_PKEY *pkey, bool is_private) {
    BIO *bio = BIO_new(BIO_s_mem());
    if (is_private) {
        PEM_write_bio_PrivateKey(bio, pkey, nullptr, nullptr, 0, nullptr, nullptr);
    } else {
        PEM_write_bio_PUBKEY(bio, pkey);
    }

    char *key_data;
    long key_length = BIO_get_mem_data(bio, &key_data);

    std::string key_str(key_data, key_length);
    BIO_free(bio);

    return key_str;
}

EVP_PKEY* load_private_key_from_json(const std::string& json_path) {
    std::ifstream ifs(json_path);
    if (!ifs) {
        std::cerr << "Cannot open JSON file: " << json_path << std::endl;
        return nullptr;
    }

    json j;
    ifs >> j;
    ifs.close();

    std::string pemPrivateKey;
    try {
        pemPrivateKey = j["server keys"][0]["private key"].get<std::string>();
    } catch (const json::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return nullptr;
    }

    if (pemPrivateKey.empty()) {
        std::cerr << "Private key not found in JSON file" << std::endl;
        return nullptr;
    }

    BIO* bio = BIO_new_mem_buf(pemPrivateKey.c_str(), -1);
    EVP_PKEY* privateKey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!privateKey) {
        std::cerr << "Error loading private key from PEM string" << std::endl;
    }

    return privateKey;
}

std::string decryptWithPrivateKey(EVP_PKEY* private_key, const char* encrypted_data, size_t encrypted_data_len) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(private_key, nullptr);
    if (!ctx) {
        std::cerr << "Error creating context for decryption" << std::endl;
        return "";
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        std::cerr << "Error initializing decryption" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    size_t outlen;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, reinterpret_cast<const unsigned char*>(encrypted_data), encrypted_data_len) <= 0) {
        std::cerr << "Error determining buffer length for decryption" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    std::string decrypted(outlen, '\0');
    if (EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char*>(&decrypted[0]), &outlen, reinterpret_cast<const unsigned char*>(encrypted_data), encrypted_data_len) <= 0) {
        std::cerr << "Error during decryption" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return "";
    }

    EVP_PKEY_CTX_free(ctx);
    return decrypted;
}
#include "../include/base64.h"

std::string base64_encode(const std::string& input) {
    // Create a BIO chain with a memory sink and base64 filter
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);  // No newlines
    BIO_push(b64, bio);

    // Write the input data to the base64 encoder
    BIO_write(b64, input.data(), input.size());
    BIO_flush(b64);

    // Fetch the encoded data
    char* encoded_data;
    long encoded_length = BIO_get_mem_data(bio, &encoded_data);

    // Create a std::string from the encoded data
    std::string encoded_string(encoded_data, encoded_length);

    // Clean up
    BIO_free_all(b64);

    return encoded_string;
}

std::string base64_decode(const std::string& encoded) {
    BIO* bio = BIO_new_mem_buf(encoded.c_str(), -1); // -1: assume string is null terminated
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // Do not use newlines to flush buffer
    bio = BIO_push(b64, bio);

    std::vector<unsigned char> buffer(encoded.length());
    int decoded_length = BIO_read(bio, buffer.data(), static_cast<int>(encoded.length()));

    if (decoded_length < 0) {
        BIO_free_all(bio);
        throw std::runtime_error("Error in Base64 decoding");
    }

    BIO_free_all(bio);

    return std::string(reinterpret_cast<char*>(buffer.data()), decoded_length);
}
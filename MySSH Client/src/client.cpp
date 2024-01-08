#include "../include/client.h"
#include "../include/read_write.h"

void command_loop(const int& sd, std::string& username) {

    // ANSI escape code for setting the color
    const std::string RED = "\033[91m";
    const std::string YELLOW = "\033[93m";
    const std::string BLUE = "\033[94m";
    const std::string GREEN = "\033[92m";
    const std::string BLACK = "\033[00m";
    const std::string CYAN = "\033[96m";
    const std::string RESET = "\033[0m";
    std::string path {"/home/alex"};
    std::string server_output_delimiter {"@@@"};
    const char* user_n = username.c_str();

    while(1) {

        char buf[1000];
        memset(buf, 0, sizeof(buf));

        std::cout << YELLOW << user_n << GREEN << " @ " << GREEN << "<" << YELLOW << path
        << GREEN << "> " << GREEN << ":: "  << RED;
        fflush(stdout);

        int bytes_num = read(0, buf, sizeof(buf));
        buf[bytes_num] = '\0';
        // printf("[Client]User input: %s", buf);

        std::cout << RESET << std::flush;
        if (bytes_num > 0 && buf[bytes_num - 1] == '\n') {
            buf[bytes_num - 1] = '\0';
            bytes_num--;
        } else if (bytes_num > 0) {
            buf[bytes_num] = '\0';
        }

        // Getting the AES key of the user
        std::string b64_aes_key = get_aes_key_from_json("../client_data.json", username);
        std::string aes_key = base64_decode(b64_aes_key); // Decode once

        // Encrypting and encoding the user input
        std::string encrypted = aes_encrypt(buf, aes_key);
        std::string base_64_encrypted = base64_encode(encrypted);
        // std::cout << "[Client]Base 64 ciphertext: " << base_64_encrypted << std::endl;

        // Adding the length header to the message
        char* full_msg_with_header = add_len_header(base_64_encrypted.c_str());
        int total_len = base_64_encrypted.length() + sizeof(int);

        // Sending the message to the server
        if (write(sd, full_msg_with_header, total_len) <= 0) {
            throw std::runtime_error("Error writing message to server.");
        }

        // Receive the server output
        int message_len {};
        read_msg_len(sd, message_len);
        std::string client_input = read_msg(sd, message_len);

        // Decoding and decrypting the message
        std::string binary_ciphertext = base64_decode(client_input);
        std::string decoded = aes_decrypt(binary_ciphertext, aes_key);

        // Parsing the message in paths and output
        std::string server_output;
        size_t delimiterPos = decoded.find(server_output_delimiter);
        if (delimiterPos != std::string::npos) {
            path = decoded.substr(0, delimiterPos);
            server_output = decoded.substr(delimiterPos + server_output_delimiter.length());
        } else {
            std::cerr << "Something went wrong with the server output." << std::endl;
        }

        std::cout << server_output;
    }
}
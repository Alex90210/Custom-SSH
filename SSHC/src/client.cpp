#include "../include/client.h"

void command_loop(const int& sd, std::string& username) {

    const char* user_n = username.c_str();
    try {
        while(1) {
            char buf[1000];
            memset(buf, 0, sizeof(buf));

            printf("%s> ", user_n);
            fflush(stdout);

            /*std::string user_input;
            std::cin >> user_input;

            std::cout << "You wrote: " << user_input << std::endl;
            std::cout << "The length of this text is: " << user_input.length() << std::endl;*/

            int bytes_num = read(0, buf, sizeof(buf));
            buf[bytes_num] = '\0';
            printf("[client]Am citit: %s", buf);
            printf("[client]Lungimea acestuia este: %zu\n", strlen(buf));

            if (bytes_num > 0 && buf[bytes_num - 1] == '\n') {
                buf[bytes_num - 1] = '\0';
                bytes_num--;
            } else if (bytes_num > 0) {
                buf[bytes_num] = '\0';
            }

            // ----------------------
            // Send message to server

            // std::string user_input {"joe biden!!!"};
            // const char* fullm = user_input.c_str();

            std::string b64_aes_key = get_aes_key_from_json("../client_data.json", username);
            std::string aes_key = base64_decode(b64_aes_key); // Decode once

            // Encrypt the buffer
            std::string encrypted = aes_encrypt(buf, aes_key);

            // Encode encrypted data to Base64 for display or transport
            std::string base64Encrypted = base64_encode(encrypted);
            std::cout << "BASE 64 CIPHERTEXT: " << base64Encrypted << std::endl;
            std::cout << "CIPHERTEXT: " << encrypted << std::endl;

            // Decrypt the original encrypted data
            std::string decrypted = aes_decrypt(encrypted, aes_key); // Use 'encrypted', not 'cipher_text'

            std::cout << "DECRYPTED TEXT: " << decrypted << std::endl;


            char* fullMessageWithHeader = add_len_header(base64Encrypted.c_str());
            int total_len = base64Encrypted.length() + sizeof(int);

            try {
                if (write(sd, fullMessageWithHeader, total_len) <= 0) {
                    throw std::runtime_error("Error writing message to server.");
                }

                printf("[client]Dimensiunea pachetului este: %ld\n", sizeof(int) + strlen(fullMessageWithHeader));
                // free(fullMessage);

                // Receive message from server
                // ADD ALOCATE MEMORY DYNAMICALLY
                int read_result = read(sd, buf, sizeof(buf));
                if (read_result < 0) {
                    throw std::runtime_error("Error reading message from server.");
                } else if (read_result == 0) {
                    printf("[client]Serverul este offline, conexiunea a fost inchisa.\n");
                    shutdown(sd, SHUT_RDWR);
                    break;
                }

                // fflush(stdout); // Flush standard output buffer
                int string_len = strlen(buf);
                buf[string_len] = '\0';
                printf("[client]dim mesaj server: %d\n", string_len);
                printf("[client]Mesajul criptat de la server: %s\n", buf);

                std::string binary_ciphertext = base64_decode(buf);
                std::string decoded = aes_decrypt(binary_ciphertext, aes_key);

                std::cout << "[client]Mesajul decriptat de la server: " << decoded << std::endl;

            } catch (std::exception &e) {
                printf("[client]Eroare la mesaj: %s\n", e.what());
                break;
            }

        }
    } catch (...) {
        printf("[client]Eroare generala in comanda.\n");
    }
}
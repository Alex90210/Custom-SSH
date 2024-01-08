#include "../include/utils.h"
#include "../include/manage_keys.h"
#include "../include/base64.h"
#include "../include/AES.h"
#include "../include/client.h"
#include <wait.h>

extern int errno;
int port;

int main (int argc, char *argv[]) {

    std::string json_path {"../client_data.json"};
    int sd {};
    struct sockaddr_in server {};

    if (argc != 3) {
        printf ("Syntax error: %s <server_addr> <port>\n", argv[0]);
        return -1;
    }

    port = atoi(argv[2]);

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("Error at socket().\n");
        return errno;
    }


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons (port);


    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1) {
        perror ("[Client]Error at connect().\n");
        return errno;
    }

    if (receive_and_store_s_public_key(sd, json_path)) {
        // std::cout << "Public key received and stored successfully." << std::endl;
    } else {
        std::cerr << "Failed to receive or store the public key." << std::endl;
    }

    char start_buf[1000];
    printf("[Client] For login press 'l'. For quit, 'q': ");
    fflush(stdout);
    int bytes_num = read(0, start_buf, sizeof(start_buf) - 1);
    start_buf[bytes_num] = '\0';

    if(strcmp(start_buf, "l") != 0 && strcmp(start_buf, "q") != 0) {
        while (strcmp(start_buf, "l\n") != 0 && strcmp(start_buf, "q\n") != 0) {
            printf("Enter a valid option: ");
            fflush(stdout);
            int bytes_num_oo = read(0, start_buf, sizeof(start_buf) - 1);
            start_buf[bytes_num_oo] = '\0';
            if (bytes_num_oo > 0 && start_buf[bytes_num_oo - 1] != '\n') {
                int c;
                while ((c = getchar()) != '\n' && c != EOF) { }
            }
        }
    }

    std::string username, password;
    if(strcmp(start_buf, "l\n") == 0) {

        json users = read_json(json_path);
        const int max_attempts = 3;
        int attempt_count = 0;

        while (attempt_count < max_attempts) {
            std::cout << "Enter your username: ";
            std::cin >> username;
            std::cout << "Enter your password: ";
            std::cin >> password;

            if (authenticate_user(users, username, password)) {
                std::cout << "Authentication successful. You are connected to the server."
                << std::endl;
                /*update_user_status(users, username, "active");
                save_json(users, "../client_data.json");*/
                break;
            } else {
                std::cout << "Authentication failed." << std::endl;
                attempt_count++;
                if (attempt_count < max_attempts) {
                    std::cout << "Please try again." << std::endl;
                }
            }
        }

        if (attempt_count == max_attempts) {
            std::cout << "Maximum login attempts reached. Exiting program." << std::endl;
            return 1;
        }

    } else if (strcmp(start_buf, "q\n") == 0) {
        printf("Connection terminated.\n");
        shutdown(sd, SHUT_RDWR);
        close(sd);
        return 0;
    }

    // Generate and store the aes key
    unsigned char aes_Key[KEY_LENGTH_BYTES];
    if (!generate_aes_key(aes_Key)) {
        std::cerr << ("AES Key generation error.");
        return 1;
    }

    std::string aes_key(reinterpret_cast<const char*>(aes_Key), KEY_LENGTH_BYTES);
    std::string base_64_encoded_key = base64_encode(aes_key);
    store_aes_key_b64(base_64_encoded_key, username);

    // print_AES_key(json_path);

    std::string aes_key_from_json = get_aes_key_from_json(json_path, username);
    EVP_PKEY* publicKey = load_public_key_JSON(json_path);

    std::string encrypted_username = encrypt_with_public_key(publicKey, username);
    std::string encrypted_aes_key = encrypt_with_public_key(publicKey, aes_key_from_json);

    int usernameLen = username.length();
    int aesKeyLen = aes_key_from_json.length();

    int encrypted_username_len = encrypted_username.size();
    int encrypted_aes_key_len = encrypted_aes_key.size();

    int total_len = sizeof(int) * 2 + encrypted_username_len + encrypted_aes_key_len;
    char* fullMessage = new char[total_len];

    // Creating the message
    memcpy(fullMessage, &usernameLen, sizeof(int));
    memcpy(fullMessage + sizeof(int), &aesKeyLen, sizeof(int));
    memcpy(fullMessage + sizeof(int) * 2, encrypted_username.data(), encrypted_username_len);
    memcpy(fullMessage + sizeof(int) * 2 + encrypted_username_len, encrypted_aes_key.data(), encrypted_aes_key_len);

    // printing the message to verify it
    // print_message(fullMessage, total_len);

    // Send the message
    if (write(sd, fullMessage, total_len) <= 0) {
        std::cerr << "Error writing message to server." << std::endl;
    }

    delete[] fullMessage;
    command_loop(sd, username);
}
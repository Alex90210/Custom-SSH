#include "../include/utils.h"
#include "../include/manage_keys.h"
#include "../include/utils.h"
#include "../include/base64.h"
#include "../include/AES.h"
#include "../include/client.h"

extern int errno;
int port;
#define KEY_LEN_BYTES 16
#define RSA_KEY_SIZE (1024 / 8)

int main (int argc, char *argv[]) {

    std::string json_path {"../client_data.json"};
    int sd {};
    struct sockaddr_in server;

    if (argc != 3) {
        printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    port = atoi (argv[2]);

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("Eroare la socket().\n");
        return errno;
    }


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons (port);


    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1) {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }

    // receiving and storing the rsa server public key
    // create a global variable for the file location
    // maybe do the function void, the client doesn't need to know about keys

    if (receive_and_store_s_public_key(sd, "../client_data.json")) {
        std::cout << "Public key received and stored successfully." << std::endl;
    } else {
        std::cerr << "Failed to receive or store the public key." << std::endl;
    }

    // -----------------------------------------------


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
            start_buf[bytes_num_oo] = '\0'; // Null-terminate the string

            if (bytes_num_oo > 0 && start_buf[bytes_num_oo - 1] != '\n') {
                int c;
                while ((c = getchar()) != '\n' && c != EOF) { }
            }
        }
    }

    std::string username, password;
    if(strcmp(start_buf, "l\n") == 0) {

        json users = read_json("../client_data.json");

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

    // generate and store the aes key
    unsigned char aesKeyC[KEY_LENGTH_BYTES];
    if (!generate_aes_key(aesKeyC)) {
        std::cerr << ("AES Key generation error.");
        return 1;
    }

    std::string aesKey(reinterpret_cast<const char*>(aesKeyC), KEY_LENGTH_BYTES);

    std::string base_64_encoded_key = base64_encode(aesKey);

    store_aes_key_b64(base_64_encoded_key, username);

    std::string jsonFileName = "../client_data.json";
    printAESKeys(jsonFileName);


    // sending the aes key to the server as the first message

    std::string aesKeyString = get_aes_key_from_json(json_path, username);
    EVP_PKEY* publicKey = load_public_key_JSON(json_path);

    std::string encryptedUsername = encrypt_with_public_key(publicKey, username);
    std::string encryptedAesKey = encrypt_with_public_key(publicKey, aesKeyString);

    int usernameLen = username.length(); // Length of unencrypted username
    int aesKeyLen = aesKeyString.length(); // Length of unencrypted AES key

    int encryptedUsernameLen = encryptedUsername.size(); // Length of encrypted username
    int encryptedAesKeyLen = encryptedAesKey.size();     // Length of encrypted AES key

    // Note: total_len includes the size of two headers and the length of the encrypted data
    int total_len = sizeof(int) * 2 + encryptedUsernameLen + encryptedAesKeyLen;
    char* fullMessage = new char[total_len];
    if (!fullMessage) {
        std::cerr << "Failed to allocate memory for the message." << std::endl;
        return 1;
    }

    // Use unencrypted lengths for the headers
    memcpy(fullMessage, &usernameLen, sizeof(int));
    memcpy(fullMessage + sizeof(int), &aesKeyLen, sizeof(int));

    // Copy the encrypted data
    memcpy(fullMessage + sizeof(int) * 2, encryptedUsername.data(), encryptedUsernameLen);
    memcpy(fullMessage + sizeof(int) * 2 + encryptedUsernameLen, encryptedAesKey.data(), encryptedAesKeyLen);


    // printing the message to verify it

    printMessage(fullMessage, total_len);

    //-----------------

    // Send the message
    if (write(sd, fullMessage, total_len) <= 0) {
        std::cerr << "Error writing message to server." << std::endl;
    }

    // Free the allocated memory
    // free(fullMessage);
    delete[] fullMessage;

    // -----------------------------
    command_loop(sd, username);
}
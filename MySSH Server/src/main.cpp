#include "../include/client_handler.h"
#include "../include/RSA.h"
#include <string>

#define PORT 2908
extern int errno;

int main ()
{
    std::string json_file {"../server_data.json"};
    struct sockaddr_in server;
    struct sockaddr_in from;
    int sd {};
    pthread_t th[100];
    int i {1};

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("[Server]Error at socket().\n");
        return errno;
    }

    int on {1};
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);


    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
        perror ("[Server]Error at bind().\n");
        return errno;
    }

    if (listen (sd, 2) == -1) {
        perror ("[Server]Error at listen().\n");
        return errno;
    }

    // Generating and storing the RSA keys
    EVP_PKEY *pkey = generate_rsa_key();
    if (pkey) {
        std::string private_key = key_to_pem(pkey, true);
        std::string public_key = key_to_pem(pkey, false);

        write_keys_to_json(json_file, private_key, public_key);

        EVP_PKEY_free(pkey);
    }

    while (1) {

        int client;
        thData * td;
        int length {sizeof(from)};

        printf ("[Server]Waiting at the %d port.\n",PORT);
        fflush (stdout);

        if ((client = accept(sd, (struct sockaddr *) &from,
                             reinterpret_cast<socklen_t *>(&length))) < 0) {

            perror ("[Server]Error at accept().\n");
            continue;
        }

        td = (struct thData*)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;

        pthread_create(&th[i], NULL, &treat, td);
    }
}
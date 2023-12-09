#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
    int sd;			// descriptorul de socket
    struct sockaddr_in server;	// structura folosita pentru conectare
    // mesajul trimis

    /* exista toate argumentele in linia de comanda? */
    if (argc != 3)
    {
        printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    port = atoi (argv[2]);

    /* cream socketul */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons (port);

    /* ne conectam la server */
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }

    while (1) {
        const unsigned max_message_len = 1000;
        char buf[max_message_len];
            
        printf("[client]Introduceti un mesaj: ");
        fflush(stdout);
        int bytes_num = read(0, buf, sizeof(buf));

        // buf[bytes_num] = '\0';
        printf("[client]Am citit: %s", buf);
        printf("[client]Dimensiunea acestuia este: %zu\n", strlen(buf));
        int string_len = strlen(buf);
        printf("strlen: %d", string_len);
        // pana aici avem lungimea buna

        char *fullMessage = (char *)malloc(sizeof(int) + string_len);
        if (fullMessage == NULL) {
            perror("Error allocating memory for full message");
        }

        memcpy(fullMessage, &string_len, sizeof(int));
        memcpy(fullMessage + sizeof(int), buf, string_len);

        /* trimiterea mesajului la server */
        if (write(sd, fullMessage, sizeof(int) + string_len) <= 0)
        {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }

        printf("[client]Dimensiunea pachetului este: %ld\n", sizeof(int) + string_len);
        free(fullMessage);

        if (read (sd, &buf,sizeof(buf)) < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }

        printf ("[client]Mesajul primit este: %s\n", buf);
    }

    shutdown (sd, SHUT_RDWR);
}
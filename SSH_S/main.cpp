#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
    int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    int nr;		//mesajul primit de trimis la client
    int sd;		//descriptorul de socket
    int pid;
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i=0;


    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server]Eroare la socket().\n");
        return errno;
    }
    /* utilizarea optiunii SO_REUSEADDR */
    int on=1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server)); // seteaza toti bitii la 0
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 2) == -1)
    {
        perror ("[server]Eroare la listen().\n");
        return errno;
    }
    /* servim in mod concurent clientii...folosind thread-uri */
    while (1)
    {
        int client;
        thData * td; //parametru functia executata de thread
        int length = sizeof (from);

        printf ("[server]Asteptam la portul %d...\n",PORT);
        fflush (stdout);

        // client= malloc(sizeof(int));
        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ((client = accept(sd, (struct sockaddr *) &from, reinterpret_cast<socklen_t *>(&length))) < 0)
        {
            perror ("[server]Eroare la accept().\n");
            continue;
        }

        /* s-a realizat conexiunea, se astepta mesajul */

        // int idThread; //id-ul threadului
        // int cl; //descriptorul intors de accept

        td=(struct thData*)malloc(sizeof(struct thData));
        td->idThread=i++;
        td->cl=client;

        pthread_create(&th[i], NULL, &treat, td);

    }//while
};
static void *treat(void * arg)
{
    struct thData tdL;
    tdL= *((struct thData*)arg);

    printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush (stdout);
    pthread_detach(pthread_self());
    raspunde((struct thData*)arg);
    /* am terminat cu acest client, inchidem conexiunea */
    close ((intptr_t)arg);
    return(NULL);
};


void raspunde(void *arg)
{
    int nr, i=0;
    struct thData tdL;
    tdL= *((struct thData*)arg);



    int message_len;
    char message_r[1000];

    int hmmm = 0;
    while (1) {
        // Read the message length header
        if (read(tdL.cl, &message_len, sizeof(int)) <= 0) {
            perror("Error reading message length header");
            // Handle the error here (set a flag, print a message, etc.)
        }

        // Allocate memory for the full message
        char *fullMessage = (char *)malloc(message_len + 1); // +1 for null terminator
        if (fullMessage == NULL) {
            perror("Error allocating memory for full message");
            // Handle the error here (set a flag, print a message, etc.)
        }

        // Read the actual message
        ssize_t bytesRead = 0;
        while (bytesRead < message_len) {
            ssize_t result = read(tdL.cl, fullMessage + bytesRead, message_len - bytesRead);
            if (result <= 0) {
                perror("Error reading message");
                free(fullMessage);
            }
            bytesRead += result;
        }

        char *newline_pos = strchr(fullMessage, '\n');

        // Check if newline character was found
        if (newline_pos != NULL) {
            // Replace newline character with null terminator
            *newline_pos = '\0';
        }

        printf("sizeofmessage: %d\n", message_len);
        printf("[Thread %d] Received message: %s\n", tdL.idThread, fullMessage);
        // printf("jeje %s sldjkfs", fullMessage);

        printf("[Thread %d]Trimitem mesajul inapoi...%d\n",tdL.idThread, nr);

        if(strcmp(fullMessage, "login") == 0) {
            char message_s[] = "Comanda 'Login' a fost recunoscuta.";
            if (write (tdL.cl, &message_s, sizeof(message_s)) <= 0)
            {
                printf("[Thread %d] ",tdL.idThread);
                perror ("[Thread]Eroare la write() catre client.\n");
            }
            else
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
        }
        else if (strcmp(fullMessage, "create account") == 0) {
            char message_s[] = "Comanda 'Create Account' a fost recunoscuta.";
            if (write (tdL.cl, &message_s, sizeof(message_s)) <= 0)
            {
                printf("[Thread %d] ",tdL.idThread);
                perror ("[Thread]Eroare la write() catre client.\n");
            }
            else
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
        }
        else if (strcmp(fullMessage, "show public key") == 0) {
            char message_s[] = "Comanda 'Show Public Key' a fost recunoscuta.";
            if (write (tdL.cl, &message_s, sizeof(message_s)) <= 0)
            {
                printf("[Thread %d] ",tdL.idThread);
                perror ("[Thread]Eroare la write() catre client.\n");
            }
            else
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
        }
        else if (strcmp(fullMessage, "show private key") == 0) {
            char message_s[] = "Comanda 'show private key' a fost recunoscuta.";
            if (write (tdL.cl, &message_s, sizeof(message_s)) <= 0)
            {
                printf("[Thread %d] ",tdL.idThread);
                perror ("[Thread]Eroare la write() catre client.\n");
            }
            else
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
        }
        else {
            char message_s[] = "Comanda introdusa nu a fost recunoscuta.";
            if (write (tdL.cl, &message_s, sizeof(message_s)) <= 0)
            {
                printf("[Thread %d] ",tdL.idThread);
                perror ("[Thread]Eroare la write() catre client.\n");
            }
            else
                printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
        }

    }
}
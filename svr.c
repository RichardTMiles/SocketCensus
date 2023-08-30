/* This program sets up a socket to allow two clients to talk to each other */
#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <netdb.h>

pthread_mutex_t mulo = PTHREAD_MUTEX_INITIALIZER;

int total = 1;

/* Port Number */
int portNumber;

int ServerSocket;

/* Listen socket */
struct sockaddr_in ServerStructure, ClientStructure;

pthread_cond_t c = PTHREAD_COND_INITIALIZER;

void *socket_connection(void *passedValue);

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    pthread_mutex_init(&mulo, NULL);

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    /* Create the socket. */
    if ((ServerSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    /* Create the address of the server.*/
    portNumber = atoi(argv[1]);
    bzero((char *) &ServerStructure, sizeof(ServerStructure));
    ServerStructure.sin_family = AF_INET;
    ServerStructure.sin_addr.s_addr = INADDR_ANY;
    ServerStructure.sin_port = htons(portNumber);


    /*Bind the socket to the address.*/
    if (bind(ServerSocket, (struct sockaddr *) &ServerStructure, sizeof(ServerStructure)) < 0) {
        perror("bind");
        exit(1);
    }

    /* Listen for connections. */
    if (listen(ServerSocket, 5) < 0) {
        perror("listen");
        exit(1);
    }

    int i = 0;

    pthread_t array[6];             // maximum number of connections
    /*Accept a connection.*/
    for (;;) {
        pthread_create(&array[i], NULL, socket_connection, (void *) &total);
        pthread_cond_wait(&c, &mulo);
    }
}

void *socket_connection(void *passedValue) {

    // this is valid to set multiple variables
    struct sockaddr_in ClientStructure;

    int passed = 1,
            clientNumber = 1,
            ClientSocket,
            ClientLength = sizeof(ClientStructure),
            *passed_in_value = ((int *) passedValue);

    char buffer[1024];

    bzero((char *) &buffer, sizeof(buffer));

    ssize_t bytesRead;

    if ((ClientSocket = accept(ServerSocket, (struct sockaddr *) &ClientStructure, &ClientLength)) < 0) {
        perror("accept");
        exit(1);
    }

    printf("Thread %d\n", *passed_in_value);

    // Add client number to buffer
    sprintf(buffer, "%d", *passed_in_value);

    // Send client number to cli
    send(ClientSocket, buffer, sizeof(buffer) + 1, 0);

    printf("Client %i Connected Successfully \n", clientNumber);

    bzero((char *) &buffer, sizeof(buffer));


    bytesRead = recv(ClientSocket, (void *) buffer, sizeof(buffer), 0); // flags = 0

    printf("Buffer is %zd bytes.\n", bytesRead);
    printf("Buffer :: %s \n", buffer);

    /* If error or eof, terminate. */
    pthread_mutex_lock(&mulo);      // Lock function threads from editing this pointer at the same time
    passed = atoi(buffer);
    pthread_mutex_unlock(&mulo);    // remove function lock

    if (bytesRead < 1) {
        printf("bytes read fail: %d\n ---> %s\n ---> %d\n ---> %d\n ---> %i\n",
               bytesRead,
               buffer,
               passed,
               *passed_in_value,
               errno);

        close(ClientSocket);
        exit(0);
    }

    pthread_cond_signal(&c);

    printf("Server Sent :: %s \n", buffer);


}

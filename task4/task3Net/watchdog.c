#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <time.h>
#include <signal.h>

#define SERVER_PORT 3000 // The port that the server listens
int main()
{
    int listeningSocket = -1;
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // 0 means default protocol for stream sockets (Equivalently, IPPROTO_TCP)
    if (listeningSocket == -1)
    {
        printf("Could not create listening socket : %d", errno);
        return 1;
    }
    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;  // any IP at this port (Address to accept any incoming messages)
    serverAddress.sin_port = htons(SERVER_PORT); // network order (makes byte order consistent)

    // set socket option to reuse the address
    int reuse = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0)
    {
        printf("setsockopt(SO_REUSEADDR) failed\n");
        return -1;
    }
    const int enable = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed\n");

    // Bind the socket to the port with any IP at this port
    int bindResult = bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult == -1)
    {
        printf("Bind failed with error code : %d\n", errno);
        // close the socket
        close(listeningSocket);
        return -1;
    }
    printf("WatchDog: Bind() success\n");
    // Make the socket listening; actually mother of all client sockets.
    // 500 is a Maximum size of queue connection requests
    // number of concurrent connections
    int listenResult = listen(listeningSocket, 2);
    if (listenResult == -1)
    {
        printf("listen() failed with error code : %d", errno);
        // close the socket
        close(listeningSocket);
        return -1;
    }
    // Accept incoming connections
    struct sockaddr_in clientAddress; //
    socklen_t clientAddressLen = sizeof(clientAddress);
    printf("WatchDog: Waiting for incoming TCP-connections...\n");
    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddressLen = sizeof(clientAddress);
    int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
    if (clientSocket == -1)
    {
        printf("listen failed with error code : %d\n", errno);
        // close the sockets
        close(listeningSocket);
        return -1;
    }
    printf("WatchDog: A new client connection accepted\n");
    int readResult = -2;
    char buffer[1024];
    memset(buffer, 0, 1024);

    while (readResult = recv(clientSocket, buffer, 1024, 0))
    {
        printf("WatchDog: creating timer.\n");
        break;
    }
    // Set the socket to be non-blocking (timeout)
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    while (1)
    {
        // Receive data from the client using
        char buffer[1024];
        memset(buffer, 0, 1024);
        int readSesault = -2;
        readResult = recv(clientSocket, buffer, 1024, 0);
        // if timeout, send timeout to client and break
        if (readResult == -1)
        {
            // kill parent process
            close(clientSocket);
            close(listeningSocket);
            kill(getppid(), SIGUSR1);
            return -1;
        }
    }
    // close the sockets
    close(clientSocket);
    close(listeningSocket);
    return 0;
}

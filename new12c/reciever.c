/*
    TCP/IP-server
*/

#include <stdio.h>

// Linux and other UNIXes
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/tcp.h>

#define SERVER_PORT 5060 // The port that the server listens
#define FILE_SIZE 1029120
#define BUFFER_SIZE 8192

void addLongToString(char *str, long num)
{
    char temp[20];
    sprintf(temp, "%ld", num);
    strcat(str, temp);
}

int main()
{

    // Time setup:
    struct timeval start, end;
    long tot = 0;

    int listeningSocket = -1;
    listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == -1)
    {
        printf("Could not create listening socket : %d", errno);
        return 1;
    }
    int enableReuse = 1;
    int ret = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int));
    if (ret < 0)
    {
        printf("setsockopt() failed with error code : %d", errno);
        return 1;
    }
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    int bindResult = bind(listeningSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult == -1)
    {
        printf("Bind failed with error code : %d", errno);
        // close the socket
        close(listeningSocket);
        return -1;
    }

    int listenResult = listen(listeningSocket, 3);
    if (listenResult == -1)
    {
        printf("listen() failed with error code : %d", errno);
        // close the socket
        close(listeningSocket);
        return -1;
    }

    // Accept and incoming connection
    printf("Waiting for incoming TCP-connections...\n");
    struct sockaddr_in clientAddress; //
    socklen_t clientAddressLen = sizeof(clientAddress);

    while (1)
    {
        printf("waiting..\n");
        memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress);
        int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (clientSocket == -1)
        {
            printf("listen failed with error code : %d", errno);
            // close the sockets
            close(listeningSocket);
            return -1;
        }

        printf("A new client connection accepted\n");

        // time calculations preperation:
        char time_text[100000] = "";
        long totClientTime_1 = 0;
        long totClientTime_2 = 0;
        int countFileSent = 1;

    restart:
        printf("Changed Congestion Control to Cubic\n");
        char ccBuffer[256];
        strcpy(ccBuffer, "cubic");
        socklen_t socklen = strlen(ccBuffer);

        if (setsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, socklen) != 0)
        {
            perror("ERROR! socket setting failed!");
            return -1;
        }
        if (getsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, &socklen) != 0)
        {
            perror("ERROR! socket getting failed!");
            return -1;
        }
        // Receive first half of the file from client
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived, amountRec = 0, chunkIndex = 1, flag = 0;

        gettimeofday(&start, NULL);

        while (((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0))
        {
            amountRec += bytesReceived;
            if (flag == 2)
            {
                puts("the is the continues of user decision");
                puts(buffer);
                if (strncmp(buffer, "g", 1) == 0)
                {
                    printf("Client has decided to end the session. Stats:\n");
                    puts(time_text);
                    printf("Total of time for first half in file[%d] is: %ld for %d times for an avarage of %ld. \n", countFileSent, totClientTime_1, countFileSent, totClientTime_1 / countFileSent);
                    printf("Total of time for second half in file[%d] is: %ld for %d times for an avarage of %ld. \n", countFileSent, totClientTime_2, countFileSent, totClientTime_2 / countFileSent);
                    // close(clientSocket);
                }
                else
                {
                    // countFileSent++;
                    puts("****************************************************");
                    goto restart;
                }
                break;
            }

            bzero(buffer, BUFFER_SIZE);
            if (flag == 1)
            {
                // time capturing handling
                gettimeofday(&end, NULL);
                tot = ((end.tv_sec * 1000000 + end.tv_usec) -
                       (start.tv_sec * 1000000 + start.tv_usec));
                totClientTime_1 += tot;
                char temp_str[50] = "time taken in micro seconds for first half: ";
                addLongToString(temp_str, tot);
                strcat(temp_str, "\n");
                strcat(time_text, temp_str);

                // Reply to client
                puts("sent special authontication to client!");
                char approval[BUFFER_SIZE] = "special auth is:";
                addLongToString(approval, 12345678);
                int messageLen = strlen(approval) + 1;

                int bytesSent = send(clientSocket, approval, messageLen, 0);
                if (bytesSent == -1)
                {
                    printf("send() failed with error code : %d", errno);
                    close(listeningSocket);
                    close(clientSocket);
                    return -1;
                }
                else if (bytesSent == 0)
                {
                    printf("peer has closed the TCP connection prior to send().\n");
                }
                else if (bytesSent < messageLen)
                {
                    printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
                }

                // Changing to reno algorithm
                printf("Changed Congestion Control to Reno\n");
                strcpy(ccBuffer, "reno");
                socklen = strlen(ccBuffer);
                if (setsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, socklen) != 0)
                {
                    perror("ERROR! socket setting failed!");
                    return -1;
                }
                socklen = sizeof(ccBuffer);
                if (getsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, &socklen) != 0)
                {
                    perror("ERROR! socket getting failed!");
                    return -1;
                }

                gettimeofday(&start, NULL);
                goto skip;
            }

            // Reply to client
            char approval[BUFFER_SIZE] = "chunk approved, chunk number ";
            addLongToString(approval, chunkIndex);
            int messageLen = strlen(approval) + 1;

            int bytesSent = send(clientSocket, approval, messageLen, 0);
            if (bytesSent == -1)
            {
                printf("send() failed with error code : %d", errno);
                close(listeningSocket);
                close(clientSocket);
                return -1;
            }
            else if (bytesSent == 0)
            {
                printf("peer has closed the TCP connection prior to send().\n");
            }
            else if (bytesSent < messageLen)
            {
                printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
            }
            printf("recieved chunk number %d, with %d bytes, and sended approval for chunk number %d\n", chunkIndex, bytesReceived, chunkIndex);
            chunkIndex++;
        skip:
            if (flag)
            {
                flag = 0;
            }
            if ((amountRec > (FILE_SIZE / 2)) && (amountRec < ((FILE_SIZE / 2) + BUFFER_SIZE)))
            {
                flag = 1;
            }
            else if (amountRec >= FILE_SIZE)
            {
                printf("finished recieving file, amount recieved is: %d \n", amountRec);
                puts("this is the begging of the user decision");

                // time capturing handling
                gettimeofday(&end, NULL);
                tot = ((end.tv_sec * 1000000 + end.tv_usec) -
                       (start.tv_sec * 1000000 + start.tv_usec));
                totClientTime_2 += tot;

                char temp_str1[50] = "time taken in micro seconds for second half: ";
                addLongToString(temp_str1, tot);
                strcat(temp_str1, "\n");
                strcat(time_text, temp_str1);

                flag = 2;
            }
        }
    }
    close(listeningSocket);
    return 0;
}

/*
        TCP/IP client
*/

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

#define SERVER_PORT 5060
#define SERVER_IP_ADDRESS "127.0.0.1"
#define FILE_SIZE 1029120
#define BUFFER_SIZE 8192
#define FILE_NAME "1.txt"

void addLongToString(char *str, long num)
{
    char temp[20];
    sprintf(temp, "%ld", num);
    strcat(str, temp);
}

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
    {
        printf("Could not create socket : %d", errno);
        return -1;
    }
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &serverAddress.sin_addr);
    if (rval <= 0)
    {
        printf("inet_pton() failed");
        return -1;
    }
    // Make a connection to the server with socket SendingSocket.
    int connectResult = connect(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectResult == -1)
    {
        printf("connect() failed with error code : %d", errno);
        // cleanup the socket;
        close(sock);
        return -1;
    }

    printf("connected to server\n");
restart:
    printf("Changed Congestion Control to Cubic\n");
    char ccBuffer[256];
    strcpy(ccBuffer, "cubic");
    socklen_t socklen = strlen(ccBuffer);
    if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, socklen) != 0)
    {
        perror("ERROR! socket setting failed!");
        return -1;
    }
    socklen = sizeof(ccBuffer);
    if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, &socklen) != 0)
    {
        perror("ERROR! socket getting failed!");
        return -1;
    }

    // Sending first half of the file data to server
    FILE *file;

    file = fopen(FILE_NAME, "r");
    int amountSent = 0, b, chunkIndex = 1, flag = 0;
    char data[BUFFER_SIZE] = {'\0'};
    while (((b = fread(data, 1, sizeof(data), file)) > 0))
    {
    anotherTry:
        if (flag == 2)
        {
            puts("the is the continues of user decision");
            // Sends some data to server
            char buffer[BUFFER_SIZE] = {'\0'};
            printf("Enter \"g\" to end session, enter \"n\" restart process: \n");
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            int messageLen = strlen(buffer) + 1;

            int bytesSent = send(sock, buffer, messageLen, 0); // 4

            if (bytesSent == -1)
            {
                printf("send() failed with error code : %d", errno);
            }
            else if (bytesSent == 0)
            {
                printf("peer has closed the TCP connection prior to send().\n");
            }
            else if (bytesSent < messageLen)
            {
                printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
            }
            else
            {
                printf("message was successfully sent.\n");
            }
            if (strncmp(buffer, "g", 1) != 0)
            {
                bzero(buffer, BUFFER_SIZE);
                puts("****************************************************");
                goto restart;
            }
            else
            {
                break;
            }
        }
        if (send(sock, data, sizeof(data), 0) == -1)
        {
            perror("ERROR! Sending has failed!\n");
            exit(1);
        }
        amountSent += b;
        bzero(data, BUFFER_SIZE);
        // Receive data from server
        char approval[BUFFER_SIZE] = "chunk approved, chunk number ";
        addLongToString(approval, chunkIndex);
        char bufferReply[BUFFER_SIZE] = {'\0'};
        int bytesReceived = recv(sock, bufferReply, BUFFER_SIZE, 0);
        if (bytesReceived == -1)
        {
            printf("recv() failed with error code : %d", errno);
        }
        else if (bytesReceived == 0)
        {
            printf("peer has closed the TCP connection prior to recv().\n");
        }
        else
        {
            if (flag == 1 && strcmp(bufferReply, "special auth is:12345678") == 0)
            {
                puts("recieved special authontication from server!");

                // Changing to reno algorithm
                printf("Changed Congestion Control to Reno\n");
                strcpy(ccBuffer, "reno");
                socklen = strlen(ccBuffer);
                if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, socklen) != 0)
                {
                    perror("ERROR! socket setting failed!");
                    return -1;
                }
                socklen = sizeof(ccBuffer);
                if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, ccBuffer, &socklen) != 0)
                {
                    perror("ERROR! socket getting failed!");
                    return -1;
                }
            }
            else if (strcmp(bufferReply, approval) != 0)
            {
                goto anotherTry;
            }
        }
        printf("sended chunk number %d, with %d bytes, and recieved approval for chunk number %d\n", chunkIndex, b, chunkIndex);
        chunkIndex++;
        if ((amountSent > (FILE_SIZE / 2)) && (amountSent < ((FILE_SIZE / 2) + BUFFER_SIZE)))
        {
            flag = 1;
        }
        else if (flag)
        {
            flag = 0;
        }

        if (amountSent >= FILE_SIZE)
        {
            printf("finished sending file amount sent is: %d \n", amountSent);
            puts("this is the begging of the user decision");
            flag = 2;
            goto anotherTry;
            // break;
        }
    }

    close(sock);
    return 0;
}

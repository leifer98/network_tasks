/*
	TCP/IP client
*/ 


#include <stdio.h>

#if defined _WIN32

// link with Ws2_32.lib
#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

#else
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#define SERVER_PORT 5060
#define SERVER_IP_ADDRESS "127.0.0.1"

int main()
{
#if defined _WIN32
    // Windows requires initialization
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
#endif

     int sock = socket(AF_INET, SOCK_STREAM, 0);

     if(sock == -1)
    {
        printf("Could not create socket : %d" 
#if defined _WIN32
	,WSAGetLastError()
#else
	,errno
#endif
		);
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
	int rval = inet_pton(AF_INET, (const char*)SERVER_IP_ADDRESS, &serverAddress.sin_addr);
	if (rval <= 0)
	{
		printf("inet_pton() failed");
		return -1;
	}

     // Make a connection to the server with socket SendingSocket.
    int check = connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
     if (check == -1)
     {
	   printf("connect() failed with error code : %d\n" 
#if defined _WIN32
	   ,WSAGetLastError()
#else
	   ,errno
#endif
	   );
     }

     printf("connected to server\n");

     // Sends some data to server
     char message[] = "Good morning, Vietnam\n";
     int messageLen = strlen(message) + 1;

     int bytesSent = send(sock, message, messageLen, 0);

     if (-1 == bytesSent)
     {
	printf("send() failed with error code : %d" 
#if defined _WIN32
	,WSAGetLastError()
#else
	,errno
#endif
		);
     }
     else if (0 == bytesSent)
     {
	printf("peer has closed the TCP connection prior to send().\n");
     }
     else if (messageLen > bytesSent)
     {
	printf("sent only %d bytes from the required %d.\n", messageLen, bytesSent);
     }
     else 
     {
	printf("message was successfully sent .\n");
     }

#if defined _WIN32
	 Sleep(3000);
#else
	 sleep(3);
#endif
	 
//process second side data
char server_reply[8096];
int request = recv(sock, server_reply ,8096,0);
if (request < 0) {
    printf("didn't get data\n");
} else {
    puts(server_reply);
}   
// TODO: All open clientSocket descriptors should be kept
    // in some container and closed as well.
#if defined _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

     return 0;
}

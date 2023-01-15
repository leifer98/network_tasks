/*
    TCP/IP-server
*/

#include<stdio.h>

#if defined _WIN32
#include<winsock2.h>   //winsock2 should be before windows
#pragma comment(lib,"ws2_32.lib")
#else
// Linux and other UNIXes
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#endif

#define SERVER_PORT 5060  //The port that the server listens

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
#else
    signal(SIGPIPE, SIG_IGN); // on linux to prevent crash on closing socket
#endif
      
    // Open the listening (server) socket
    int listeningSocket = -1;  
	 
    if((listeningSocket = socket(AF_INET , SOCK_STREAM , 0 )) == -1)
    {
        printf("Could not create listening socket : %d" 
#if defined _WIN32
		,WSAGetLastError()
#else
		,errno
#endif
		);
    }

	// Reuse the address if the server socket on was closed
	// and remains for 45 seconds in TIME-WAIT state till the final removal.
	//
    int enableReuse = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, 
#if defined _WIN32
		(const char*)
#endif
		&enableReuse, 
		
		sizeof(int)) < 0)
    {
         printf("setsockopt() failed with error code : %d" , 
#if defined _WIN32
		WSAGetLastError()
#else
		errno
#endif
		);
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);  //network order
      
    // Bind the socket to the port with any IP at this port
    if (bind(listeningSocket, (struct sockaddr *)&serverAddress , sizeof(serverAddress)) == -1)
    {
        printf("Bind failed with error code : %d" , 
#if defined _WIN32
	WSAGetLastError()
#else
	errno
#endif
	);
	// TODO: close the socket
        return -1;
    }
      
    printf("Bind() success\n");
  
    // Make the socket listening; actually mother of all client sockets.
    if (listen(listeningSocket, 500) == -1) //500 is a Maximum size of queue connection requests
											//number of concurrent connections 
    {
	printf("listen() failed with error code : %d"
#if defined _WIN32
	,WSAGetLastError()
#else
	,errno
#endif
	);
	// TODO: close the socket
        return -1;
    }
      
    //Accept and incoming connection
    printf("Waiting for incoming TCP-connections...\n");
      
    struct sockaddr_in clientAddress;  //
    socklen_t clientAddressLen = sizeof(clientAddress);

    while (1)
    {
    	memset(&clientAddress, 0, sizeof(clientAddress));
        clientAddressLen = sizeof(clientAddress);
        int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
    	if (clientSocket == -1)
    	{
           printf("listen failed with error code : %d"
#if defined _WIN32
		,WSAGetLastError()
#else
		,errno
#endif
			);
	   // TODO: close the sockets
           return -1;
    	}
      
    	printf("A new client connection accepted\n");   



    	//Reply to client
    	char message[] = "Welcome to our TCP-server\n";
        int messageLen = strlen(message) + 1;

    	int bytesSent = send(clientSocket, message, messageLen, 0);
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

        //process second side data
        char server_reply[8096];
        //to ask him about that
        char buffer_size[1024];
        int request = recv(listeningSocket, buffer ,8096,0);
        if (request < 0) {
            printf("didn't get data\n");
        } else {
            puts(server_reply);
        }

    }

       
  
    // TODO: All open clientSocket descriptors should be kept
    // in some container and closed as well.
#if defined _WIN32
    closesocket(listeningSocket);
    WSACleanup();
#else
    close(listeningSocket);
#endif
      
    return 0;
}

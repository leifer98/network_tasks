#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

// IPv4 header len without options
#define IP4_HDRLEN 20

// ICMP header len for echo req
#define ICMP_HDRLEN 8

#define SERVER_PORT 3000 // The port that the server listens
#define SERVER_IP_ADDRESS "127.0.0.1"
// Checksum algo
unsigned short calculate_checksum(unsigned short *paddress, int len);
// signal handler to close the socket and exit
void sig_handler(int signo);
int snd_socket; // global variable fd to close the socket
int loc_socket; // global variable fd to close the socket
char *dst_ip;
int main(int count, char *argv[])
{
    signal(SIGUSR1, sig_handler);
    if (count != 2)
    {
        printf("usage: %s <ip> \n", argv[0]);
        exit(0);
    }
    dst_ip = argv[1];
    count = 0;
    struct icmp icmphdr; // ICMP-header
    char data[IP_MAXPACKET] = "This is the ping.\n";

    int datalen = strlen(data) + 1;

    // Create raw socket for IP-RAW (make IP-header by yourself)
    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        fprintf(stderr, "socket() failed with error: %d", errno);
        fprintf(stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }
    loc_socket = sock; // global variable to close the socket later

    char *args[2];
    // compiled watchdog.c by makefile
    args[0] = "./watchdog";
    args[1] = NULL;
    int status;
    int pid = fork();
    if (pid == 0)
    {
        // printf("in child \n");
        execvp(args[0], args);
    }
    // Creating new socket
    int SendingSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    snd_socket = SendingSocket; // global variable to close the socket later
    if (SendingSocket == -1)
    {
        printf("Could not create socket : %d", errno);
        return -1;
    }
    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    // convert IPv4 and IPv6 addresses from text to binary form
    int rval = inet_pton(AF_INET, (const char *)SERVER_IP_ADDRESS, &serverAddress.sin_addr);
    if (rval <= 0)
    {
        printf("inet_pton() failed");
        return -1;
    }
    int reuse = 1;
    if (setsockopt(SendingSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0)
    {
        printf("setsockopt(SO_REUSEADDR) failed\n");
        return -1;
    }
    const int enable = 1;
    if (setsockopt(SendingSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed\n");
    // Make a connection to the server with socket SendingSocket.
    sleep(3); // wait for watchdog to start
    int connectResult = connect(SendingSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (connectResult == -1)
    {
        printf("NewPing: connect() failed with error code : %d\n", errno);
        // cleanup the socket;
        close(SendingSocket);
        return -1;
    }
    printf("NEWPING: connected to watchdog..\n");
    send(SendingSocket, "hello", 5, 0);
    int flag = 1;  // flag for the first print of info
    int index = 0; // index for icmp seq
    //===================
    // ICMP header
    //===================
loop:
    // Message Type (8 bits): ICMP_ECHO_REQUEST
    icmphdr.icmp_type = ICMP_ECHO;

    // Message Code (8 bits): echo request
    icmphdr.icmp_code = 0;

    // Identifier (16 bits): some number to trace the response.
    // It will be copied to the response packet and used to map response to the request sent earlier.
    // Thus, it serves as a Transaction-ID when we need to make "ping"
    icmphdr.icmp_id = 18;

    // Sequence Number (16 bits): starts at 0
    icmphdr.icmp_seq = count++;

    // ICMP header checksum (16 bits): set to 0 not to include into checksum calculation
    icmphdr.icmp_cksum = 0;

    // Combine the packet
    char packet[IP_MAXPACKET];

    // Next, ICMP header
    memcpy((packet), &icmphdr, ICMP_HDRLEN);

    // After ICMP header, add the ICMP data.
    memcpy(packet + ICMP_HDRLEN, data, datalen);

    // Calculate the ICMP header checksum
    icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), ICMP_HDRLEN + datalen);
    memcpy((packet), &icmphdr, ICMP_HDRLEN);

    struct sockaddr_in dest_in;
    memset(&dest_in, 0, sizeof(struct sockaddr_in));
    dest_in.sin_family = AF_INET;

    // The port is irrelant for Networking and therefore was zeroed.
    // dest_in.sin_addr.s_addr = iphdr.ip_dst.s_addr;
    dest_in.sin_addr.s_addr = inet_addr(dst_ip);
    // inet_pton(AF_INET, dst_ip, &(dest_in.sin_addr.s_addr));

    struct timeval start, end;
    gettimeofday(&start, 0);

    // Send the packet using sendto() for sending datagrams.
    int bytes_sent = sendto(sock, packet, ICMP_HDRLEN + datalen, 0, (struct sockaddr *)&dest_in, sizeof(dest_in));
    if (bytes_sent == -1)
    {
        fprintf(stderr, "sendto() failed with error: %d", errno);
        return -1;
    }
    // printf("Successfuly sent one packet : ICMP HEADER : %d bytes, data length : %d , icmp header : %d \n", bytes_sent, datalen, ICMP_HDRLEN);
    if (flag)
    {
        printf("NEWPING: PING %s (%s): %d data bytes\n", dst_ip, dst_ip, bytes_sent);
        flag = 0;
    }

    // toggle on and off to create timeout
    while (count > 10)
        ;
    // Get the ping response
    bzero(packet, IP_MAXPACKET);
    socklen_t len = sizeof(dest_in);
    size_t bytes_received = -1;
    while ((bytes_received = recvfrom(sock, packet, sizeof(packet), 0, (struct sockaddr *)&dest_in, &len)))
    {
        if (bytes_received > 0)
        {
            // Check the IP header
            struct iphdr *iphdr = (struct iphdr *)packet;
            struct icmphdr *icmphdr = (struct icmphdr *)(packet + (iphdr->ihl * 4));
            // printf("%ld bytes from %s\n", bytes_received, inet_ntoa(dest_in.sin_addr));
            // icmphdr->type

            gettimeofday(&end, 0);

            char reply[IP_MAXPACKET];
            memcpy(reply, packet + ICMP_HDRLEN + IP4_HDRLEN, datalen);
            // printf("ICMP reply: %s \n", reply);

            char sourceIPAddrReadable[32] = {'\0'};
            inet_ntop(AF_INET, &iphdr->saddr, sourceIPAddrReadable, sizeof(sourceIPAddrReadable));
            char destinationIPAddrReadable[32] = {'\0'};
            inet_ntop(AF_INET, &iphdr->daddr, destinationIPAddrReadable, sizeof(destinationIPAddrReadable));
            float milliseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
            unsigned long microseconds = (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec);
            printf("NEWPING: %ld bytes from %s to %s: icmp_seq=%d time=%f ms\n", bytes_received, sourceIPAddrReadable, destinationIPAddrReadable,
                   icmphdr->un.echo.sequence, milliseconds);

            break;
        }
    }
    // sleep(1);
    // toggle on and off comment below to create timeout with watchdog
    send(SendingSocket, "reset", 5, 0);
    goto loop;

    // Close the raw socket descriptor.
    close(sock);

    return 0;
}

// Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short *paddress, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = paddress;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *((unsigned char *)&answer) = *((unsigned char *)w);
        sum += answer;
    }

    // add back carry outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);                 // add carry
    answer = ~sum;                      // truncate to 16 bits

    return answer;
}
void sig_handler(int signo)
{
    printf("received signal %d, closing socket\n", signo);
    printf("------------------------------------------\n");
    printf("server %s cannot be reached\n", dst_ip);
    printf("------------------------------------------\n");

    close(snd_socket);
    close(loc_socket);
    exit(1);
}

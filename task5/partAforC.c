#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <linux/filter.h>
#include <linux/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>

int main()
{
    char buffer[IP_MAXPACKET];
    struct sockaddr saddr;
    struct packet_mreq mr;

    // Create the raw socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    // Turn on the promiscuous mode.
    mr.mr_type = PACKET_MR_PROMISC;
    setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr));

    int on = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));

    while (1)
    {
        int data_size = recvfrom(sock, buffer, IP_MAXPACKET, 0, &saddr, (socklen_t *)sizeof(saddr));

        struct iphdr *iph = (struct iphdr *)(buffer);
        struct icmphdr *icmph = (struct icmphdr *)(buffer + sizeof(struct iphdr));

        if (icmph->type == ICMP_ECHOREPLY)
        {
            printf("ICMP Echo Reply\n");
        }
        else if (icmph->type == ICMP_ECHO)
        {
            printf("ICMP Echo Request\n");
        }

        if (data_size)
        {
            unsigned int ip = iph->saddr;
            char addrS[16];
            sprintf(addrS, "%d.%d.%d.%d", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);

            ip = iph->daddr;
            char addrD[16];
            sprintf(addrD, "%d.%d.%d.%d", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);

            printf("sniffed a packet, from %s to %s .\n", addrS, addrD);
        }
    }

    close(sock);
    return 0;
}
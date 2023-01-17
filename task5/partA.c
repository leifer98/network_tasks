#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <stdio.h>
#include <linux/filter.h>
#include <linux/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define PORT 9999
/**********************************************
 * Listing 12.1: A compiled BPF code
 **********************************************/

struct calculatorPacket
{
    uint32_t unixtime;
    uint16_t length;

    uint16_t _ : 3, c_flag : 1, s_flag : 1, t_flag : 1, status : 10;
    // union
    // {
    //     uint16_t flags;
    // };

    uint16_t cache;
    uint16_t __;
};

int main()
{
    int PACKET_LEN = 512;
    char buffer[PACKET_LEN];
    struct sockaddr saddr;
    struct packet_mreq mr;

    // Create the raw socket
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));

    // Turn on the promiscuous mode.
    mr.mr_type = PACKET_MR_PROMISC;
    setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr,
               sizeof(mr));
    FILE *fp;
    // Getting captured packets
    while (1)
    {
        int data_size = recvfrom(sock, buffer, PACKET_LEN, 0,
                                 &saddr, (socklen_t *)sizeof(saddr));

        struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        unsigned short iphdrlen = iph->ihl * 4;
        struct tcphdr *tcph = (struct tcphdr *)(buffer + iphdrlen + sizeof(struct ethhdr));
        struct calculatorPacket *clch = (struct calculatorPacket *)(buffer + iphdrlen + sizeof(struct ethhdr) + sizeof(struct tcphdr) + 12);

        unsigned int source_port = ntohs(tcph->source);
        unsigned int dest_port = ntohs(tcph->dest);

        if (data_size)
        {
            if ((dest_port == PORT || source_port == PORT) && ntohs(iph->tot_len) > 85)
            {
                unsigned int ip = iph->saddr;
                unsigned char octet1 = (ip >> 24) & 0xff;
                unsigned char octet2 = (ip >> 16) & 0xff;
                unsigned char octet3 = (ip >> 8) & 0xff;
                unsigned char octet4 = ip & 0xff;
                char addrS[16];
                sprintf(addrS, "%d.%d.%d.%d", octet4, octet3, octet2, octet1);

                ip = iph->daddr;
                octet1 = (ip >> 24) & 0xff;
                octet2 = (ip >> 16) & 0xff;
                octet3 = (ip >> 8) & 0xff;
                octet4 = ip & 0xff;
                char addrD[16];
                sprintf(addrD, "%d.%d.%d.%d", octet4, octet3, octet2, octet1);




                if (ntohs(iph->tot_len) > 500)
                {
                    fp = fopen("output.txt", "w");
                    fprintf(fp, "REQUEST: \n");
                }
                else
                {
                    fprintf(fp, "\nRESPOND: \n");
                }

                fprintf(fp,"{ source_ip: %s, dest_ip: %s, source_port: %d, dest_port: %d, timestamp: %u, \ntotal_length: %d, cache_flag:%d, steps_flag:%d, type_flag:%d, status_code:%d, cache_control:%d, data:\n",
                       addrD, addrS, dest_port, source_port, ntohl(clch->unixtime), ntohs(iph->tot_len), clch->c_flag, clch->s_flag, clch->t_flag, ntohs(clch->status), ntohs(clch->cache));
                int i = (iphdrlen + sizeof(struct ethhdr) + sizeof(struct tcphdr)) + 12;
                for (; i < PACKET_LEN + 40; i++)
                {
                    fprintf(fp, "%02x ", buffer[i] & 0xff);
                    if (i % 20 == 0)
                        fprintf(fp, "\n");
                }
                fprintf(fp, "}\n");

                if (ntohs(iph->tot_len) < 500)
                {
                    fclose(fp);
                    printf("text file updated! \n");
                }
            }
        }
    }

    close(sock);
    return 0;
}
#include <pcap.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/tcp.h>
#include <time.h>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <string.h>

/* Ethernet header */
struct ethheader
{
    u_char ether_dhost[ETHER_ADDR_LEN]; /* destination host address */
    u_char ether_shost[ETHER_ADDR_LEN]; /* source host address */
    u_short ether_type;                 /* IP? ARP? RARP? etc */
};

/* IP Header */
struct ipheader
{
    unsigned char iph_ihl : 4,       // IP header length
        iph_ver : 4;                 // IP version
    unsigned char iph_tos;           // Type of service
    unsigned short int iph_len;      // IP Packet length (data + header)
    unsigned short int iph_ident;    // Identification
    unsigned short int iph_flag : 3, // Fragmentation flags
        iph_offset : 13;             // Flags offset
    unsigned char iph_ttl;           // Time to Live
    unsigned char iph_protocol;      // Protocol type
    unsigned short int iph_chksum;   // IP datagram checksum
    struct in_addr iph_sourceip;     // Source IP address
    struct in_addr iph_destip;       // Destination IP address
};

unsigned short in_cksum(unsigned short *buf, int length)
{
    unsigned short *w = buf;
    int nleft = length;
    int sum = 0;
    unsigned short temp = 0;

    /*
     * The algorithm uses a 32 bit accumulator (sum), adds
     * sequential 16 bit words to it, and at the end, folds back all
     * the carry bits from the top 16 bits into the lower 16 bits.
     */
    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    /* treat the odd byte at the end, if any */
    if (nleft == 1)
    {
        *(u_char *)(&temp) = *(u_char *)w;
        sum += temp;
    }

    /* add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);                 // add carry
    return (unsigned short)(~sum);
}

/* Ethernet header */
// struct ethheader
// {
//     u_char ether_dhost[ETHER_ADDR_LEN]; /* destination host address */
//     u_char ether_shost[ETHER_ADDR_LEN]; /* source host address */
//     u_short ether_type;                 /* IP? ARP? RARP? etc */
// };

void send_raw_ip_packet(struct ipheader *ip)
{
    struct sockaddr_in dest_info;
    int enable = 1;

    // Step 1: Create a raw network socket.
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    // Step 2: Set socket option.
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL,
               &enable, sizeof(enable));

    // Step 3: Provide needed information about destination.
    dest_info.sin_family = AF_INET;
    dest_info.sin_addr = ip->iph_destip;

    // Step 4: Send the packet out.
    sendto(sock, ip, ntohs(ip->iph_len), 0,
           (struct sockaddr *)&dest_info, sizeof(dest_info));
    close(sock);
}
// /* ICMP Header  */
struct icmpheader
{
    unsigned char icmp_type;        // ICMP message type
    unsigned char icmp_code;        // Error code
    unsigned short int icmp_chksum; // Checksum for ICMP Header and data
    unsigned short int icmp_id2;    // Used for identifying request
    unsigned short int icmp_seq2;   // Sequence number
};

/* UDP Header */
struct udpheader
{
    u_int16_t udp_sport; /* source port */
    u_int16_t udp_dport; /* destination port */
    u_int16_t udp_ulen;  /* udp length */
    u_int16_t udp_sum;   /* udp checksum */
};

int send_fake_icmp(char *dest, char *source)
{

    char buffer[1500];

    memset(buffer, 0, 1500);

    /*********************************************************
       Step 1: Fill in the ICMP header.
     ********************************************************/
    struct icmpheader *icmp = (struct icmpheader *)(buffer + sizeof(struct ipheader));
    icmp->icmp_type = 0; // ICMP Type: 8 is request, 0 is reply.

    // Calculate the checksum for integrity
    icmp->icmp_chksum = 0;
    icmp->icmp_chksum = (unsigned short)in_cksum((unsigned short *)icmp, sizeof(struct icmpheader));

    /*********************************************************
       Step 2: Fill in the IP header.
     ********************************************************/
    struct ipheader *ip = (struct ipheader *)buffer;
    ip->iph_ver = 4;
    ip->iph_ihl = 5;
    ip->iph_ttl = 20;
    ip->iph_sourceip.s_addr = inet_addr(source); // CHANGE HERE
    ip->iph_destip.s_addr = inet_addr(dest);     // CHANGE HERE
    ip->iph_protocol = IPPROTO_ICMP;
    ip->iph_len = htons(sizeof(struct ipheader) +
                        sizeof(struct icmpheader));

    /*********************************************************
       Step 3: Finally, send the spoofed packet
     ********************************************************/
    send_raw_ip_packet(ip);

    return 0;
}

void got_packet(u_char *args, const struct pcap_pkthdr *header,
                const u_char *packet)
{

    struct ethheader *eth = (struct ethheader *)packet;

    // Find where the IP header starts, and typecast it to the IP Header structure
    struct ipheader *ip = (struct ipheader *)(packet + sizeof(struct ethheader));
    unsigned short iphdrlen = (ip->iph_ihl) * 4;

    struct icmphdr *icmph = (struct icmphdr *)(packet + sizeof(struct iphdr));

    if (ntohs(eth->ether_type) == 0x0800)
    { // 0x0800 is IP type

        // Find where the IP header starts, and typecast it to the IP Header structure
        struct ipheader *ip = (struct ipheader *)(packet + sizeof(struct ethheader));
        unsigned short iphdrlen = (ip->iph_ihl) * 4;

        // printf("       From: %s\n", inet_ntoa(ip->iph_sourceip));
        // printf("         To: %s\n", inet_ntoa(ip->iph_destip));
        // printf("       type: %d\n", icmph->type);

        if (icmph->type == 64) {
            send_fake_icmp(inet_ntoa(ip->iph_destip), inet_ntoa(ip->iph_sourceip));
            printf("sniffed a ICMP request -> sending a fake ICMP replay\n");
        } 

        /* determine protocol */
        // switch (ip->iph_protocol)
        // {
        // case IPPROTO_TCP:
        //     printf("   Protocol: TCP\n");
        //     return;
        // case IPPROTO_UDP:
        //     printf("   Protocol: UDP\n");
        //     return;
        // case IPPROTO_ICMP:
        // {
        //     printf("   Protocol: ICMP\n");

        //     return;
        // }
        // default:
        //     printf("   Protocol: others\n");
        //     return;
        // }
    }
}

int main()
{
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;
    char filter_exp[] = "icmp";
    bpf_u_int32 net;

    // Step 1: Open live pcap session on NIC with name eth3
    handle = pcap_open_live("eth0", BUFSIZ, 1, 1000, errbuf);

    // Step 2: Compile filter_exp into BPF psuedo-code
    pcap_compile(handle, &fp, filter_exp, 0, net);
    pcap_setfilter(handle, &fp);

    // Step 3: Capture packets
    printf("starting to sniffing...\n");
    pcap_loop(handle, -1, got_packet, NULL);

    pcap_close(handle); // Close the handle

    return 0;
}
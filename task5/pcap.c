#include <pcap.h>
#include <stdio.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <linux/filter.h>
#include <linux/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define PORT 9999
#define OFFSET 12

struct calculatorPacket
{
  uint32_t unixtime;
  uint16_t length;

  uint16_t _ : 3, c_flag : 1, s_flag : 1, t_flag : 1, status : 10;

  uint16_t cache;
  uint16_t __;
};

void got_packet(u_char *args, const struct pcap_pkthdr *header,
                const u_char *buffer)
{
  printf("Got a packet\n");
  struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ethhdr));
  unsigned short iphdrlen = iph->ihl * 4;
  struct tcphdr *tcph = (struct tcphdr *)(buffer + iphdrlen + sizeof(struct ethhdr));
  struct calculatorPacket *clch = (struct calculatorPacket *)(buffer + iphdrlen + sizeof(struct ethhdr) + sizeof(struct tcphdr) + OFFSET);

  unsigned int source_port = ntohs(tcph->source);
  unsigned int dest_port = ntohs(tcph->dest);
  if ((dest_port == PORT || source_port == PORT) && ntohs(iph->tot_len) > 85)
  {
    int i = (iphdrlen + sizeof(struct ethhdr) + sizeof(struct tcphdr)) + OFFSET;
    for (; i < 550; i++)
    {
      printf("%02x ", buffer[i] & 0xff);
      // fprintf(fp, "%02x ", packet[i] & 0xff);
      if (i % 18 == 0)
        printf("\n");
      // fprintf(fp, "\n");
    }
  }
}

int main()
{
  pcap_t *handle;
  char errbuf[PCAP_ERRBUF_SIZE];
  struct bpf_program fp;
  char filter_exp[] = "tcp";
  bpf_u_int32 net;

  // Step 1: Open live pcap session on NIC with name eth3
  handle = pcap_open_live("lo", BUFSIZ, 1, 1000, errbuf);

  // Step 2: Compile filter_exp into BPF psuedo-code
  pcap_compile(handle, &fp, filter_exp, 0, net);
  pcap_setfilter(handle, &fp);

  // Step 3: Capture packets
  pcap_loop(handle, -1, got_packet, NULL);

  pcap_close(handle); // Close the handle
  return 0;
}
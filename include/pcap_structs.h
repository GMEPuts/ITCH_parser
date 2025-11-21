#pragma once
#include <cstdint>

// These are simple, packed versions of the Linux structs,
// matching the sizes used when the firm wrote the file.

#pragma pack(push, 1)

struct file_header_t 
{
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t  thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
};

struct record_header_t
{
    uint32_t ts_sec;
    uint32_t ts_nsec;
    uint32_t incl_len;
    uint32_t orig_len;
};

// minimal Ethernet header (14 bytes)
struct ether_header
{
    uint8_t  dst[6];
    uint8_t  src[6];
    uint16_t ethertype;
};

// minimal IPv4 header (20 bytes, no options)
struct iphdr
{
    uint8_t  version_ihl;
    uint8_t  tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
};

// minimal UDP header (8 bytes)
struct udphdr
{
    uint16_t source;
    uint16_t dest;
    uint16_t len;
    uint16_t check;
};

struct packet_headers_t
{
    static constexpr uint64_t NETWORK_HEADER_LENGTH =
        sizeof(ether_header) +
        sizeof(iphdr) +
        sizeof(udphdr);

    record_header_t m_pcap_hdr;
    ether_header    m_ether_header;
    char            m_vlan_header[4]; // present in their sample
    iphdr           m_iphdr;
    udphdr          m_udphdr;
};

#pragma pack(pop)

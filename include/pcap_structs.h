#pragma once
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

struct __attribute__((packed)) file_header_t {
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t  thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
};

struct __attribute__((packed)) record_header_t {
    uint32_t ts_sec;
    uint32_t ts_nsec;
    uint32_t incl_len;
    uint32_t orig_len;
};

class packet_headers_t {
public:
    static constexpr uint64_t NETWORK_HEADER_LENGTH =
        sizeof(ether_header) +
        sizeof(iphdr) +
        sizeof(udphdr);

    record_header_t m_pcap_hdr;
    ether_header     m_ether_header;
    char             m_vlan_header[4];
    iphdr            m_iphdr;
    udphdr           m_udphdr;
};

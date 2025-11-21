#include <pcap.h>
#include <iostream>
#include <cstring>
#include "../include/itch_parser.h"

// ---- Big-endian helpers ----
static inline uint16_t read_be16(const uint8_t* p) {
    return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}

static inline uint64_t read_be64(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v = (v << 8) | uint64_t(p[i]);
    return v;
}

// ---- MoldUDP64 decoder ----
void process_moldudp64_payload(ITCHParser& parser, const uint8_t* buf, size_t len) {
    if (len < 20) return;

    const uint8_t* p = buf;
    const uint8_t* end = buf + len;

    char session[11];
    memcpy(session, p, 10);
    session[10] = '\0';
    p += 10;

    uint64_t seq = read_be64(p);
    p += 8;

    uint16_t msgCount = read_be16(p);
    p += 2;

    std::cout << "MoldUDP64 session=" << session
              << " seq=" << seq
              << " msgCount=" << msgCount << "\n";

    for (uint16_t i = 0; i < msgCount; ++i) {
        if (p + 2 > end) break;

        uint16_t msgLen = read_be16(p);
        p += 2;

        if (p + msgLen > end) break;

        const uint8_t* msg = p;
        p += msgLen;

        uint16_t symbol_id = parser.parse_message(msg, msgLen);
        if (symbol_id != 0) {
            parser.print_book(symbol_id, 5);
        }
    }
}

// ---- VERY SIMPLE UDP payload extraction ----
void process_pcap_packet(ITCHParser& parser, const uint8_t* pkt, size_t caplen) {
    if (caplen < 42) return;  // Ethernet(14) + IPv4(min20) + UDP(8)

    // Ethernet header = 14 bytes
    const uint8_t* ip = pkt + 14;

    // IPv4 header length
    uint8_t ihl = ip[0] & 0x0F;     // in units of 32-bit words
    size_t ip_header_len = ihl * 4;

    if (caplen < 14 + ip_header_len + 8) return;

    // UDP header is after IP header
    const uint8_t* udp = ip + ip_header_len;

    // UDP total length
    uint16_t udp_len = (udp[4] << 8) | udp[5];
    if (udp_len < 8) return;

    // Payload starts after UDP header
    const uint8_t* payload = udp + 8;
    size_t payload_len = udp_len - 8;

    // Clamp to captured length
    size_t max_avail = caplen - (payload - pkt);
    if (payload_len > max_avail) payload_len = max_avail;

    // Process MoldUDP64 message
    process_moldudp64_payload(parser, payload, payload_len);
}

// ---- main ----
int main() {
    const char* filename = "../data/nasdaq.pcap";

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* handle = pcap_open_offline(filename, errbuf);
    if (!handle) {
        std::cerr << "pcap_open_offline failed: " << errbuf << "\n";
        return 1;
    }

    ITCHParser parser;

    const u_char* packetData;
    struct pcap_pkthdr* header;

    while (true) {
        int rc = pcap_next_ex(handle, &header, &packetData);
        if (rc == 1) {
            process_pcap_packet(parser,
                                reinterpret_cast<const uint8_t*>(packetData),
                                header->caplen);
        }
        else if (rc == -2) break;
        else if (rc == -1) {
            std::cerr << "Error: " << pcap_geterr(handle) << "\n";
            break;
        }
    }

    pcap_close(handle);
    return 0;
}

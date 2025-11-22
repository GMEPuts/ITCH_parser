#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include "../include/itch_parser.h"
#include "../include/pcap_structs.h"

static inline uint16_t be16(const uint8_t* p) {
    return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}

static inline uint64_t be64(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v = (v << 8) | p[i];
    return v;
}

void parse_moldudp(const uint8_t* p, size_t len, ITCHParser& parser) {
    if (len < 20) return;

    // 10-byte session
    char session[11];
    std::memcpy(session, p, 10);
    session[10] = '\0';
    p += 10;

    // 8-byte seq (big-endian)
    uint64_t seq = be64(p);
    p += 8;

    // 2-byte msgCount (big-endian)
    uint16_t msgCount = be16(p);
    p += 2;

    std::cout << "MoldUDP64 session=" << session
              << " seq=" << seq
              << " msgCount=" << msgCount << "\n";

    for (uint16_t i = 0; i < msgCount; ++i) {
        if (p + 2 > p + len) {
            std::cerr << "  Truncated: no space for msg_len\n";
            return;
        }

        uint16_t msgLen = be16(p);
        p += 2;

        if (p + msgLen > p + len) {
            std::cerr << "  Truncated: not enough bytes for ITCH msg\n";
            return;
        }

        const uint8_t* msg = p;
        p += msgLen;

        uint16_t symbol_id = parser.parse_message(msg, msgLen);
        if (symbol_id != 0) {
            parser.print_book(symbol_id, 5);
        }
    }
}

int main() {
    const char* filename = "../data/nasdaq.pcap";

    std::ifstream f(filename, std::ios::binary);
    if (!f) {
        std::cerr << "Cannot open file\n";
        return 1;
    }

    file_header_t file_hdr{};
    if (!f.read(reinterpret_cast<char*>(&file_hdr), sizeof(file_hdr))) {
        std::cerr << "Failed to read PCAP file header\n";
        return 1;
    }

    ITCHParser parser;

    while (true) {
        packet_headers_t pkt{};
        // Read record header
        if (!f.read(reinterpret_cast<char*>(&pkt.m_pcap_hdr),
                    sizeof(record_header_t))) {
            break; // EOF
        }

        // Need enough bytes for ether+ip+udp (42 bytes)
        if (pkt.m_pcap_hdr.incl_len < packet_headers_t::NETWORK_HEADER_LENGTH) {
            f.seekg(pkt.m_pcap_hdr.incl_len, std::ios::cur);
            continue;
        }

        // Read exactly what NETWORK_HEADER_LENGTH counts: ether + ip + udp
        f.read(reinterpret_cast<char*>(&pkt.m_ether_header), sizeof(ether_header));
        std::memset(pkt.m_vlan_header, 0, sizeof(pkt.m_vlan_header));
        f.read(reinterpret_cast<char*>(&pkt.m_iphdr), sizeof(iphdr));
        f.read(reinterpret_cast<char*>(&pkt.m_udphdr), sizeof(udphdr));

        // Payload length and data
        uint32_t payload_len = pkt.m_pcap_hdr.incl_len - packet_headers_t::NETWORK_HEADER_LENGTH;

        std::vector<uint8_t> payload(payload_len);
        if (!f.read(reinterpret_cast<char*>(payload.data()), payload_len)) {
            std::cerr << "Truncated payload\n";
            break;
        }

        parse_moldudp(payload.data(), payload_len, parser);
    }

    return 0;
}

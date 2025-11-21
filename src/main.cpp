#include <cstdint>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include "../include/pcap_structs.h"
#include "../include/itch_parser.h"

// ---- your existing helpers (shortened) ----
static inline uint16_t read_be16(const uint8_t* p) {
    return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}

static inline uint64_t read_be64(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v = (v << 8) | uint64_t(p[i]);
    return v;
}

void process_moldudp64_payload(ITCHParser& parser, const uint8_t* buf, size_t len) {
    if (len < 20) return;

    const uint8_t* p   = buf;
    const uint8_t* end = buf + len;

    char session[11];
    std::memcpy(session, p, 10);
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
int main() {
    const char* filename = "../data/nasdaq.pcap";

    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return 1;
    }

    file_header_t file_hdr{};
    if (!in.read(reinterpret_cast<char*>(&file_hdr), sizeof(file_hdr))) {
        std::cerr << "Failed to read file_header_t\n";
        return 1;
    }

    ITCHParser parser;

    while (true) {
        packet_headers_t pkt_hdr{};

        if (!in.read(reinterpret_cast<char*>(&pkt_hdr), sizeof(pkt_hdr))) {
            if (!in.eof())
                std::cerr << "Error reading packet_headers_t\n";
            break;
        }

        uint32_t incl_len = pkt_hdr.m_pcap_hdr.incl_len;
        if (incl_len < packet_headers_t::NETWORK_HEADER_LENGTH) {
            std::cerr << "incl_len < NETWORK_HEADER_LENGTH, skipping\n";
            continue;
        }

        uint32_t udp_payload_len = incl_len - packet_headers_t::NETWORK_HEADER_LENGTH;
        if (udp_payload_len == 0) continue;

        std::vector<uint8_t> payload(udp_payload_len);
        if (!in.read(reinterpret_cast<char*>(payload.data()), udp_payload_len)) {
            std::cerr << "Truncated UDP payload\n";
            break;
        }

        // MoldUDP64 → ITCH
        process_moldudp64_payload(parser, payload.data(), udp_payload_len);
    }

    return 0;
}

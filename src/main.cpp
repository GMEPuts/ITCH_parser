// src/main.cpp
#include <iostream>

#include <PcapFileDevice.h>
#include <Packet.h>
#include <UdpLayer.h>
#include <PayloadLayer.h>

#include "../include/itch_parser.h"   // your parser header

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: itch_reader <file.pcap>\n";
        return 1;
    }

    const char* filename = argv[1];

    // PcapPlusPlus: automatically picks the right reader type (pcap/pcapng)
    pcpp::IFileReaderDevice* reader = pcpp::IFileReaderDevice::getReader(filename);
    if (reader == nullptr) {
        std::cerr << "Cannot open reader for file: " << filename << "\n";
        return 1;
    }

    if (!reader->open()) {
        std::cerr << "Error opening pcap file: " << filename << "\n";
        delete reader;
        return 1;
    }

    ITCHParser parser;

    pcpp::RawPacket rawPacket;
    while (reader->getNextPacket(rawPacket)) {
        pcpp::Packet parsedPacket(&rawPacket);

        // Only care about UDP packets
        pcpp::UdpLayer* udpLayer = parsedPacket.getLayerOfType<pcpp::UdpLayer>();
        if (!udpLayer)
            continue;

        // UDP payload should be your ITCH (or SoupBinTCP carrying ITCH)
        pcpp::PayloadLayer* payloadLayer = parsedPacket.getLayerOfType<pcpp::PayloadLayer>();
        if (!payloadLayer)
            continue;

        const uint8_t* data = payloadLayer->getPayload();
        size_t len          = payloadLayer->getPayloadLen();

        // Your code: from UDP payload -> ITCH messages
        if (len != 0) {
            parser.parse_message(data, len);
        }


        // If assignment wants "after each packet":
        // parser.print_book(symbol_id, 5);
    }

    reader->close();
    delete reader;
    return 0;
}

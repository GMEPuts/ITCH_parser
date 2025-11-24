ITCH_parser
============

This project parses NASDAQ ITCH messages from a MoldUDP64 stream contained in a PCAP file and prints the top 5 levels of the orderbook after each **relevant** packet. Note: each orderbook print corresponds to the symbol for that packet.

Usage
-----------------------------

- Place your PCAP file at `data/nasdaq.pcap`, or edit `src/main.cpp` to set `filename` to your file's path.
- Build the project with CMake and run the produced executable from the build directory.



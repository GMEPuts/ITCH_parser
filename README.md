ITCH_parser
============

This project parses NASDAQ ITCH messages from a MoldUDP64 stream contained in a PCAP file and prints the top 5 levels of the orderbook after each **relevant** packet. Note: each orderbook print corresponds to the symbol for that packet.

Usage
-----------------------------

- Place your PCAP file at `data/nasdaq.pcap`, or edit `src/main.cpp` to set `filename` to your file's path.
- Build the project with CMake and run the produced executable from the build directory.

Orderbook Print Structure 
------
- Both bids and asks are printed in **descending** order by price. Level 0 is best bid / ask.
- Each level shows {quantity} @ $ {price}
```
=== Orderbook for AMZN (Top 5 Levels) ===
------ Asks ------
level 4 |    100 @  $ 248.70  
level 3 |    200 @  $ 248.60  
level 2 |    200 @  $ 248.50  
level 1 |    100 @  $ 248.40  
level 0 |    100 @  $ 248.32  
------ Bids ------
level 0 |    100 @  $ 247.46  
level 1 |    240 @  $ 246.97  
level 2 |    200 @  $ 246.90  
level 3 |    200 @  $ 246.80  
level 4 |    200 @  $ 246.70  
```
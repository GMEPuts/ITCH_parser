#pragma once
#include <cstdint>
#include <unordered_map>
#include <array>
#include "data_structs.h"
#include "../src/orderbook.cpp"
#include "itch_messages.h"


class ITCHParser {
private:
    std::unordered_map<uint64_t, Order> orders; // key: order_id
    std::array<Orderbook, 65536> books; // indexed by symbol_id (locate code)
    std::array<SymbolInfo, 65536> symbol_lookup; // indexed by symbol_id (locate code)

public:
    ITCHParser() = default;
    // Parse a single ITCH message
    uint16_t parse_message(const uint8_t* buffer, size_t length);

    // Message handlers
    void handle_add_order(const AddOrderMessage& msg);
    void handle_order_executed(const OrderExecutedMessage& msg);
    void handle_order_cancel(const OrderCancelMessage& msg);
    void handle_order_delete(const OrderDeleteMessage& msg);
    void handle_order_replace(const OrderReplaceMessage& msg);

    void print_book(uint16_t symbol_id, size_t levels = 5);
};
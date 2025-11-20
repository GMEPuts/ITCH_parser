#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
#include "data_structs.h"
#include "../src/orderbook.cpp"
#include "itch_messages.h"

class ITCHParser {
private:
    std::unordered_map<uint64_t, Order> orders; // key: order_id
    std::unordered_map<uint64_t, Orderbook> books; // key: symbol_id

public:
    ITCHParser() = default;

    // Parse a single ITCH message
    void parse_message(const uint8_t* buffer, size_t length);

    Orderbook& get_or_create_book(uint64_t symbol_id);

    // Message handlers
    void handle_add_order(const AddOrderMessage& msg);
    void handle_order_executed(const OrderExecutedMessage& msg);
    void handle_order_cancel(const OrderCancelMessage& msg);
    void handle_order_delete(const OrderDeleteMessage& msg);
    void handle_order_replace(const OrderReplaceMessage& msg);

    void print_book(uint64_t symbol_id, size_t levels = 5);
};
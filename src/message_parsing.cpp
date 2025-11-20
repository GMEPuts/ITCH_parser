#include "../include/itch_parser.h"
#include "../include/parsing_helpers.h"
#include <iostream>
#include <cstring>

// Implement parse_message as a member of ITCHParser
void ITCHParser::parse_message(const uint8_t* buffer, size_t length) {
    if (length == 0) return;

    char message_type = static_cast<char>(buffer[0]);

    switch (message_type) {
        case 'A': {  // Add Order (no MPID)
            AddOrderMessage msg{};
            msg.order_id = read_uint64(buffer, 11);
            msg.is_buy = (buffer[19] == 'B');
            msg.quantity = read_uint32(buffer, 20);
            msg.symbol_id = symbol_to_id(reinterpret_cast<const char*>(buffer + 24));
            msg.price = read_uint32(buffer, 32);

            handle_add_order(msg);
            break;
        }

        case 'F': {  // Add Order with MPID
            AddOrderMessage msg{};
            msg.order_id = read_uint64(buffer, 11);
            msg.is_buy = (buffer[19] == 'B');
            msg.quantity = read_uint32(buffer, 20);
            msg.symbol_id = symbol_to_id(reinterpret_cast<const char*>(buffer + 24));
            msg.price = read_uint32(buffer, 32);

            handle_add_order(msg);
            break;
        }

        case 'E': {  // Order Executed
            OrderExecutedMessage msg{};
            msg.order_id = read_uint64(buffer, 11);
            msg.quantity_executed = read_uint32(buffer, 19);

            handle_order_executed(msg);
            break;
        }

        case 'C': {  // Order Executed with Price
            OrderExecutedMessage msg{};
            msg.order_id = read_uint64(buffer, 11);
            msg.quantity_executed = read_uint32(buffer, 19);
            // ignoring execution price

            handle_order_executed(msg);
            break;
        }

        case 'X': {  // Order Cancel
            OrderCancelMessage msg{};
            msg.order_id = read_uint64(buffer, 11);
            msg.quantity_cancelled = read_uint32(buffer, 19);

            handle_order_cancel(msg);
            break;
        }

        case 'D': {  // Order Delete
            OrderDeleteMessage msg{};
            msg.order_id = read_uint64(buffer, 11);

            handle_order_delete(msg);
            break;
        }

        case 'U': {  // Order Replace
            OrderReplaceMessage msg{};
            msg.orig_order_id = read_uint64(buffer, 11);
            msg.new_order_id = read_uint64(buffer, 19);
            msg.quantity = read_uint32(buffer, 27);
            msg.price = read_uint32(buffer, 31);

            handle_order_replace(msg);
            break;
        }

        default:
            break;
    }
}

Orderbook& ITCHParser::get_or_create_book(uint64_t symbol_id) {
    // create book with symbol id constructor arg if it doesnt exist in the map
    auto [it, inserted] = books.try_emplace(symbol_id, symbol_id);
    return it->second;
}

void ITCHParser::handle_add_order(const AddOrderMessage& msg) {
    // Create and store the order
    uint64_t order_id = msg.order_id;
    Order order = msg.create_order();

    // Get or create the orderbook for this symbol
    Orderbook& book = get_or_create_book(msg.symbol_id);
    book.add_quantity(order);

    // add to orders map
    orders.insert({order_id, order});
}

void ITCHParser::handle_order_executed(const OrderExecutedMessage& msg) {
    auto it = orders.find(msg.order_id);
    if (it == orders.end()) return;

    Order& order = it->second;
    order.quantity -= msg.quantity_executed; // reduce quantity

    // Get or create the orderbook for this symbol
    Orderbook& book = get_or_create_book(order.symbol_id);
    book.reduce_quantity(order.price, msg.quantity_executed, order.is_buy);

    if (order.quantity == 0) {
        orders.erase(it);
    }
}

void ITCHParser::handle_order_cancel(const OrderCancelMessage& msg) {
    auto it = orders.find(msg.order_id);
    if (it == orders.end()) return;

    Order& order = it->second;
    order.quantity -= msg.quantity_cancelled; // reduce quantity

    // Get or create the orderbook for this symbol
    Orderbook& book = get_or_create_book(order.symbol_id);

    book.reduce_quantity(order.price, msg.quantity_cancelled, order.is_buy);

    if (order.quantity == 0) { // shouldn't happen since would get a delete instead of cancel
        orders.erase(it);
    }
}

void ITCHParser::handle_order_delete(const OrderDeleteMessage& msg) {
    auto it = orders.find(msg.order_id);
    if (it == orders.end()) return;

    Order& order = it->second;

    Orderbook& book = get_or_create_book(order.symbol_id);
    book.reduce_quantity(order.price, order.quantity, order.is_buy);

    orders.erase(it);
}

void ITCHParser::handle_order_replace(const OrderReplaceMessage& msg) {
    auto it = orders.find(msg.orig_order_id);
    if (it == orders.end()) return;
    
    Order& old_order = it->second;
    Order new_order = msg.create_order(old_order);

    // Delete old order
    Orderbook& book = get_or_create_book(old_order.symbol_id);
    book.reduce_quantity(old_order.price, old_order.quantity, old_order.is_buy);

    // Add new order
    book.add_quantity(new_order);

    orders.insert({msg.new_order_id, new_order});
    orders.erase(it);

}

void ITCHParser::print_book(uint64_t symbol_id, size_t levels) {
    Orderbook& book = get_or_create_book(symbol_id);
    book.print_book(levels);
}
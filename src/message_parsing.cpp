#include "../include/itch_parser.h"
#include "../include/parsing_helpers.h"
#include "../include/constants.h"
#include <iostream>
#include <cstring>

uint16_t ITCHParser::parse_message(const uint8_t* buffer, size_t length) {
    if (length == 0) return 0;

    switch (char message_type = static_cast<char>(buffer[0])) {
        case 'A': {  // Add Order (no MPID)
            AddOrderMessage msg{};
            msg.symbol_id = read_uint16(buffer, 1);;
            msg.order_id = read_uint64(buffer, 11);
            msg.is_buy = (buffer[19] == 'B');
            msg.quantity = read_uint32(buffer, 20);
            if (symbol_lookup[msg.symbol_id].seen == false) {
                // populate symbol lookup with human readable symbol at first occurrence
                char symb[9];
                read_string(buffer, 24, symb, 8);
                symb[8] = '\0'; // null terminate
                symbol_lookup[msg.symbol_id].symbol = clean_symbol(symb);
                symbol_lookup[msg.symbol_id].seen = true;
            }
            msg.price = read_uint32(buffer, 32);

            handle_add_order(msg);
            return msg.symbol_id;
        }

        case 'F': {  // Add Order with MPID
            AddOrderMessage msg{};
            msg.symbol_id = read_uint16(buffer, 1);;
            msg.order_id = read_uint64(buffer, 11);
            msg.is_buy = (buffer[19] == 'B');
            msg.quantity = read_uint32(buffer, 20);
            if (symbol_lookup[msg.symbol_id].seen == false) {
                // populate symbol lookup with human readable symbol at first occurrence
                char symb[9];
                read_string(buffer, 24, symb, 8);
                symb[8] = '\0'; // null terminate
                symbol_lookup[msg.symbol_id].symbol = clean_symbol(symb);
                symbol_lookup[msg.symbol_id].seen = true;
            }
            msg.price = read_uint32(buffer, 32);

            handle_add_order(msg);
            return msg.symbol_id;
        }

        case 'E': {  // Order Executed
            OrderExecutedMessage msg{};
            msg.symbol_id = read_uint16(buffer, 1);
            msg.order_id = read_uint64(buffer, 11);
            msg.quantity_executed = read_uint32(buffer, 19);

            handle_order_executed(msg);
            return msg.symbol_id;
        }

        case 'C': {  // Order Executed with Price
            OrderExecutedMessage msg{};
            msg.symbol_id = read_uint16(buffer, 1);
            msg.order_id = read_uint64(buffer, 11);
            msg.quantity_executed = read_uint32(buffer, 19);
            // ignoring execution price

            handle_order_executed(msg);
            return msg.symbol_id;
        }

        case 'X': {  // Order Cancel
            OrderCancelMessage msg{};
            msg.symbol_id = read_uint16(buffer, 1);
            msg.order_id = read_uint64(buffer, 11);
            msg.quantity_cancelled = read_uint32(buffer, 19);

            handle_order_cancel(msg);
            return msg.symbol_id;
        }

        case 'D': {  // Order Delete
            OrderDeleteMessage msg{};
            msg.symbol_id = read_uint16(buffer, 1);
            msg.order_id = read_uint64(buffer, 11);

            handle_order_delete(msg);
            return msg.symbol_id;
        }

        case 'U': {  // Order Replace
            OrderReplaceMessage msg{};
            msg.symbol_id = read_uint16(buffer, 1);
            msg.orig_order_id = read_uint64(buffer, 11);
            msg.new_order_id = read_uint64(buffer, 19);
            msg.quantity = read_uint32(buffer, 27);
            msg.price = read_uint32(buffer, 31);

            handle_order_replace(msg);
            return msg.symbol_id;
        }

        default:
            std::cout << "Ignoring message type: " << message_type << "\n";
            break;
    }
    return 0;
}

void ITCHParser::handle_add_order(const AddOrderMessage& msg) {
    // Create and store the order
    uint64_t order_id = msg.order_id;
    Order order = msg.create_order();

    // Get the orderbook, initialize if not already
    Orderbook& book = books[msg.symbol_id];
    if (!book.initialized) {
        book.initialize(msg.symbol_id, symbol_lookup[msg.symbol_id].symbol);
    }
    if constexpr (DEBUG_MODE) {
        std::cout << "Book before add order: " << std::endl;
        std::cout << "--------------------------------" << std::endl;
        book.print_book(5);
        std::cout << "--------------------------------" << std::endl;
        msg.display();
    }
    book.add_quantity(order);

    orders.insert({order_id, order});
}

void ITCHParser::handle_order_executed(const OrderExecutedMessage& msg) {
    // Get orderbook, ignore if not initialized
    Orderbook& book = books[msg.symbol_id];
    if (!book.initialized) { return; }

    auto it = orders.find(msg.order_id);
    if (it == orders.end()) return;

    Order& order = it->second;
    order.quantity -= msg.quantity_executed; // reduce quantity

    if constexpr (DEBUG_MODE) {
        std::cout << "Book before order executed: " << std::endl;
        std::cout << "--------------------------------" << std::endl;
        book.print_book(5);
        std::cout << "--------------------------------" << std::endl;
        msg.display();
    }

    book.reduce_quantity(order.price, msg.quantity_executed, order.is_buy);

    if (order.quantity == 0) {
        orders.erase(it);
    }
}

void ITCHParser::handle_order_cancel(const OrderCancelMessage& msg) {
    // Get orderbook, ignore if not initialized
    Orderbook& book = books[msg.symbol_id];
    if (!book.initialized) { return; }

    auto it = orders.find(msg.order_id);
    if (it == orders.end()) return;

    Order& order = it->second;
    order.quantity -= msg.quantity_cancelled; // reduce quantity

    if constexpr (DEBUG_MODE) {
        std::cout << "Book before order cancelled: " << std::endl;
        std::cout << "--------------------------------" << std::endl;
        book.print_book(5);
        std::cout << "--------------------------------" << std::endl;
        msg.display();
    }

    book.reduce_quantity(order.price, msg.quantity_cancelled, order.is_buy);

    if (order.quantity == 0) { // shouldn't happen since would get a delete instead of cancel
        orders.erase(it);
    }
}

void ITCHParser::handle_order_delete(const OrderDeleteMessage& msg) {
    // Get orderbook, ignore if not initialized
    Orderbook& book = books[msg.symbol_id];
    if (!book.initialized) { return; }

    auto it = orders.find(msg.order_id);
    if (it == orders.end()) return;

    Order& order = it->second;

    if constexpr (DEBUG_MODE) {
        std::cout << "Book before order deleted: " << std::endl;
        std::cout << "--------------------------------" << std::endl;
        book.print_book(5);
        std::cout << "--------------------------------" << std::endl;
        msg.display();
    }

    book.reduce_quantity(order.price, order.quantity, order.is_buy);

    orders.erase(it);
}

void ITCHParser::handle_order_replace(const OrderReplaceMessage& msg) {
    // Get orderbook, ignore if not initialized
    Orderbook& book = books[msg.symbol_id];
    if (!book.initialized) { return; }

    auto it = orders.find(msg.orig_order_id);
    if (it == orders.end()) return;
    
    Order& old_order = it->second;
    Order new_order = msg.create_order(old_order.is_buy);

    if constexpr (DEBUG_MODE) {
        std::cout << "Book before order replaced: " << std::endl;
        std::cout << "--------------------------------" << std::endl;
        book.print_book(5);
        std::cout << "--------------------------------" << std::endl;
        msg.display();
    }

    book.reduce_quantity(old_order.price, old_order.quantity, old_order.is_buy);

    // Add new order
    book.add_quantity(new_order);

    orders.insert({msg.new_order_id, new_order});
    orders.erase(it);

}

void ITCHParser::print_book(uint16_t symbol_id, size_t levels) {
    // Get orderbook, ignore if not initialized
    Orderbook& book = books[symbol_id];
    if (!book.initialized) {
        std::cout << "Orderbook for symbol_id " << symbol_id << " not initialized yet." << std::endl;
        return;
    }
    book.print_book(levels);
}
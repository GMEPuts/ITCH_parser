#pragma once
#include <cstdint>
#include "data_structs.h"

// Storing prices internally as integers (raw from itch feed).
// Then only upon printing, will convert to a human-readable decimal format.


// ----- ADD -----
// New order | Message type "A" or "F"
struct AddOrderMessage {
    uint16_t symbol_id;
    uint64_t order_id;
    bool is_buy;
    uint32_t price;
    uint32_t quantity;

    Order create_order() const {
        return Order { symbol_id , price, quantity, is_buy };
    }

    void display() const {
        std::cout << "== Add Order ==" << std::endl;
        std::cout << "Symbol ID: " << symbol_id << std::endl;
        std::cout << "Order ID: " << order_id << std::endl;
        std::cout << "Is Buy: " << is_buy << std::endl;
        std::cout << "Price: " << price << std::endl;
        std::cout << "Quantity: " << quantity << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }
};


// ----- MODIFY -----
// Execution | Message type "E"
struct OrderExecutedMessage {
    uint16_t symbol_id;
    uint64_t order_id;
    uint32_t quantity_executed;

    void display() const {
        std::cout << "== Order Executed ==" << std::endl;
        std::cout << "Symbol ID: " << symbol_id << std::endl;
        std::cout << "Order ID: " << order_id << std::endl;
        std::cout << "Quantity Executed: " << quantity_executed << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }
};

// Execution with different price than original | Message type "C"
struct OrderExecutedWithPriceMessage {
    uint16_t symbol_id;
    uint64_t order_id;
    uint32_t quantity_executed;
    uint32_t execution_price;

    void display() const {
        std::cout << "== Order Executed With Price ==" << std::endl;
        std::cout << "Symbol ID: " << symbol_id << std::endl;
        std::cout << "Order ID: " << order_id << std::endl;
        std::cout << "Quantity Executed: " << quantity_executed << std::endl;
        std::cout << "Execution Price: " << execution_price << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }
};

// Partial cancellation | Message type "X"
struct OrderCancelMessage {
    uint16_t symbol_id;
    uint64_t order_id;
    uint32_t quantity_cancelled;

    void display() const {
        std::cout << "== Order Cancel ==" << std::endl;
        std::cout << "Symbol ID: " << symbol_id << std::endl;
        std::cout << "Order ID: " << order_id << std::endl;
        std::cout << "Quantity Cancelled: " << quantity_cancelled << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }
};

// Full cancellation | Message Type "D"
struct OrderDeleteMessage {
    uint16_t symbol_id;
    uint64_t order_id;

    void display() const {
        std::cout << "== Order Delete ==" << std::endl;
        std::cout << "Symbol ID: " << symbol_id << std::endl;
        std::cout << "Order ID: " << order_id << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }
};


// Cancel & Replace order | Message Type "U"
struct OrderReplaceMessage {
    uint16_t symbol_id;
    uint64_t orig_order_id;
    uint64_t new_order_id;
    uint32_t price;
    uint32_t quantity;

    // remember to remove original order
    Order create_order(bool is_buy) const {
        return Order { symbol_id, price, quantity, is_buy };
    }

    void display() const {
        std::cout << "== Order Replace ==" << std::endl;
        std::cout << "Symbol ID: " << symbol_id << std::endl;
        std::cout << "Original Order ID: " << orig_order_id << std::endl;
        std::cout << "New Order ID: " << new_order_id << std::endl;
        std::cout << "Price: " << price << std::endl;
        std::cout << "Quantity: " << quantity << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }
};

// ----- TRADE -----
// ignoring for now

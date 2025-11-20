#pragma once
#include <cstdint>
#include "data_structs.h"

// Storing prices internally as integers (raw from itch feed).
// Then only upon printing, will convert to a human-readable decimal format.


// ----- ADD -----
// New order | Message type "A" or "F"
struct AddOrderMessage {
    uint64_t symbol_id;
    uint64_t order_id;
    bool is_buy;
    uint32_t price;
    uint32_t quantity;

    Order create_order() const {
        return Order { symbol_id , price, quantity, is_buy };
    }
};


// ----- MODIFY -----
// Execution | Message type "E"
struct OrderExecutedMessage {
    uint64_t order_id;
    uint32_t quantity_executed;
};

// Execution with different price than original | Message type "C"
struct OrderExecutedWithPriceMessage {
    uint64_t order_id;
    uint32_t quantity_executed;
    uint32_t execution_price;
};

// Partial cancellation | Message type "X"
struct OrderCancelMessage {
    uint64_t order_id;
    uint32_t quantity_cancelled;
};

// Full cancellation | Message Type "D"
struct OrderDeleteMessage {
    uint64_t order_id;
};


// Cancel + Replace order | Message Type "U"
struct OrderReplaceMessage {
    uint64_t orig_order_id;
    uint64_t new_order_id;
    uint32_t price;
    uint32_t quantity;

    // remember to remove original order
    Order create_order(const Order& orig_order) const {
        return Order { orig_order.symbol_id, price, quantity, orig_order.is_buy };
    }
};

// ----- TRADE -----
// ignoring for now

#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <string>
#include "../include/data_structs.h"

// To store price levels, I am using std::vector.
// This may not be the most optimal, depending on the number of levels we have to maintain.
// If we have many price levels, vector becomes slow because we have O(N) insert / delete time if not the last level, but good cache locality overall.
// In this case I would use something like std::map<Price, Quantity> for O(log N) search / insert / update time.

constexpr int MAX_PRICE_LEVELS_GUESS = 100;

class Orderbook {
private:
    std::vector<PriceLevel> bids; //sorted descending
    std::vector<PriceLevel> asks; //sorted ascending

    LevelSearchResult search_bids_for_level(uint32_t search_price) {
        // returns whether level exists
        // if exists, returns <true, index>
        // if not exists, returns <false, insertion_index>
        auto it = std::lower_bound(
        bids.begin(),
        bids.end(),
        search_price,
            [](const PriceLevel& level, uint32_t target_price) {
                return level.price > target_price;
            }
        ); // finds first level <= search price
        int idx = static_cast<int>(it - bids.begin());

        if (it != bids.end() && it->price == search_price) {
            // level found
            return LevelSearchResult { true, idx };
        } else {
            // not found
            return LevelSearchResult { false, idx };
        }
    }

    LevelSearchResult search_asks_for_level(uint32_t search_price) {
        // returns whether level exists
        // if exists, returns <true, index>
        // if not exists, returns <false, insertion_index>
        auto it = std::lower_bound(
        asks.begin(),
        asks.end(),
        search_price,
            [](const PriceLevel& level, uint32_t target_price) {
                return level.price < target_price;
            }
        ); // finds first level >= search price
        int idx = static_cast<int>(it - asks.begin());

        if (it != asks.end() && it->price == search_price) {
            // level found
            return LevelSearchResult { true, idx };
        } else {
            // not found
            return LevelSearchResult { false, idx };
        }
    }

public:
    uint16_t symbol_id = 0;
    std::string symbol;
    bool initialized = false;

    Orderbook() = default;

    void initialize(uint16_t symb_id, const char* sym_name) {
        if (initialized) return;          // don't re-init
        symbol_id = symb_id;
        symbol = std::string(sym_name);
        bids.reserve(MAX_PRICE_LEVELS_GUESS);
        asks.reserve(MAX_PRICE_LEVELS_GUESS);
        initialized = true;
    }

    void print_book(size_t levels) {
        std::cout << "\n=== Order Book: " << symbol << " (Top " << levels << " Levels) ===\n";
        std::cout << "Bids:\n";
        for (int i = 0; i < levels && i < bids.size(); ++i) {
            bids[i].display();
        }
        std::cout << "Asks:\n";
        for (int i = 0; i < levels && i < asks.size(); ++i) {
            asks[i].display();
        }
    }

    void add_quantity(const Order& order) {
        // find price level using binary search
        if (order.is_buy) {
            LevelSearchResult result = search_bids_for_level(order.price);
            if (result.found) {
                bids[result.idx].quantity += order.quantity;
            } else {
                bids.insert(bids.begin()+result.idx, PriceLevel{order.price, order.quantity});
            }
        } else {
            LevelSearchResult result = search_asks_for_level(order.price);
            if (result.found) {
                asks[result.idx].quantity += order.quantity;
            } else {
                asks.insert(asks.begin()+result.idx, PriceLevel{order.price, order.quantity});
            }
        }
    }

    void reduce_quantity(uint32_t price, uint32_t quantity, bool is_buy) {
        if (is_buy) {
            LevelSearchResult result = search_bids_for_level(price);
            if (result.found) {
                uint32_t remaining_quantity = bids[result.idx].quantity - quantity;
                if (remaining_quantity == 0) {
                    bids.erase(bids.begin()+result.idx);
                } else {
                    bids[result.idx].quantity = remaining_quantity;
                }
            } // do not do anything if not found
        } else {
            LevelSearchResult result = search_asks_for_level(price);
            if (result.found) {
                uint32_t remaining_quantity = asks[result.idx].quantity - quantity;
                if (remaining_quantity == 0) {
                    asks.erase(asks.begin()+result.idx);
                } else {
                    asks[result.idx].quantity = remaining_quantity;
                }
            } // do not do anything if not found
        }
    }
};
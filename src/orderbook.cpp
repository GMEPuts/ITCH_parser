#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <string>
#include "../include/data_structs.h"
#include "../include/constants.h"
// To store price levels, I am using std::vector.
// This may not be the most optimal, depending on the number of levels we have to maintain.
// If we have many price levels, vector becomes slow because we have O(N) insert / delete time if not the last level, but good cache locality overall.
// In this case I would use something like std::map<Price, Quantity> for O(log N) search / insert / update time.

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
        size_t idx = it - bids.begin();
        bool found = (it != bids.end() && it->price == search_price);
        return {found, idx};
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
        size_t idx = it - asks.begin();
        bool found = (it != asks.end() && it->price == search_price);
        return {found, idx};
    }

public:
    uint16_t symbol_id = 0;
    std::string symbol;
    bool initialized = false;

    Orderbook() = default;

    // lazy initialization
    void initialize(uint16_t symb_id, std::string symb) {
        if (initialized) return;          // don't re-init
        symbol_id = symb_id;
        symbol = symb;
        bids.reserve(MAX_PRICE_LEVELS_GUESS);
        asks.reserve(MAX_PRICE_LEVELS_GUESS);
        initialized = true;
    }

    void print_book(size_t levels) {
        std::cout << "\n=== Orderbook for " << symbol << " (Top " << levels << " Levels) ===\n";

        std::cout << "------ Asks ------\n";
        if (levels > 0 && !asks.empty()) {
            size_t n = std::min(levels, asks.size());
            for (size_t i = 0; i < n; ++i) {
                size_t idx = n - 1 - i;
                asks[idx].display(idx);
            }
        }

        std::cout << "------ Bids ------\n";
        if (levels > 0 && !bids.empty()) {
            size_t n = std::min(levels, bids.size());
            for (size_t i = 0; i < n; ++i) {
                bids[i].display(i);
            }
        }
    }


    void add_quantity(const Order& order) {
        if (order.is_buy) {
            LevelSearchResult result = search_bids_for_level(order.price);

            if (result.found) {
                // add to existing level
                auto& level = bids[result.idx];
                level.quantity += order.quantity;
            } else {
                // insert new level
                bids.insert(bids.begin() + result.idx, PriceLevel{order.price, order.quantity});
            }
        } else {
            LevelSearchResult result = search_asks_for_level(order.price);

            if (result.found) {
                auto& level = asks[result.idx];
                level.quantity += order.quantity;
            } else {
                asks.insert(asks.begin() + result.idx, PriceLevel{order.price, order.quantity});
            }
        }
    }

    void reduce_quantity(uint32_t price, uint32_t quantity, bool is_buy) {
        if (is_buy) {
            LevelSearchResult result = search_bids_for_level(price);
            if (!result.found) return;

            auto& level = bids[result.idx];
            if (quantity >= level.quantity) {
                // quantity should never exceed level quantity, but just in case
                bids.erase(bids.begin() + result.idx);
            } else {
                level.quantity -= quantity;
            }
        } else {
            LevelSearchResult result = search_asks_for_level(price);
            if (!result.found) return;

            auto& level = asks[result.idx];
            if (quantity >= level.quantity) {
                asks.erase(asks.begin() + result.idx);
            } else {
                level.quantity -= quantity;
            }
        }
    }
};
#pragma once
#include <cstdint>
#include <iostream>
#include <iomanip>

inline double price_to_double(uint32_t price) {
    return price / 10000.0; // since implied decimals = 4
}

struct PriceLevel {
    uint32_t price;
    uint32_t quantity;

    void display(size_t level) const {
        std::cout << "level " << level << " | " 
                  << std::setw(6) << std::right << quantity 
                  << " @  $ " 
                  << std::setw(8) << std::left << std::fixed << std::setprecision(2) << price_to_double(price) << "\n";
    }
};

struct Order {
    uint16_t symbol_id;
    uint32_t price;
    uint32_t quantity;
    bool is_buy;

    Order(uint64_t symbol_id, uint32_t price, uint32_t quantity, bool is_buy)
        : symbol_id(symbol_id), price(price), quantity(quantity), is_buy(is_buy) {}
};

struct LevelSearchResult {
    bool found;
    size_t idx; // if not found, this is the index to update, if not found, index to insert at
};

struct SymbolInfo {
    bool seen = false;
    std::string symbol;

    SymbolInfo() = default;
};
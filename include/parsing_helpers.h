#pragma once
#include <cstdint>
#include <cstring>

// ITCH integers are big endian, our cpu (x64/x86) uses little endian, so we have to convert
inline uint16_t read_uint16(const uint8_t* buffer, size_t offset) {
    return (static_cast<uint16_t>(buffer[offset + 0]) << 8) |
            static_cast<uint16_t>(buffer[offset + 1]);
}

inline uint32_t read_uint32(const uint8_t* buffer, size_t offset) {
    return (static_cast<uint32_t>(buffer[offset + 0]) << 24) |
           (static_cast<uint32_t>(buffer[offset + 1]) << 16) |
           (static_cast<uint32_t>(buffer[offset + 2]) << 8)  |
            static_cast<uint32_t>(buffer[offset + 3]);
}

inline uint64_t read_uint64(const uint8_t* buffer, size_t offset) {
    return (static_cast<uint64_t>(buffer[offset + 0]) << 56) |
           (static_cast<uint64_t>(buffer[offset + 1]) << 48) |
           (static_cast<uint64_t>(buffer[offset + 2]) << 40) |
           (static_cast<uint64_t>(buffer[offset + 3]) << 32) |
           (static_cast<uint64_t>(buffer[offset + 4]) << 24) |
           (static_cast<uint64_t>(buffer[offset + 5]) << 16) |
           (static_cast<uint64_t>(buffer[offset + 6]) << 8)  |
            static_cast<uint64_t>(buffer[offset + 7]);
}

inline void read_string(const uint8_t* buffer, size_t offset, char* dest, size_t length) {
    // read into destination buffer
    std::memcpy(dest, buffer + offset, length);
}

#ifndef DEFS_H
#define DEFS_H

//libraries
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <iostream> 
#include <algorithm>
#include <exception>
#include <cassert>
#include <cstdint>
#include <memory>

#define DEMO_NAMESPACE_START namespace DemoJKA {
#define DEMO_NAMESPACE_END }

// Constants - made constexpr for compile-time evaluation
constexpr int MAX_MSGLEN = 49152;
constexpr int NYT = 256; /* NYT = Not Yet Transmitted (HMAX) */
constexpr int INTERNAL_NODE = 257; /* HMAX + 1 */
constexpr int HMAX = 256; /* Maximum symbol */
constexpr int BIG_INFO_STRING = 8192;
constexpr int MAX_STRING_CHARS = 1024;
constexpr int FLOAT_INT_BITS = 13;
constexpr int FLOAT_INT_BIAS = (1 << (FLOAT_INT_BITS - 1));
constexpr int MAX_CONFIGSTRINGS = 1700;
constexpr int GENTITYNUM_BITS = 10;
constexpr int MAX_GENTITIES = (1 << GENTITYNUM_BITS);

// Size constants enum class for type safety
enum class BitSize : int {
    FLOAT = 0,
    FLOATINT = 13,
    ONE_BIT = 1,
    EIGHT_BITS = 8,
    SIXTEEN_BITS = 16,
    NINETEEN_BITS = 19,   // special stuff in ps_stats
    THIRTY_TWO_BITS = 32,
    ENTITY_BITS = 10,
};

// Legacy enum for backward compatibility
enum {
    SIZE_FLOAT = 0,
    SIZE_FLOATINT = 13,
    SIZE_1BIT = 1,
    SIZE_8BITS = 8,
    SIZE_16BITS = 16,
    SIZE_19BITS = 19,   //special stuff in ps_stats
    SIZE_32BITS = 32,
    SIZE_ENTITY_BITS = 10,
};

// Server commands enum class for type safety
enum class ServerCommand : std::uint8_t {
    BAD = 0,
    NOP,
    GAMESTATE,
    CONFIGSTRING,   // [short] [string] only in gamestate messages
    BASELINE,       // only in gamestate messages
    SERVER_COMMAND, // [string] to be executed by client game module
    DOWNLOAD,       // [short] size [size bytes]
    SNAPSHOT,
    SETGAME,
    MAPCHANGE,
    END_OF_FILE
};

// Legacy enum for backward compatibility
enum {
    svc_bad = 0,
    svc_nop,
    svc_gamestate,
    svc_configstring,   // [short] [string] only in gamestate messages
    svc_baseline,       // only in gamestate messages
    svc_serverCommand,  // [string] to be executed by client game module
    svc_download,       // [short] size [size bytes]
    svc_snapshot,
    svc_setgame,
    svc_mapchange,
    svc_EOF
};

// Modern exception class with better inheritance and noexcept specifications
class DemoException : public std::runtime_error {
public:
    explicit DemoException(const char* message) : std::runtime_error(message) {}
    explicit DemoException(const std::string& message) : std::runtime_error(message) {}
    
    // Provide what() method explicitly for clarity
    const char* what() const noexcept override {
        return std::runtime_error::what();
    }
};

// Type aliases for better readability and modern C++
using byte = std::uint8_t;
using Byte = std::uint8_t; // Alternative capitalized version

// Utility functions for common operations
DEMO_NAMESPACE_START

// Helper function to safely convert between signed/unsigned
template<typename T, typename U>
constexpr T safe_cast(U value) noexcept {
    static_assert(std::is_integral_v<T> && std::is_integral_v<U>, 
                  "safe_cast can only be used with integral types");
    return static_cast<T>(value);
}

// Helper to check if a value fits in a given bit size
constexpr bool fits_in_bits(std::int32_t value, int bits) noexcept {
    const std::int32_t max_val = (1 << (bits - 1)) - 1;
    const std::int32_t min_val = -(1 << (bits - 1));
    return value >= min_val && value <= max_val;
}

// Helper to check if an unsigned value fits in given bits
constexpr bool fits_in_unsigned_bits(std::uint32_t value, int bits) noexcept {
    const std::uint32_t max_val = (1u << bits) - 1u;
    return value <= max_val;
}

DEMO_NAMESPACE_END

#endif
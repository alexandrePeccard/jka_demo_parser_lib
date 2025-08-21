// qcommon.hpp - C++ modernized version of qcommon.h
// Shared definitions for demo/network parsing (dm_26, msg_t, huffman, etc.)

#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <string_view>

// ---------------------------------------------------
// Compile-time constants (constexpr instead of macros)
// ---------------------------------------------------

namespace qcommon {

constexpr int MAX_MSGLEN         = 16384;   // maximum size of a network message
constexpr int MAX_PARSE_ENTITIES = 1024;    // maximum entities parsed in one frame
constexpr int MAX_STRING_CHARS   = 1024;    // maximum string length in messages
constexpr int BIG_INFO_STRING    = 8192;    // large buffer for configstrings

// ---------------------------------------------------
// Network message buffer (RAII-friendly)
// ---------------------------------------------------

struct Msg {
    bool        allowOverflow{false};   // if false, errors on overflow
    bool        overflowed{false};      // set if the buffer overflowed
    bool        readOnly{false};        // true if buffer is read-only
    uint8_t*    data{nullptr};          // pointer to message data
    int         maxSize{0};             // size of allocated buffer
    int         curSize{0};             // current size used
    int         readCount{0};           // read pointer
    int         bit{0};                 // bit offset for bit-level access
};

// ---------------------------------------------------
// Network field (for delta compression)
// ---------------------------------------------------

struct NetField {
    std::string_view name;  // field name
    int              offset; // byte offset
    int              bits;   // number of bits used
};

// ---------------------------------------------------
// Service operations (message opcodes)
// ---------------------------------------------------

enum class SvcOp : int {
    Bad = 0,
    Nop,
    GameState,
    ConfigString,
    Baseline,
    ServerCommand,
    Download,
    Snapshot,
    EndOfFile
};

// ---------------------------------------------------
// Error levels
// ---------------------------------------------------

enum class ErrorParm : int {
    Fatal = 0,        // exit the entire program
    Drop,             // print and disconnect
    ServerDisconnect,
    Disconnect,
    NeedCD
};

// ---------------------------------------------------
// Helper template for array length
// ---------------------------------------------------

template <typename T, std::size_t N>
constexpr std::size_t arrayLength(const std::array<T, N>&) noexcept {
    return N;
}

} // namespace qcommon

// qcommon.h - Modernized from OpenJK/ioquake3
// Shared definitions for demo/network parsing (dm_26, msg_t, huffman, etc.)

#ifndef QCOMMON_H
#define QCOMMON_H

#include <cstdint>
#include <cstddef>

// ---------------------------------------------------
// Basic compile-time constants
// ---------------------------------------------------

constexpr int MAX_MSGLEN       = 16384;   // max length of a network message
constexpr int MAX_PARSE_ENTITIES = 1024;  // max entities parsed in one frame
constexpr int MAX_STRING_CHARS = 1024;    // max length of string in network msgs
constexpr int BIG_INFO_STRING  = 8192;    // big configstring buffer

// ---------------------------------------------------
// Network message buffer
// ---------------------------------------------------

struct msg_t {
    bool        allowoverflow;   // if false, errors on overflow
    bool        overflowed;      // set if the buffer overflowed
    bool        readOnly;        // true if buffer is read-only
    uint8_t*    data;            // pointer to message data
    int         maxsize;         // size of allocated buffer
    int         cursize;         // current size used
    int         readcount;       // position for reading
    int         bit;             // bit-level access
};

// ---------------------------------------------------
// Network field (for delta compression)
// ---------------------------------------------------

struct netField_t {
    const char* name;
    int         offset;
    int         bits;        // number of bits used to encode this field
};

// ---------------------------------------------------
// Message types
// ---------------------------------------------------

enum class svc_ops_e : int {
    BAD,                // 0
    NOP,                // 1
    GAMESTATE,          // 2
    CONFIGSTRING,       // 3
    BASELINE,           // 4
    SERVERCOMMAND,      // 5
    DOWNLOAD,           // 6
    SNAPSHOT,           // 7
    EOF_                // 8 (end of file/stream)
};

// ---------------------------------------------------
// Error levels
// ---------------------------------------------------

enum class errorParm_t : int {
    ERR_FATAL,      // exit the entire game
    ERR_DROP,       // print to console and disconnect
    ERR_SERVERDISCONNECT, 
    ERR_DISCONNECT, 
    ERR_NEED_CD
};

// ---------------------------------------------------
// Misc helpers
// ---------------------------------------------------

#ifndef ARRAY_LEN
#define ARRAY_LEN(x) (sizeof(x) / sizeof(*(x)))
#endif

#endif // QCOMMON_H

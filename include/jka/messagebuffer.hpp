#pragma once
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <string>
#include <jka/huffman.hpp>

namespace jka {

    class MessageBuffer {
    public:
        explicit MessageBuffer(size_t maxSize = 16384)
            : data(maxSize), maxsize(maxSize), readcount(0), bit(0),
              cursize(0), overflowed(false) {}

        // Reset the buffer (reuse for new message)
        void clear() {
            readcount = 0;
            bit = 0;
            cursize = 0;
            overflowed = false;
        }

        // Write raw bytes (no compression)
        void writeBytes(const void* src, size_t length) {
            if (cursize + length > maxsize) {
                overflowed = true;
                throw std::runtime_error("MessageBuffer overflow");
            }
            const uint8_t* bytes = static_cast<const uint8_t*>(src);
            std::copy(bytes, bytes + length, data.begin() + cursize);
            cursize += length;
        }

        // Read raw bytes
        void readBytes(void* dest, size_t length) {
            if (readcount + length > cursize) {
                throw std::runtime_error("MessageBuffer read past end");
            }
            std::copy(data.begin() + readcount,
                      data.begin() + readcount + length,
                      static_cast<uint8_t*>(dest));
            readcount += length;
        }

        // Accessors
        const uint8_t* buffer() const { return data.data(); }
        uint8_t* buffer() { return data.data(); }
        size_t size() const { return cursize; }
        size_t capacity() const { return maxsize; }
        bool isOverflowed() const { return overflowed; }

        // --- Huffman streaming interface ---
        // Encode (compress) input into this buffer at offset
        void huffTransmit(HuffmanStream& stream,
                          const uint8_t* input, size_t length,
                          size_t offset = 0) {
            if (offset + length > maxsize) {
                overflowed = true;
                throw std::runtime_error("MessageBuffer overflow in huffTransmit");
            }
            stream.encode(input, length, data.data() + offset, maxsize - offset);
            cursize = offset + length; // update logical size
        }

        // Decode (decompress) from this buffer into output
        void huffReceive(HuffmanStream& stream,
                         uint8_t* output, size_t length,
                         size_t offset = 0) {
            if (offset + length > cursize) {
                throw std::runtime_error("MessageBuffer huffReceive out of range");
            }
            stream.decode(data.data() + offset, length, output, length);
        }

        // Convenience: write one byte through Huffman
        void writeByteHuff(HuffmanStream& stream, uint8_t value) {
            if (cursize + 1 > maxsize) {
                overflowed = true;
                throw std::runtime_error("MessageBuffer overflow in writeByteHuff");
            }
            stream.encode(&value, 1, data.data() + cursize, maxsize - cursize);
            cursize += 1;
        }

        // Convenience: read one byte through Huffman
        uint8_t readByteHuff(HuffmanStream& stream) {
            if (readcount + 1 > cursize) {
                throw std::runtime_error("MessageBuffer read past end in readByteHuff");
            }
            uint8_t value{};
            stream.decode(data.data() + readcount, 1, &value, 1);
            readcount += 1;
            return value;
        }

    private:
        std::vector<uint8_t> data;
        size_t maxsize;
        size_t readcount;
        size_t bit;
        size_t cursize;
        bool overflowed;
    };

    //
    // --- OpenJK/Quake3-style wrappers (modern C++) ---
    //

    // Transmit bytes with adaptive Huffman, into buffer at offset.
    inline void Huff_offsetTransmit(MessageBuffer& msg,
                                    HuffmanStream& stream,
                                    const uint8_t* data,
                                    size_t length,
                                    size_t offset = 0) {
        msg.huffTransmit(stream, data, length, offset);
    }

    // Receive bytes with adaptive Huffman, from buffer at offset.
    inline void Huff_offsetReceive(const MessageBuffer& msg,
                                   HuffmanStream& stream,
                                   uint8_t* output,
                                   size_t length,
                                   size_t offset = 0) {
        // const_cast because huffReceive needs mutable buffer pointer
        const_cast<MessageBuffer&>(msg).huffReceive(stream, output, length, offset);
    }
}

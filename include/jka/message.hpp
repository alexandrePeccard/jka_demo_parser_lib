#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <jka/messagebuffer.hpp>
#include <jka/huffman.hpp>

namespace jka {

    class Message {
    public:
        Message(size_t maxSize = 16384)
            : buffer(maxSize), huffStream(std::make_shared<HuffmanStream>()) {}

        void clear() {
            buffer.clear();
        }

        // --- Raw read/write ---
        void writeBytes(const void* src, size_t length) {
            buffer.writeBytes(src, length);
        }

        void readBytes(void* dest, size_t length) {
            buffer.readBytes(dest, length);
        }

        // --- Huffman streaming read/write ---
        void writeHuffBytes(const uint8_t* src, size_t length) {
            buffer.huffTransmit(*huffStream, src, length, buffer.size());
        }

        void readHuffBytes(uint8_t* dest, size_t length) {
            buffer.huffReceive(*huffStream, dest, length, buffer.size() - length);
        }

        void writeHuffByte(uint8_t value) {
            buffer.writeByteHuff(*huffStream, value);
        }

        uint8_t readHuffByte() {
            return buffer.readByteHuff(*huffStream);
        }

        // --- File I/O (raw, no Huffman) ---
        bool loadFromFile(const std::string& filename) {
            std::ifstream in(filename, std::ios::binary);
            if (!in) return false;

            buffer.clear();
            std::vector<uint8_t> tmp((std::istreambuf_iterator<char>(in)),
                                     std::istreambuf_iterator<char>());
            if (tmp.size() > buffer.capacity()) return false;

            buffer.writeBytes(tmp.data(), tmp.size());
            return true;
        }

        bool saveToFile(const std::string& filename) const {
            std::ofstream out(filename, std::ios::binary);
            if (!out) return false;

            out.write(reinterpret_cast<const char*>(buffer.buffer()), buffer.size());
            return true;
        }

        // --- Accessors ---
        size_t size() const { return buffer.size(); }
        const uint8_t* data() const { return buffer.buffer(); }
        bool overflowed() const { return buffer.isOverflowed(); }

        // Huffman stream accessor (for external reuse)
        std::shared_ptr<HuffmanStream> getHuffStream() { return huffStream; }

    private:
        MessageBuffer buffer;
        std::shared_ptr<HuffmanStream> huffStream; // shared so state can persist across messages if desired
    };

}

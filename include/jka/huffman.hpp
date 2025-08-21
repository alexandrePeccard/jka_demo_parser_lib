/*// huffman.hpp - Modern adaptive Huffman compression for Quake3-style demos
// Based on ioquake3/OpenJK implementation, modernized for C++17+

#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <memory>
#include <limits>
#include <algorithm>

#include "qcommon.hpp"  // for constants (MAX_MSGLEN, etc.)

namespace qcommon {

// ---------------------------------------------------
// Constants
// ---------------------------------------------------

constexpr int HUFFMAN_NODES   = 256 * 2 - 1;   // internal + leaf nodes
constexpr int HUFFMAN_SYMBOLS = 256;           // full byte range

// Special marker for internal node
constexpr int INTERNAL_NODE = -1;

// ---------------------------------------------------
// Huffman Tree Node
// ---------------------------------------------------

struct HuffmanNode {
    int symbol{INTERNAL_NODE};   // byte value (0-255) or INTERNAL_NODE
    int weight{0};               // frequency
    int parent{-1};              // parent node index
    int left{-1};                // left child index
    int right{-1};               // right child index
};

// ---------------------------------------------------
// Huffman Tree (adaptive coding)
// ---------------------------------------------------

class HuffmanTree {
public:
    HuffmanTree();

    // Reset to initial state (with base frequencies)
    void reset();

    // Encode a byte into the bitstream
    void encode(uint8_t symbol, std::vector<uint8_t>& outBits);

    // Decode a byte from the bitstream
    uint8_t decode(const std::vector<uint8_t>& inBits, size_t& bitPos);

private:
    std::array<HuffmanNode, HUFFMAN_NODES> nodes{};
    int numNodes{0};
    int root{-1};

    void buildInitialTree();
    void updateFrequency(int nodeIndex);
    void rebuild();
};

// ---------------------------------------------------
// Huffman Codec (wrapper)
// ---------------------------------------------------

class HuffmanCodec {
public:
    HuffmanCodec();

    // Encode raw buffer into compressed form
    std::vector<uint8_t> compress(const std::vector<uint8_t>& input);

    // Decode compressed buffer back to raw
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& input);

private:
    HuffmanTree tree;
};

// ---------------------------------------------------
// Implementation
// ---------------------------------------------------

inline HuffmanTree::HuffmanTree() {
    reset();
}

inline void HuffmanTree::reset() {
    buildInitialTree();
}

inline void HuffmanTree::buildInitialTree() {
    // Start with uniform frequencies
    numNodes = 0;
    for (int i = 0; i < HUFFMAN_SYMBOLS; i++) {
        nodes[numNodes] = { i, 1, -1, -1, -1 };
        numNodes++;
    }

    // Build tree by combining nodes
    std::vector<int> active;
    for (int i = 0; i < HUFFMAN_SYMBOLS; i++) {
        active.push_back(i);
    }

    while (active.size() > 1) {
        std::sort(active.begin(), active.end(),
                  [&](int a, int b) { return nodes[a].weight < nodes[b].weight; });

        int left = active[0];
        int right = active[1];
        active.erase(active.begin(), active.begin() + 2);

        nodes[numNodes] = { INTERNAL_NODE,
                            nodes[left].weight + nodes[right].weight,
                            -1, left, right };
        nodes[left].parent = numNodes;
        nodes[right].parent = numNodes;
        active.push_back(numNodes);
        numNodes++;
    }

    root = active[0];
}

inline void HuffmanTree::updateFrequency(int nodeIndex) {
    while (nodeIndex != -1) {
        nodes[nodeIndex].weight++;
        nodeIndex = nodes[nodeIndex].parent;
    }
}

inline void HuffmanTree::rebuild() {
    // For simplicity, rebuild completely (could be optimized)
    buildInitialTree();
}

inline void HuffmanTree::encode(uint8_t symbol, std::vector<uint8_t>& outBits) {
    // Traverse back from leaf to root to find code
    int nodeIndex = symbol;
    std::vector<int> code;

    while (nodes[nodeIndex].parent != -1) {
        int parent = nodes[nodeIndex].parent;
        if (nodes[parent].left == nodeIndex)
            code.push_back(0);
        else
            code.push_back(1);
        nodeIndex = parent;
    }

    // Write bits in reverse order
    for (auto it = code.rbegin(); it != code.rend(); ++it) {
        outBits.push_back(static_cast<uint8_t>(*it));
    }

    updateFrequency(symbol);
}

inline uint8_t HuffmanTree::decode(const std::vector<uint8_t>& inBits, size_t& bitPos) {
    int nodeIndex = root;

    while (nodes[nodeIndex].symbol == INTERNAL_NODE) {
        if (bitPos >= inBits.size()) {
            throw std::runtime_error("Huffman decode: out of bits");
        }
        int bit = inBits[bitPos++];
        nodeIndex = (bit == 0) ? nodes[nodeIndex].left : nodes[nodeIndex].right;
    }

    uint8_t symbol = static_cast<uint8_t>(nodes[nodeIndex].symbol);
    updateFrequency(nodeIndex);
    return symbol;
}

inline HuffmanCodec::HuffmanCodec() {
    tree.reset();
}

inline std::vector<uint8_t> HuffmanCodec::compress(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> bits;
    for (uint8_t sym : input) {
        tree.encode(sym, bits);
    }
    return bits;
}

inline std::vector<uint8_t> HuffmanCodec::decompress(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> output;
    size_t bitPos = 0;
    while (bitPos < input.size()) {
        output.push_back(tree.decode(input, bitPos));
    }
    return output;
}

} // namespace qcommon
*/
#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include <stdexcept>

namespace jka {

/// Adaptive Huffman coder/decoder (FGK/Vitter) similaire à OpenJK/ioquake3.
/// Alphabet = 0..255. Deux "symboles" internes :
///  - NYT (Not Yet Transmitted) = 256
///  - INTERNAL_NODE            = 257
class AdaptiveHuffman {
public:
    static constexpr int HMAX = 256;                // 256 symboles octet
    static constexpr int NYT  = HMAX;               // 256
    static constexpr int INTERNAL_NODE = HMAX + 1;  // 257
    static constexpr int NODE_CAPACITY = (HMAX + 1) * 2 + 2;

    AdaptiveHuffman();

    /// Réinitialise entièrement les contextes encodeur/décodeur.
    void reset();

    /// Compresse un buffer (append dans out).
    void compress(const uint8_t* data, size_t len, std::vector<uint8_t>& out);
    std::vector<uint8_t> compress(const std::vector<uint8_t>& in);

    /// Décompresse un buffer (append dans out).
    /// Lance std::runtime_error si le flux est invalide/incomplet.
    void decompress(const uint8_t* data, size_t len, std::vector<uint8_t>& out);
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& in);

private:
    struct Node {
        int   symbol;
        int   weight;
        Node* parent;
        Node* left;
        Node* right;

        // chaînage par poids (ordre croissant), groupes "blocks" de même poids
        Node*  next;
        Node*  prev;
        Node** head; // pointeur vers le "head" du block de même poids
    };

    struct Context {
        std::array<Node, NODE_CAPACITY> pool{};
        int  blocNode{0};

        std::array<Node*, HMAX + 1> loc{}; // feuille pour chaque symbole [0..255]
        Node* lhead{nullptr};              // tête de la liste
        Node* ltail{nullptr};              // fin de la liste
        int   blocPtrs{0};
        std::array<Node**, NODE_CAPACITY> pppool{}; // slots de têtes de blocks

        Node* tree{nullptr};
        Node* nyt{nullptr};

        void  reset();
        Node* newNode();
        Node** getHeadSlot();
    };

    // ----- I/O de bits (LSB-first, comme ioquake3/OpenJK) -----
    class BitWriter {
    public:
        explicit BitWriter(std::vector<uint8_t>& out);
        void putBit(int bit);
        void putByte(uint8_t b);     // 8 bits LSB-first
        void flushPartialByte();     // flush si octet non complet
    private:
        void flushByte();
        std::vector<uint8_t>& out_;
        uint8_t cur_{0};
        int bitpos_{0}; // 0..7
    };

    class BitReader {
    public:
        BitReader(const uint8_t* data, size_t len);
        bool eof() const;
        int  getBit();               // -1 si fin
        bool getByte(uint8_t& out);  // 8 bits LSB-first; false si fin avant 8 bits
    private:
        const uint8_t* data_{nullptr};
        size_t len_{0};
        size_t pos_{0}; // index d’octet
        int    bitpos_{0};
    };

    // ---- mécanique Huffman adaptatif ----
    static bool isLeaf(const Node* n);
    Node* highestInBlock(Node* n);
    void   swap(Context& h, Node* a, Node* b);
    void   increment(Context& h, Node* n);
    void   addRef(Context& h, int ch);

    void   sendPathToNode(Context& h, BitWriter& bw, Node* n);
    void   sendSymbol(Context& h, BitWriter& bw, int ch);

    bool   receiveSymbol(Context& h, BitReader& br, int& outSym);

    Context enc_;
    Context dec_;
};

} // namespace jka

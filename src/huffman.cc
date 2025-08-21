#include "huffman.h"
#include <stdexcept>
#include <cstring>

namespace DemoJKA {

Huffman::Huffman() {
    initTree();
}

void Huffman::initTree() {
    nodes.clear();
    for (int i = 0; i < 256; i++) {
        auto node = std::make_unique<Node>();
        node->weight = 1;
        node->symbol = i;
        decodeTree[i] = node.get();
        nodes.push_back(std::move(node));
    }
}

int Huffman::decompress(const uint8_t* in, int inSize, uint8_t* out, int outSize) const {
    if (!in || !out || inSize <= 0 || outSize <= 0) {
        throw std::invalid_argument("Invalid buffer passed to Huffman::decompress");
    }
    // ⚠️ Ici tu devras réimplémenter MSG_ReadBits() ou équivalent
    // en t’appuyant sur les netfields pour reconstruire l’arbre réel.
    // Pour l’instant on fait une copie brute (placeholder).
    int toCopy = (inSize < outSize) ? inSize : outSize;
    std::memcpy(out, in, toCopy);
    return toCopy;
}

int Huffman::compress(const uint8_t* in, int inSize, std::vector<uint8_t>& out) const {
    if (!in || inSize <= 0) {
        throw std::invalid_argument("Invalid buffer passed to Huffman::compress");
    }
    // ⚠️ Même remarque : ici, compression Huffman réelle à coder.
    out.assign(in, in + inSize);
    return inSize;
}

} // namespace DemoJKA

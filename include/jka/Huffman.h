#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace DemoJKA {

class Huffman {
public:
    Huffman();
    ~Huffman() = default;

    [[nodiscard]] int decompress(const uint8_t* in, int inSize, uint8_t* out, int outSize) const;
    [[nodiscard]] int compress(const uint8_t* in, int inSize, std::vector<uint8_t>& out) const;

private:
    struct Node {
        int weight = 0;
        int symbol = -1;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    using NodePool = std::vector<std::unique_ptr<Node>>;

    std::array<Node*, 256> decodeTree{};  
    NodePool nodes;                      

    void initTree();
};

} // namespace DemoJKA

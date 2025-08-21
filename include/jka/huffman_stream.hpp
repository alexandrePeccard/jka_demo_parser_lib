#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <stdexcept>

// -----------------------------------------------------------------------------
// Hypothèses d’API sur ton AdaptiveHuffman (mode streaming) :
//   - conserve son état entre appels
//   - fournit :
//        void sendSymbol(uint8_t sym, auto writeBit /* callable: void(int bit) */);
//        int  receiveSymbol(auto readBit /* callable: int() -> 0|1 */);
//     (si tes noms diffèrent, adapte juste les deux appels marqués "TODO rename").
// -----------------------------------------------------------------------------

namespace jka {

/// Helpers bas-niveau pour lire/écrire des bits dans un buffer binaire avec offset en bits.
/// Ces helpers sont robustes (offset peut dépasser la taille -> on étend le buffer si std::vector,
/// sinon on jette une exception pour le pointeur brut).

// ---------------------- Écriture bit à bit ----------------------
inline void writeBitToBuffer(std::vector<uint8_t>& out, uint32_t& bitOffset, int bit) {
    const uint32_t byteIndex = bitOffset >> 3;
    const uint32_t bitIndex  = bitOffset & 7;

    if (byteIndex >= out.size())
        out.resize(byteIndex + 1, 0);

    if (bit)
        out[byteIndex] |= static_cast<uint8_t>(1u << bitIndex);

    ++bitOffset;
}

inline void writeBitToBuffer(uint8_t* out, size_t outCapacityBytes, uint32_t& bitOffset, int bit) {
    const uint32_t byteIndex = bitOffset >> 3;
    const uint32_t bitIndex  = bitOffset & 7;

    if (!out || byteIndex >= outCapacityBytes)
        throw std::out_of_range("Huff_offsetTransmit: out of capacity");

    if (bit)
        out[byteIndex] |= static_cast<uint8_t>(1u << bitIndex);
    else
        out[byteIndex] &= static_cast<uint8_t>(~(1u << bitIndex));

    ++bitOffset;
}

// ---------------------- Lecture bit à bit ----------------------
inline int readBitFromBuffer(const std::vector<uint8_t>& in, uint32_t& bitOffset) {
    const uint32_t byteIndex = bitOffset >> 3;
    const uint32_t bitIndex  = bitOffset & 7;

    if (byteIndex >= in.size())
        throw std::out_of_range("Huff_offsetReceive: not enough input bits");

    const int bit = (in[byteIndex] >> bitIndex) & 1;
    ++bitOffset;
    return bit;
}

inline int readBitFromBuffer(const uint8_t* in, size_t inSizeBytes, uint32_t& bitOffset) {
    const uint32_t byteIndex = bitOffset >> 3;
    const uint32_t bitIndex  = bitOffset & 7;

    if (!in || byteIndex >= inSizeBytes)
        throw std::out_of_range("Huff_offsetReceive: not enough input bits");

    const int bit = (in[byteIndex] >> bitIndex) & 1;
    ++bitOffset;
    return bit;
}

// -----------------------------------------------------------------------------
// Wrappers "OpenJK-compatibles": Huff_offsetTransmit / Huff_offsetReceive
// -----------------------------------------------------------------------------

class AdaptiveHuffman; // fwd (fourni par ton huffman.hpp)

/// Version std::vector<uint8_t>
/// - out est agrandi automatiquement au besoin
/// - bitOffset est mis à jour
void Huff_offsetTransmit(AdaptiveHuffman& h, int ch,
                         std::vector<uint8_t>& out, uint32_t& bitOffset);

/// Version pointeur brut + capacité en octets
/// - lève std::out_of_range si la capacité est dépassée
void Huff_offsetTransmit(AdaptiveHuffman& h, int ch,
                         uint8_t* out, size_t outCapacityBytes, uint32_t& bitOffset);

/// Version std::vector<uint8_t>
/// - lit un symbole depuis in à partir de bitOffset
/// - lève std::out_of_range si les bits ne suffisent pas
int Huff_offsetReceive(AdaptiveHuffman& h,
                       const std::vector<uint8_t>& in, uint32_t& bitOffset);

/// Version pointeur brut + taille
/// - lève std::out_of_range si les bits ne suffisent pas
int Huff_offsetReceive(AdaptiveHuffman& h,
                       const uint8_t* in, size_t inSizeBytes, uint32_t& bitOffset);


// -----------------------------------------------------------------------------
// Helpers d’intégration MessageBuffer (facultatif mais pratique)
// -----------------------------------------------------------------------------

/// Écrit un octet compressé (un symbole) dans out, conserve l’état du huffman (streaming)
inline void Huff_WriteByte(AdaptiveHuffman& h, uint8_t value,
                           std::vector<uint8_t>& out, uint32_t& bitOffset) {
    Huff_offsetTransmit(h, static_cast<int>(value), out, bitOffset);
}

/// Lit un octet compressé (un symbole) depuis in, conserve l’état du huffman (streaming)
inline uint8_t Huff_ReadByte(AdaptiveHuffman& h,
                             const std::vector<uint8_t>& in, uint32_t& bitOffset) {
    return static_cast<uint8_t>(Huff_offsetReceive(h, in, bitOffset));
}

/// Variante pointeur/longueur si ton MessageBuffer stocke un bloc binaire brut
inline void Huff_WriteByte(AdaptiveHuffman& h, uint8_t value,
                           uint8_t* out, size_t outCapacityBytes, uint32_t& bitOffset) {
    Huff_offsetTransmit(h, static_cast<int>(value), out, outCapacityBytes, bitOffset);
}

inline uint8_t Huff_ReadByte(AdaptiveHuffman& h,
                             const uint8_t* in, size_t inSizeBytes, uint32_t& bitOffset) {
    return static_cast<uint8_t>(Huff_offsetReceive(h, in, inSizeBytes, bitOffset));
}

} // namespace jka

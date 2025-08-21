#include "huffman_stream.hpp"
#include "huffman.hpp" // doit fournir la classe jka::AdaptiveHuffman avec sendSymbol / receiveSymbol

namespace jka {

// NOTE IMPORTANTE :
// Si tes noms sont différents, adapte les deux TODO ci-dessous.
// - TODO(1): appel encode -> h.sendSymbol(symbol, writeBit)
// - TODO(2): appel decode -> return h.receiveSymbol(readBit)

// ----------------- Transmit (encode un symbole) -----------------

void Huff_offsetTransmit(AdaptiveHuffman& h, int ch,
                         std::vector<uint8_t>& out, uint32_t& bitOffset)
{
    // callable capturant out & bitOffset
    auto writeBit = [&out, &bitOffset](int bit) {
        writeBitToBuffer(out, bitOffset, bit);
    };

    // TODO(1): si ta méthode s'appelle autrement, remplace ici
    h.sendSymbol(static_cast<uint8_t>(ch), writeBit);
}

void Huff_offsetTransmit(AdaptiveHuffman& h, int ch,
                         uint8_t* out, size_t outCapacityBytes, uint32_t& bitOffset)
{
    auto writeBit = [out, outCapacityBytes, &bitOffset](int bit) {
        writeBitToBuffer(out, outCapacityBytes, bitOffset, bit);
    };

    // TODO(1): si ta méthode s'appelle autrement, remplace ici
    h.sendSymbol(static_cast<uint8_t>(ch), writeBit);
}

// ----------------- Receive (décode un symbole) -----------------

int Huff_offsetReceive(AdaptiveHuffman& h,
                       const std::vector<uint8_t>& in, uint32_t& bitOffset)
{
    auto readBit = [&in, &bitOffset]() -> int {
        return readBitFromBuffer(in, bitOffset);
    };

    // TODO(2): si ta méthode s'appelle autrement, remplace ici
    return h.receiveSymbol(readBit);
}

int Huff_offsetReceive(AdaptiveHuffman& h,
                       const uint8_t* in, size_t inSizeBytes, uint32_t& bitOffset)
{
    auto readBit = [in, inSizeBytes, &bitOffset]() -> int {
        return readBitFromBuffer(in, inSizeBytes, bitOffset);
    };

    // TODO(2): si ta méthode s'appelle autrement, remplace ici
    return h.receiveSymbol(readBit);
}

} // namespace jka

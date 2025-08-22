#pragma once
#include <memory>
#include <vector>
#include "messagebuffer.h"
#include "snapshot_instr.h"

namespace jka {

/**
 * SnapshotParser = couche bas niveau.
 * Il lit le flux binaire (MessageBuffer + Huffman) et construit
 * un SnapshotInstr brut qui contient les instructions PlayerStateInstr
 * et EntityStateInstr sans les convertir.
 */
class SnapshotParser {
public:
    explicit SnapshotParser(MessageBuffer &buffer) : buffer_(buffer) {}

    // Parse un snapshot brut
    std::unique_ptr<SnapshotInstr> parse() {
        auto snap = std::make_unique<SnapshotInstr>();

        // Lecture des métadonnées
        snap->serverTime = buffer_.readInt();
        snap->messageNum = buffer_.readInt();
        snap->deltaNum   = buffer_.readInt();
        snap->snapFlags  = buffer_.readInt();

        // Lecture du areamask
        int areamaskLen = buffer_.readByte();
        snap->areamask.resize(areamaskLen);
        for (int i = 0; i < areamaskLen; i++) {
            snap->areamask[i] = buffer_.readByte();
        }

        // Lecture PlayerStateInstr
        snap->playerStateInstr = std::make_unique<PlayerStateInstr>();
        snap->playerStateInstr->read(buffer_);

        // Lecture EntityStateInstr[]
        int entityCount = buffer_.readShort();
        for (int i = 0; i < entityCount; i++) {
            auto entInstr = std::make_unique<EntityStateInstr>();
            entInstr->read(buffer_);
            snap->entitiesInstr.push_back(std::move(entInstr));
        }

        return snap;
    }

private:
    MessageBuffer &buffer_;
};

} // namespace jka

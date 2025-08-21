#pragma once

#include <vector>
#include <memory>
#include <iostream>

#include "instruction.h"
#include "playerstate_instr.h"
#include "entitystate_instr.h"
#include "state.h"        // PlayerState / EntityState
#include "messagebuffer.h"

namespace DemoJKA {

/**
 * Snapshot complet d’un instant de jeu.
 * Contient : 
 *  - un PlayerStateInstr
 *  - une collection d’EntityStateInstr
 */
class Snapshot : public Instruction {
private:
    int serverTime;
    std::unique_ptr<PlayerStateInstr> playerStateInstr;
    std::vector<std::unique_ptr<EntityStateInstr>> entities;

public:
    Snapshot() 
        : Instruction(INSTR_SNAPSHOT), serverTime(0),
          playerStateInstr(std::make_unique<PlayerStateInstr>()) {}

    explicit Snapshot(int stime) 
        : Instruction(INSTR_SNAPSHOT), serverTime(stime),
          playerStateInstr(std::make_unique<PlayerStateInstr>()) {}

    // I/O
    void Save() const override {
        if (playerStateInstr) playerStateInstr->Save();
        for (const auto& ent : entities) {
            if (ent) ent->Save();
        }
    }

    void Load() override {
        // On suppose qu’un MessageBuffer global est actif
        MessageBuffer& buf = MessageBuffer::getGlobal();

        serverTime = buf.readInt();

        // PlayerState
        playerStateInstr = std::make_unique<PlayerStateInstr>();
        playerStateInstr->Load();

        // Entities
        entities.clear();
        int numEntities = buf.readInt();
        for (int i = 0; i < numEntities; i++) {
            auto instr = std::make_unique<EntityStateInstr>();
            instr->Load();
            entities.push_back(std::move(instr));
        }
    }

    void report(std::ostream& os) const override {
        os << "[Snapshot] serverTime=" << serverTime 
           << " entities=" << entities.size() << "\n";
        if (playerStateInstr) {
            os << "   ";
            playerStateInstr->report(os);
            os << "\n";
        }
        for (const auto& ent : entities) {
            os << "   ";
            if (ent) ent->report(os);
            os << "\n";
        }
    }

    // Getters
    int getServerTime() const noexcept { return serverTime; }
    const PlayerStateInstr* getPlayerStateInstr() const noexcept { return playerStateInstr.get(); }
    const std::vector<std::unique_ptr<EntityStateInstr>>& getEntities() const noexcept { return entities; }

    // Mutators
    void setServerTime(int t) noexcept { serverTime = t; }
    void setPlayerStateInstr(std::unique_ptr<PlayerStateInstr> ps) { playerStateInstr = std::move(ps); }
    void addEntityInstr(std::unique_ptr<EntityStateInstr> es) { entities.push_back(std::move(es)); }
};

} // namespace DemoJKA

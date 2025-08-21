#pragma once

#include <memory>
#include <iostream>
#include "instruction.h"
#include "state.h"   // pour PlayerState

namespace DemoJKA {

/**
 * Instruction encapsulant un PlayerState dans un snapshot.
 */
class PlayerStateInstr : public Instruction {
private:
    std::unique_ptr<PlayerState> playerState;

public:
    PlayerStateInstr()
        : Instruction(INSTR_SNAPSHOT), playerState(std::make_unique<PlayerState>()) {}

    explicit PlayerStateInstr(std::unique_ptr<PlayerState> ps)
        : Instruction(INSTR_SNAPSHOT), playerState(std::move(ps)) {}

    // I/O
    void Save() const override {
        if (playerState) playerState->Save();
    }

    void Load() override {
        if (playerState) playerState->Load();
    }

    void report(std::ostream& os) const override {
        os << "[PlayerStateInstr] ";
        if (playerState) playerState->report(os);
    }

    // Getters
    const PlayerState* getPlayerState() const noexcept { return playerState.get(); }
    PlayerState* getPlayerState() noexcept { return playerState.get(); }

    // Setters
    void setPlayerState(std::unique_ptr<PlayerState> ps) { playerState = std::move(ps); }
};

} // namespace DemoJKA

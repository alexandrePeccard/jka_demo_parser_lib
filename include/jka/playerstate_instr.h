#pragma once
// playerstate_instr.hpp — Instruction qui encapsule un jka::PlayerState
//
// - Homogène avec EntityStateInstr.
// - Stocke un PlayerState moderne (jka::PlayerState).
// - Pas de parsing direct (SnapshotParser s’en charge).
// - Support de report() pour debug.

#include <memory>
#include <ostream>

#include "instruction.h"          // base Instruction + INSTR_SNAPSHOT
#include "playerstate.hpp"        // jka::PlayerState moderne
#include "playerstate_instr.hpp"    // jka::report(const PlayerState&) -> std::string

namespace jka {

class PlayerStateInstr : public Instruction {
public:
    PlayerStateInstr()
        : Instruction(INSTR_SNAPSHOT)
        , state_(std::make_unique<jka::PlayerState>()) {}

    explicit PlayerStateInstr(const jka::PlayerState& ps)
        : Instruction(INSTR_SNAPSHOT)
        , state_(std::make_unique<jka::PlayerState>(ps)) {}

    explicit PlayerStateInstr(jka::PlayerState&& ps)
        : Instruction(INSTR_SNAPSHOT)
        , state_(std::make_unique<jka::PlayerState>(std::move(ps))) {}

    explicit PlayerStateInstr(std::unique_ptr<jka::PlayerState> ps)
        : Instruction(INSTR_SNAPSHOT)
        , state_(std::move(ps)) {}

    // --- I/O (no-op ici, séparation parsing/objet) --------------------------
    void Save() const override {}
    void Load() override {}

    // --- Debug ---------------------------------------------------------------
    void report(std::ostream& os) const override {
        os << "[PlayerStateInstr]\n";
        if (state_) {
            os << jka::report(*state_);
        } else {
            os << "  (null state)\n";
        }
    }

    // --- Accès ---------------------------------------------------------------
    const jka::PlayerState* state() const noexcept { return state_.get(); }
    jka::PlayerState*       state()       noexcept { return state_.get(); }

    void setState(const jka::PlayerState& ps) {
        ensureState();
        *state_ = ps;
    }
    void setState(jka::PlayerState&& ps) {
        ensureState();
        *state_ = std::move(ps);
    }
    void setState(std::unique_ptr<jka::PlayerState> ps) {
        state_ = std::move(ps);
    }

private:
    void ensureState() {
        if (!state_) state_ = std::make_unique<jka::PlayerState>();
    }

    std::unique_ptr<jka::PlayerState> state_;
};

    // --- Comparaison ---------------------------------------------------------
    friend bool operator==(const PlayerStateInstr& a, const PlayerStateInstr& b) {
        if (!a.state_ && !b.state_) return true;
        if (!a.state_ || !b.state_) return false;
        return *a.state_ == *b.state_;
    }

    friend bool operator!=(const PlayerStateInstr& a, const PlayerStateInstr& b) {
        return !(a == b);
    }

}

namespace std {
    template<>
    struct hash<DemoJKA::PlayerStateInstr> {
        std::size_t operator()(const DemoJKA::PlayerStateInstr& instr) const noexcept {
            if (const auto* ps = instr.getPlayerState()) {
                return std::hash<DemoJKA::PlayerState>{}(*ps);
            }
            return 0;
        }
    };
}

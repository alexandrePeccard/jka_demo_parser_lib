#pragma once
// entitystate_instr.hpp — Instruction qui encapsule un jka::EntityState
//
// - Reste compatible avec la hiérarchie "Instruction" existante (INSTR_SNAPSHOT).
// - Stocke un EntityState moderne (jka::EntityState) avec Vec3i/Vec3 + Trajectory.
// - Ne parse pas: la logique de parsing reste dans SnapshotParser, on ne fait
//   ici que transporter des données et fournir du reporting/debug.

#include <memory>
#include <ostream>

#include "instruction.h"         // base Instruction + INSTR_SNAPSHOT
#include "entitystate.hpp"       // jka::EntityState (moderne)
#include "entitystate_instr.h"   // jka::report(const EntityState&) -> std::string

namespace DemoJKA {

class EntityStateInstr : public Instruction {
public:
    EntityStateInstr()
        : Instruction(INSTR_SNAPSHOT)
        , entityNum_(-1)
        , state_(std::make_unique<jka::EntityState>()) {}

    explicit EntityStateInstr(int num)
        : Instruction(INSTR_SNAPSHOT)
        , entityNum_(num)
        , state_(std::make_unique<jka::EntityState>()) {}

    EntityStateInstr(int num, const jka::EntityState& es)
        : Instruction(INSTR_SNAPSHOT)
        , entityNum_(num)
        , state_(std::make_unique<jka::EntityState>(es)) {}

    EntityStateInstr(int num, jka::EntityState&& es)
        : Instruction(INSTR_SNAPSHOT)
        , entityNum_(num)
        , state_(std::make_unique<jka::EntityState>(std::move(es))) {}

    EntityStateInstr(int num, std::unique_ptr<jka::EntityState> es)
        : Instruction(INSTR_SNAPSHOT)
        , entityNum_(num)
        , state_(std::move(es)) {}

    // --- I/O (no-op ici) ----------------------------------------------------
    // La séparation parsing/objet implique que cette instruction ne (dé)sérialise
    // plus le wire-format elle-même. SnapshotParser se charge de remplir l’Instr.
    void Save() const override {}
    void Load() override {}

    // --- Debug/Reporting -----------------------------------------------------
    void report(std::ostream& os) const override {
        os << "[EntityStateInstr] id=" << entityNum_ << "\n";
        if (state_) {
            os << jka::report(*state_);
        } else {
            os << "  (null state)\n";
        }
    }

    // --- Accès ---------------------------------------------------------------
    int entityNum() const noexcept { return entityNum_; }
    void setEntityNum(int num) noexcept { entityNum_ = num; }

    const jka::EntityState* state() const noexcept { return state_.get(); }
    jka::EntityState*       state()       noexcept { return state_.get(); }

    void setState(const jka::EntityState& es) {
        ensureState();
        *state_ = es;
    }
    void setState(jka::EntityState&& es) {
        ensureState();
        *state_ = std::move(es);
    }
    void setState(std::unique_ptr<jka::EntityState> es) {
        state_ = std::move(es);
    }

private:
    void ensureState() {
        if (!state_) state_ = std::make_unique<jka::EntityState>();
    }

    int entityNum_;
    std::unique_ptr<jka::EntityState> state_;
};

// --- Comparaison ---------------------------------------------------------
friend bool operator==(const EntityStateInstr& a, const EntityStateInstr& b) {
    if (a.entityNum != b.entityNum) return false;
    if (!a.entityState && !b.entityState) return true;
    if (!a.entityState || !b.entityState) return false;
    return *a.entityState == *b.entityState;
}

friend bool operator!=(const EntityStateInstr& a, const EntityStateInstr& b) {
    return !(a == b);
}

} // namespace DemoJKA
namespace std {
    template<>
    struct hash<DemoJKA::EntityStateInstr> {
        std::size_t operator()(const DemoJKA::EntityStateInstr& instr) const noexcept {
            std::size_t h = std::hash<int>{}(instr.getEntityNum());
            if (const auto* es = instr.getEntityState()) {
                h ^= std::hash<DemoJKA::EntityState>{}(*es) + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
            }
            return h;
        }
    };
}

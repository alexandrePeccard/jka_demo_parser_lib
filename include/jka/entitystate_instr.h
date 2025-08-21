#pragma once

#include <memory>
#include <iostream>
#include "instruction.h"
#include "state.h"   // pour EntityState

namespace DemoJKA {

/**
 * Instruction encapsulant un EntityState (par ID) dans un snapshot.
 */
class EntityStateInstr : public Instruction {
private:
    int entityNum;
    std::unique_ptr<EntityState> entityState;

public:
    EntityStateInstr()
        : Instruction(INSTR_SNAPSHOT), entityNum(-1), entityState(std::make_unique<EntityState>()) {}

    EntityStateInstr(int num, std::unique_ptr<EntityState> es)
        : Instruction(INSTR_SNAPSHOT), entityNum(num), entityState(std::move(es)) {}

    // I/O
    void Save() const override {
        if (entityState) entityState->Save();
    }

    void Load() override {
        if (entityState) entityState->Load();
    }

    void report(std::ostream& os) const override {
        os << "[EntityStateInstr] id=" << entityNum << " ";
        if (entityState) entityState->report(os);
    }

    // Getters
    int getEntityNum() const noexcept { return entityNum; }
    const EntityState* getEntityState() const noexcept { return entityState.get(); }
    EntityState* getEntityState() noexcept { return entityState.get(); }

    // Setters
    void setEntityNum(int num) noexcept { entityNum = num; }
    void setEntityState(std::unique_ptr<EntityState> es) { entityState = std::move(es); }
};

} // namespace DemoJKA

#pragma once
#include <memory>
#include <iostream>
#include "instruction.h"
#include "snapshot.hpp"
#include "snapshot_adapter.hpp"

namespace DemoJKA {

/**
 * Instruction encapsulant un Snapshot complet dans la timeline.
 *
 * Hérite de Instruction pour s’intégrer dans le parsing/écriture.
 * Wrappe un jka::Snapshot moderne via unique_ptr.
 */
class SnapshotInstr : public Instruction {
private:
    std::unique_ptr<jka::Snapshot> snapshot;

public:
    SnapshotInstr()
        : Instruction(INSTR_SNAPSHOT), snapshot(std::make_unique<jka::Snapshot>()) {}

    explicit SnapshotInstr(std::unique_ptr<jka::Snapshot> snap)
        : Instruction(INSTR_SNAPSHOT), snapshot(std::move(snap)) {}

    // Clone profond
    std::unique_ptr<SnapshotInstr> clone() const {
        return std::make_unique<SnapshotInstr>(
            std::make_unique<jka::Snapshot>(*snapshot)
        );
    }

    // I/O (à implémenter côté parsing DM_26)
    void Save() const override {
        // TODO: encoder snapshot->playerState, entities, etc.
    }
    void Load() override {
        // TODO: remplir snapshot depuis un flux DM_26
    }

    void report(std::ostream& os) const override {
        os << "[SnapshotInstr] " << snapshot->toString();
    }

    // Accès direct au snapshot moderne
    jka::Snapshot*       getSnapshot()       noexcept { return snapshot.get(); }
    const jka::Snapshot* getSnapshot() const noexcept { return snapshot.get(); }

    // Wrappers compatibles avec l’API historique
    int getServertime() const noexcept { return snapshot->serverTime; }
    int getDeltanum()   const noexcept { return snapshot->deltaNum; }
    int getFlags()      const noexcept { return snapshot->flags; }

    jka::PlayerState* getPlayerState()       noexcept { return &snapshot->playerState; }
    jka::PlayerState* getVehicleState()      noexcept { return &snapshot->vehicleState; }
    const jka::PlayerState* getPlayerState() const noexcept { return &snapshot->playerState; }
    const jka::PlayerState* getVehicleState()const noexcept { return &snapshot->vehicleState; }

    auto&       getEntities()       noexcept { return snapshot->entities; }
    const auto& getEntities() const noexcept { return snapshot->entities; }

    // Delta application
    void applyOn(const SnapshotInstr& base) {
        *snapshot = jka::applyDelta(*base.snapshot, *snapshot);
    }
    void removeNotChanged(const SnapshotInstr& ref) {
        jka::removeNotChanged(*snapshot, *ref.snapshot);
    }
};

} // namespace DemoJKA

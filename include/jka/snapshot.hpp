#ifndef SNAPSHOT_HPP
#define SNAPSHOT_HPP

#include <jka/defs.h>
#include <jka/state.h>
#include <jka/messagebuffer.h>
#include <jka/instruction.h>

#include <vector>
#include <memory>
#include <ostream>

DEMO_NAMESPACE_START

class PlayerState;
class VehicleState;
class EntityState;

/**
 * Snapshot (serverTime, delta, flags, areaMask, player/vehicle state, entities)
 *
 * — Déporté dans ce header pour casser les dépendances circulaires,
 *   tout en gardant l’API existante utilisée par le projet.
 * — La mémoire des states est modernisée via unique_ptr ; les getters
 *   renvoient des pointeurs bruts pour rester compatibles.
 */
class Snapshot : public Instruction {
protected:
    int serverTime{0};
    int deltaNum{0};
    int flags{0};
    std::vector<byte> areaMask;

    // Modernisation mémoire : unique_ptr (compat getters bruts)
    std::unique_ptr<PlayerState>  playerState;
    std::unique_ptr<PlayerState>  vehicleState;

    // Map d’entités (héritée de Instruction)
    entitymap entities;

public:
    Snapshot() : Instruction(INSTR_SNAPSHOT) {}
    ~Snapshot() override = default;

    // Non copiable (snapshots référencent des pointeurs/ressources)
    Snapshot(const Snapshot&) = delete;
    Snapshot& operator=(const Snapshot&) = delete;

    // Clone « profond » moderne (si besoin dans tes outils)
    std::unique_ptr<Snapshot> clone() const;

    // I/O (implémentations dans le .cpp de ton projet)
    void Save() const override;
    void Load() override;              // ← lit DM_26 et « ajoute » ps, veh, entities
    void report(std::ostream& os) const override;

    // Accès
    int  getAreamaskLen() const noexcept { return static_cast<int>(areaMask.size()); }
    int  getAreamask(int id) const { return static_cast<int>(areaMask.at(id)); }
    int  getDeltanum() const noexcept { return deltaNum; }
    int  getServertime() const noexcept { return serverTime; }
    int  getSnapflags() const noexcept { return flags; }

    PlayerState*       getPlayerstate()       noexcept { return playerState.get(); }
    PlayerState*       getVehiclestate()      noexcept { return vehicleState.get(); }
    const PlayerState* getPlayerstate() const noexcept { return playerState.get(); }
    const PlayerState* getVehiclestate()const noexcept { return vehicleState.get(); }

    // Mutateurs
    void setAreamaskLen(int value) { areaMask.resize(static_cast<size_t>(value)); }
    void setAreamask(int id, int v) { areaMask.at(id) = static_cast<byte>(v); }

    void setSnapflags(int v) noexcept { flags = v; }
    void setDeltanum(int v) noexcept { deltaNum = v; }
    void setServertime(int v) noexcept { serverTime = v; }

    // Helpers « init » / « delta » (conservés)
    void makeInit();          // enlève tous les 0 pour un snap non-compressé initial
    void removeNotChanged();  // gèle les champs non modifiés (delta)

    // Application de delta/coupe (conservées)
    void applyOn(Snapshot* snap);
    void delta(Snapshot* snap);

    // Accès à la map d’entités (compat historique)
    entitymap&       getEntities()       noexcept { return entities; }
    const entitymap& getEntities() const noexcept { return entities; }
};

DEMO_NAMESPACE_END

#endif // SNAPSHOT_HPP
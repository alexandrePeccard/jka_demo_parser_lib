#pragma once
#include "snapshot.hpp"

namespace jka {

/**
 * Marque le snapshot comme "init" :
 * - garantit que tous les champs sont valides,
 * - supprime les optimisations de compression/delta.
 */
inline void makeInit(Snapshot& snap) {
    snap.deltaNum = 0;
    snap.flags    = 0;
    // Rien de particulier côté states : ils sont complets par construction
}

/**
 * Supprime les champs identiques par rapport à une référence.
 * Utile pour stocker/émettre un delta compact.
 */
inline void removeNotChanged(Snapshot& target, const Snapshot& reference) {
    if (target.playerState == reference.playerState) {
        target.playerState = PlayerState{}; // reset si identique
    }
    if (target.vehicleState == reference.vehicleState) {
        target.vehicleState = PlayerState{};
    }

    // Parcours des entités
    for (auto it = target.entities.begin(); it != target.entities.end(); ) {
        auto jt = reference.entities.find(it->first);
        if (jt != reference.entities.end() && jt->second == it->second) {
            it = target.entities.erase(it); // supprime si identique
        } else {
            ++it;
        }
    }
}

/**
 * Applique un delta sur un snapshot de base pour reconstruire un snapshot complet.
 */
inline Snapshot applyDelta(const Snapshot& base, const Snapshot& delta) {
    Snapshot out = base;

    out.serverTime = delta.serverTime;
    out.deltaNum   = delta.deltaNum;
    out.flags      = delta.flags;

    if (!(delta.playerState == PlayerState{})) {
        out.playerState = delta.playerState;
    }
    if (!(delta.vehicleState == PlayerState{})) {
        out.vehicleState = delta.vehicleState;
    }

    // Fusion entités
    for (const auto& [id, es] : delta.entities) {
        out.entities[id] = es;
    }

    return out;
}

} // namespace jka

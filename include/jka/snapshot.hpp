#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include "playerstate.hpp"
#include "entitystate.hpp"

namespace jka {

/**
 * Snapshot moderne — encapsule un état complet du monde
 * (playerState, vehicleState, entities, métadonnées).
 *
 * Conçu comme modèle de données pur, sans logique de parsing/delta.
 */
struct Snapshot {
    int serverTime{0};                     ///< Temps serveur (ms)
    int deltaNum{0};                       ///< Numéro de delta
    int flags{0};                          ///< Flags divers
    std::vector<std::uint8_t> areaMask;    ///< Masque des zones visibles

    PlayerState playerState;               ///< Joueur principal
    PlayerState vehicleState;              ///< État véhicule (optionnel)
    std::unordered_map<int, EntityState> entities; ///< Map entityNum → état

    // --- Helpers entités -----------------------------------------------------

    /// Retourne un pointeur vers l’entité (ou nullptr si absente)
    EntityState* findEntity(int num) {
        auto it = entities.find(num);
        return (it != entities.end()) ? &it->second : nullptr;
    }
    const EntityState* findEntity(int num) const {
        auto it = entities.find(num);
        return (it != entities.end()) ? &it->second : nullptr;
    }

    /// Ajoute ou met à jour une entité
    EntityState& upsertEntity(int num, const EntityState& es) {
        return entities[num] = es;
    }

    /// Supprime une entité
    void removeEntity(int num) {
        entities.erase(num);
    }

    // --- Debug ---------------------------------------------------------------

    std::string toString() const {
        std::ostringstream oss;
        oss << "Snapshot{time=" << serverTime
            << ", delta=" << deltaNum
            << ", flags=" << flags
            << ", areaMask=" << areaMask.size()
            << ", entities=" << entities.size()
            << "}";
        return oss.str();
    }

    // --- Comparaisons --------------------------------------------------------
    friend bool operator==(const Snapshot& a, const Snapshot& b) {
        return a.serverTime == b.serverTime &&
               a.deltaNum   == b.deltaNum &&
               a.flags      == b.flags &&
               a.areaMask   == b.areaMask &&
               a.playerState == b.playerState &&
               a.vehicleState == b.vehicleState &&
               a.entities == b.entities;
    }
    friend bool operator!=(const Snapshot& a, const Snapshot& b) { return !(a==b); }
};

} // namespace jka

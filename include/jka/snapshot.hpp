#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <algorithm>

#include "playerstate.hpp"
#include "entitystate.hpp"
#include "usercmd.hpp"   // (moderne) jka::UserCommand

namespace jka {

/**
 * Événement UserCmd dans un snapshot : on conserve le client,
 * le commandTime (issu du UserCommand), et la commande décodée.
 * NOTE: Les démos DM_26 peuvent contenir plusieurs commandes
 *       (et de plusieurs clients) entre deux snapshots.
 */
struct UserCommandSample {
    int clientNum{0};
    // Redondant avec cmd.commandTime, mais pratique pour requêtes :
    int commandTime{0};
    UserCommand cmd{};

    friend bool operator==(const UserCommandSample& a, const UserCommandSample& b) {
        return a.clientNum == b.clientNum &&
               a.commandTime == b.commandTime &&
               a.cmd == b.cmd;
    }
    friend bool operator!=(const UserCommandSample& a, const UserCommandSample& b) { return !(a==b); }
};

/**
 * Snapshot moderne — modèle de données pur (aucune logique de parsing/delta ici).
 * Représente l’état serveur (DM_26) à un instant donné, ainsi que les commandes
 * joueurs observées entre le snapshot précédent et celui-ci.
 *
 * Ce type est rempli par snapshot_parser.hpp (flux binaire → Instr),
 * puis snapshot_adapter.hpp (Instr → objets modernes).
 */
struct Snapshot {
    // --- Métadonnées snapshot ------------------------------------------------
    int serverTime{0};                      ///< Temps serveur (ms)
    int deltaNum{0};                        ///< Numéro de delta (par ex. -1 pour full snap)
    int flags{0};                           ///< Flags divers (SNAPFLAG_* si besoin)
    std::vector<std::uint8_t> areaMask;     ///< Masque d’aires PVS/PAS

    // --- États principaux ----------------------------------------------------
    PlayerState playerState;                ///< Joueur principal (point de vue)
    PlayerState vehicleState;               ///< Optionnel selon jeu/mod

    // Map entityNum → état (états réseau des entités)
    std::unordered_map<int, EntityState> entities;

    // --- Commandes joueurs vues dans l’intervalle précédent → ce snapshot ---
    // On capture *toutes* les commandes observées (serveur/démo) afin de pouvoir
    // reconstruire l’input timeline si besoin.
    std::vector<UserCommandSample> usercmds;

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

    /// Ajoute ou met à jour une entité (upsert)
    EntityState& upsertEntity(int num, const EntityState& es) {
        return entities[num] = es;
    }

    /// Supprime une entité
    void removeEntity(int num) {
        entities.erase(num);
    }

    // --- Helpers UserCmd -----------------------------------------------------

    /// Ajoute une commande joueur pour un client donné
    void addUserCommand(int clientNum, const UserCommand& cmd) {
        usercmds.push_back(UserCommandSample{
            clientNum,
            cmd.commandTime, // recopie: pratique pour tri/filtrage
            cmd
        });
    }

    /// Renvoie un vecteur filtré des commandes d’un client pour ce snapshot
    std::vector<const UserCommand*> getUserCommandsForClient(int clientNum) const {
        std::vector<const UserCommand*> out;
        out.reserve(usercmds.size());
        for (const auto& s : usercmds) {
            if (s.clientNum == clientNum) out.push_back(&s.cmd);
        }
        return out;
    }

    /// Dernière commande (par commandTime) pour un client dans ce snapshot (ou nullptr)
    const UserCommand* getLastUserCommand(int clientNum) const {
        const UserCommand* last = nullptr;
        int                lastTime = std::numeric_limits<int>::min();
        for (const auto& s : usercmds) {
            if (s.clientNum == clientNum && s.commandTime >= lastTime) {
                lastTime = s.commandTime;
                last     = &s.cmd;
            }
        }
        return last;
    }

    /// Tri optionnel des usercmds par (commandTime, clientNum) pour analyses
    void sortUserCommands() {
        std::sort(usercmds.begin(), usercmds.end(),
                  [](const UserCommandSample& a, const UserCommandSample& b) {
                      if (a.commandTime != b.commandTime) return a.commandTime < b.commandTime;
                      return a.clientNum < b.clientNum;
                  });
    }

    // --- Debug ---------------------------------------------------------------

    std::string toString() const {
        std::ostringstream oss;
        oss << "Snapshot{time=" << serverTime
            << ", delta=" << deltaNum
            << ", flags=" << flags
            << ", areaMask=" << areaMask.size()
            << ", entities=" << entities.size()
            << ", usercmds=" << usercmds.size()
            << "}";
        return oss.str();
    }

    // --- Comparaisons --------------------------------------------------------

    friend bool operator==(const Snapshot& a, const Snapshot& b) {
        return a.serverTime  == b.serverTime  &&
               a.deltaNum    == b.deltaNum    &&
               a.flags       == b.flags       &&
               a.areaMask    == b.areaMask    &&
               a.playerState == b.playerState &&
               a.vehicleState== b.vehicleState&&
               a.entities    == b.entities    &&
               a.usercmds    == b.usercmds;
    }
    friend bool operator!=(const Snapshot& a, const Snapshot& b) { return !(a==b); }
};

} // namespace jka

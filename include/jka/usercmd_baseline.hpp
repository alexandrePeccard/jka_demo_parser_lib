#pragma once
#include <unordered_map>
#include <memory>
#include "usercmd.hpp"

namespace jka {

    /**
     * Gestion centralisée des baselines UserCommand.
     * Chaque client possède un dernier UserCommand (baseline)
     * pour permettre la compression/delta (DM_26).
     */
    class UserCmdBaseline {
    private:
        // clientNum -> dernier UserCommand
        std::unordered_map<int, UserCommand> baselines;

    public:
        UserCmdBaseline() = default;

        /// Vérifie si un baseline existe pour un client donné
        bool hasBaseline(int clientNum) const {
            return baselines.find(clientNum) != baselines.end();
        }

        /// Récupère un baseline (nullptr si absent)
        const UserCommand* getBaseline(int clientNum) const {
            auto it = baselines.find(clientNum);
            return (it != baselines.end()) ? &it->second : nullptr;
        }

        /// Met à jour / installe le baseline d’un client
        void updateBaseline(int clientNum, const UserCommand& cmd) {
            baselines[clientNum] = cmd;
        }

        /// Supprime le baseline pour un client (ex. fin de connexion)
        void removeBaseline(int clientNum) {
            baselines.erase(clientNum);
        }

        /// Réinitialise toutes les baselines
        void clear() {
            baselines.clear();
        }
    };

}

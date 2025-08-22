#pragma once
#include <cstddef>
#include <string_view>
#include <array>

#include "playerstate.hpp"
#include "entitystate.hpp"
#include "trajectory.hpp"
#include "event.hpp"
#include "usercommand.hpp"

namespace jka {

    /**
     * @brief Description d’un champ sérialisé dans une structure réseau.
     */
    struct NetField {
        std::string_view name;  ///< Nom du champ tel qu’il apparaît dans le protocole
        size_t offset;          ///< Décalage dans la structure associée
        size_t size;            ///< Taille en octets
    };

    /**
     * @brief Table des champs d’un PlayerState
     */
    constexpr std::array<NetField, 5> playerStateFields = {{
        {"commandTime", offsetof(PlayerState, commandTime), sizeof(int)},
        {"origin",      offsetof(PlayerState, origin),      sizeof(PlayerState::origin)},
        {"velocity",    offsetof(PlayerState, velocity),    sizeof(PlayerState::velocity)},
        {"weapon",      offsetof(PlayerState, weapon),      sizeof(int)},
        {"health",      offsetof(PlayerState, health),      sizeof(int)},
        // TODO: compléter avec les autres champs de PlayerState
    }};

    /**
     * @brief Table des champs d’une EntityState
     */
    constexpr std::array<NetField, 4> entityStateFields = {{
        {"number",   offsetof(EntityState, number),   sizeof(int)},
        {"pos",      offsetof(EntityState, pos),      sizeof(Trajectory)},
        {"apos",     offsetof(EntityState, apos),     sizeof(Trajectory)},
        {"event",    offsetof(EntityState, event),    sizeof(int)},
        // TODO: compléter avec les autres champs de EntityState
    }};

    /**
     * @brief Table des champs d’un UserCmd
     */
    constexpr std::array<NetField, 4> userCmdFields = {{
        {"serverTime", offsetof(UserCommand, serverTime), sizeof(int)},
        {"angles",     offsetof(UserCommand, angles),     sizeof(UserCommand::angles)},
        {"buttons",    offsetof(UserCommand, buttons),    sizeof(int)},
        {"weapon",     offsetof(UserCommand, weapon),     sizeof(int)},
        // TODO: compléter avec les autres champs de UserCommand
    }};
}
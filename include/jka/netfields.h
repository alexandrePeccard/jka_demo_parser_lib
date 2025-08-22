#pragma once
#include <string_view>
#include <array>
#include <optional>
#include <algorithm>
#include <span>

namespace jka {

    using std::string_view;

    // =============================
    // Types de champs
    // =============================
    enum class FieldType {
        Int,
        Float,
        Angle,
        String,
        Entity,
        Origin,
        Time,
        Vector
    };

    struct NetField {
        string_view name;
        FieldType type;
        int offset;
        int bits;
        int divisor;

        constexpr bool isSigned() const noexcept {
            return type == FieldType::Int || type == FieldType::Time;
        }
        constexpr int getBitWidth() const noexcept { return bits; }
        constexpr int getDivisor()  const noexcept { return divisor; }
    };

    // =============================
    // Champs d’EntityState
    // =============================
    constexpr std::array EntityNetfields{
        NetField{"number",        FieldType::Int,    0,  10, 1},
        NetField{"eType",         FieldType::Int,    4,   8, 1},
        NetField{"torsoAnim",     FieldType::Int,    8,  10, 1},
        NetField{"legsAnim",      FieldType::Int,   12,  10, 1},
        NetField{"pos.trBase",    FieldType::Origin,16,  24, 8},
        NetField{"apos.trBase",   FieldType::Angle, 28,  24, 360},
        NetField{"time",          FieldType::Time,  40,  32, 1},
        NetField{"otherEntityNum",FieldType::Entity,44,  10, 1},
        NetField{"weapon",        FieldType::Int,   48,   8, 1},
        NetField{"clientNum",     FieldType::Int,   52,   8, 1}
    };

    // =============================
    // Champs de PlayerState
    // =============================
    constexpr std::array PlayerNetfields{
        NetField{"commandTime",    FieldType::Time,   0,  32, 1},
        NetField{"pm_type",        FieldType::Int,    4,   8, 1},
        NetField{"origin",         FieldType::Origin, 8,  24, 8},
        NetField{"velocity",       FieldType::Vector,20,  24, 8},
        NetField{"weaponTime",     FieldType::Int,   32,  16, 1},
        NetField{"gravity",        FieldType::Int,   36,  16, 1},
        NetField{"speed",          FieldType::Int,   40,  16, 1},
        NetField{"delta_angles",   FieldType::Angle, 44,  24, 360},
        NetField{"groundEntityNum",FieldType::Entity,56,  10, 1},
        NetField{"legsAnim",       FieldType::Int,   60,  10, 1},
        NetField{"torsoAnim",      FieldType::Int,   64,  10, 1},
        NetField{"movementDir",    FieldType::Int,   68,   8, 1},
        NetField{"eventSequence",  FieldType::Int,   72,  16, 1},
        NetField{"events",         FieldType::Int,   76,  16, 1},
        NetField{"eventParms",     FieldType::Int,   80,  16, 1},
        NetField{"externalEvent",  FieldType::Int,   84,  16, 1},
        NetField{"externalEventParm",FieldType::Int, 88, 16, 1},
        NetField{"clientNum",      FieldType::Int,   92,   8, 1},
        NetField{"weapon",         FieldType::Int,   96,   8, 1},
        NetField{"viewangles",     FieldType::Angle,100,  24, 360}
    };

    // =============================
    // Champs de PilotState
    // =============================
    constexpr std::array PilotNetfields{
        NetField{"origin",         FieldType::Origin, 0,  24, 8},
        NetField{"velocity",       FieldType::Vector,12,  24, 8},
        NetField{"angles",         FieldType::Angle, 24,  24, 360},
        NetField{"angularVelocity",FieldType::Vector,36,  24, 8},
        NetField{"weapon",         FieldType::Int,   48,   8, 1},
        NetField{"health",         FieldType::Int,   52,  10, 1},
        NetField{"armor",          FieldType::Int,   56,  10, 1}
    };

    // =============================
    // ConfigStrings (exemple JKA)
    // =============================
    constexpr std::array ConfigStringNames{
        "CS_MUSIC", "CS_MESSAGE", "CS_MOTD", "CS_WARMUP",
        "CS_VOTE_TIME", "CS_VOTE_STRING", "CS_VOTE_YES", "CS_VOTE_NO",
        "CS_GAME_VERSION", "CS_LEVEL_START_TIME", "CS_INTERMISSION",
        "CS_MODELS", "CS_SOUNDS", "CS_PLAYERS", "CS_ITEMS"
    };

    // =============================
    // ServerCommand types
    // =============================
    enum class SvcCommand {
        Bad, Nop, GameState, ConfigString,
        Baseline, ServerCommand, Download, Snapshot,
        EOFCommand
    };

    // =============================
    // Event types (exemple simplifié)
    // =============================
    enum class EventType {
        Footstep,
        FireWeapon,
        Jump,
        Death,
        Respawn
    };

    // =============================
    // Type global
    // =============================
    enum class NetfieldType {
        Entity,
        Player,
        Pilot
    };

    // =============================
    // API générique
    // =============================
    constexpr std::span<const NetField> getNetfields(NetfieldType type) {
        switch (type) {
            case NetfieldType::Entity: return EntityNetfields;
            case NetfieldType::Player: return PlayerNetfields;
            case NetfieldType::Pilot:  return PilotNetfields;
        }
        return {};
    }

    constexpr std::optional<NetField> findFieldByName(NetfieldType type, string_view name) {
        for (auto&& f : getNetfields(type))
            if (f.name == name) return f;
        return std::nullopt;
    }
}
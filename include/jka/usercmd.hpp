#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>
#include "Vec3.hpp"

namespace jka {

/**
 * Enum des boutons (bg_public.h — OpenJK)
 * NB: Enum class type-safe, chaque valeur est un masque (bitflag).
 */
enum class Button : uint32_t {
    ATTACK          = 1 << 0,
    TALK            = 1 << 1,
    USE_HOLDABLE    = 1 << 2,
    WALKING         = 1 << 3,
    CROUCH          = 1 << 4,
    PRONE           = 1 << 5,   // (si utilisé par mods)
    ZOOM            = 1 << 6,
    SPRINT          = 1 << 7,
    MELEE           = 1 << 8,
    GRAPPLE         = 1 << 9,
    ACTIVATE        = 1 << 10,
    ALT_ATTACK      = 1 << 11,
    FORCE_LIGHTNING = 1 << 12,
    FORCE_GRIP      = 1 << 13,
    FORCE_PUSH      = 1 << 14,
    FORCE_PULL      = 1 << 15,
    USE_FORCE       = 1 << 16,
    SABERTHROW      = 1 << 17,
    // NB : bg_public.h définit parfois d’autres boutons réservés
    // (par ex. BUTTON_FORCEPOWER_NEXT / PREV pour cycle)
    FORCEPOWER_NEXT = 1 << 18,
    FORCEPOWER_PREV = 1 << 19,
    INVENTORY_NEXT  = 1 << 20,
    INVENTORY_PREV  = 1 << 21,
    RELOAD          = 1 << 22,
    LEAN_LEFT       = 1 << 23,
    LEAN_RIGHT      = 1 << 24,
    // ... extensions mod si besoin
};

/// Commande utilisateur moderne (équivalent usercmd_t)
struct UserCommand {
    int32_t serverTime{0};       ///< Horodatage client → serveur
    Vec3T<int32_t> angles{};     ///< Viewangles (pitch, yaw, roll) → int16 en réseau
    int8_t forwardmove{0};       ///< -127..127
    int8_t rightmove{0};         ///< -127..127
    int8_t upmove{0};            ///< -127..127
    uint8_t weapon{0};           ///< Index d’arme
    uint32_t buttons{0};         ///< Masque de boutons (bitfield)

    // Helpers boutons
    bool hasButton(Button b) const noexcept {
        return (buttons & static_cast<uint32_t>(b)) != 0;
    }
    void setButton(Button b) noexcept {
        buttons |= static_cast<uint32_t>(b);
    }
    void clearButton(Button b) noexcept {
        buttons &= ~static_cast<uint32_t>(b);
    }

    // JSON export (debug/trace)
    friend void to_json(nlohmann::json& j, const UserCommand& c) {
        j = {
            {"serverTime", c.serverTime},
            {"angles", c.angles},
            {"forwardmove", c.forwardmove},
            {"rightmove", c.rightmove},
            {"upmove", c.upmove},
            {"weapon", c.weapon},
            {"buttons", c.buttons}
        };
    }
    friend void from_json(const nlohmann::json& j, UserCommand& c) {
        j.at("serverTime").get_to(c.serverTime);
        j.at("angles").get_to(c.angles);
        j.at("forwardmove").get_to(c.forwardmove);
        j.at("rightmove").get_to(c.rightmove);
        j.at("upmove").get_to(c.upmove);
        j.at("weapon").get_to(c.weapon);
        j.at("buttons").get_to(c.buttons);
    }
};

} // namespace jka

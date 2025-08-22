#pragma once
// jka/playerstate.hpp — Modern PlayerState (réseau DM_26) avec Vec3T<int32_t>

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <nlohmann/json.hpp>

#include "jka/Vec3.hpp"        // Vec3T<T>, Vec3, Vec3i
#include "jka/trajectory.hpp"  // pour cohérence (même si Trajectory pas utilisé ici)
#include "jka/netfields.hpp"   // pour NetField defs éventuelles

namespace jka {

    /// PlayerState moderne (équivalent `playerState_t` dans q_shared.h, DM_26)
    /// - Conçu pour représenter l'état réseau du joueur
    /// - Coordonnées et angles en Vec3i (quantification réseau)
    /// - Autres champs scalaires en int32_t
    struct PlayerState {
        // ---- Champs réseau scalaires ----
        int32_t commandTime {0};     ///< Horodatage de la dernière commande
        int32_t pm_type {0};         ///< Type de mouvement (enum PM_*)
        int32_t weapon {0};          ///< Arme active
        int32_t groundEntityNum {0}; ///< Index entité sur laquelle le joueur est posé
        int32_t legsAnim {0};        ///< Animation des jambes
        int32_t torsoAnim {0};       ///< Animation du torse
        int32_t eFlags {0};          ///< Flags d’état
        int32_t externalEvent {0};   ///< Événement réseau externe
        int32_t clientNum {0};       ///< Identifiant du joueur
        int32_t ping {0};            ///< Ping en ms

        // ---- Champs réseau vectoriels (quantifiés int32) ----
        Vec3i origin {0,0,0};        ///< Position du joueur (quantifiée)
        Vec3i velocity {0,0,0};      ///< Vitesse du joueur (quantifiée)
        Vec3i viewangles {0,0,0};    ///< Angles de vue (quantifiés en ticks)

        // ---- Collections (tailles fixes dans protocole, dynamiques ici) ----
        std::vector<int32_t> stats;       ///< Stats joueur (santé, armure, etc.)
        std::vector<int32_t> persistant;  ///< Stats persistantes
        std::vector<int32_t> ammo;        ///< Quantités de munitions
        std::vector<int32_t> powerups;    ///< Powerups actifs

        // ---- Champs additionnels (extensibilité mod) ----
        std::unordered_map<std::string, int64_t> extras; ///< Champs supplémentaires éventuels

        // ---- Helpers Netfields ----
        std::optional<int64_t> getByNetfieldName(const std::string& name) const {
            if (name == "commandTime") return commandTime;
            if (name == "pm_type") return pm_type;
            if (name == "weapon") return weapon;
            if (name == "groundEntityNum") return groundEntityNum;
            if (name == "legsAnim") return legsAnim;
            if (name == "torsoAnim") return torsoAnim;
            if (name == "eFlags") return eFlags;
            if (name == "externalEvent") return externalEvent;
            if (name == "clientNum") return clientNum;
            if (name == "ping") return ping;

            if (name == "origin[0]") return origin.x;
            if (name == "origin[1]") return origin.y;
            if (name == "origin[2]") return origin.z;

            if (name == "velocity[0]") return velocity.x;
            if (name == "velocity[1]") return velocity.y;
            if (name == "velocity[2]") return velocity.z;

            if (name == "viewangles[0]") return viewangles.x;
            if (name == "viewangles[1]") return viewangles.y;
            if (name == "viewangles[2]") return viewangles.z;

            auto it = extras.find(name);
            if (it != extras.end()) return it->second;

            return std::nullopt;
        }

        bool setByNetfieldName(const std::string& name, int64_t value) {
            if (name == "commandTime") { commandTime = static_cast<int32_t>(value); return true; }
            if (name == "pm_type") { pm_type = static_cast<int32_t>(value); return true; }
            if (name == "weapon") { weapon = static_cast<int32_t>(value); return true; }
            if (name == "groundEntityNum") { groundEntityNum = static_cast<int32_t>(value); return true; }
            if (name == "legsAnim") { legsAnim = static_cast<int32_t>(value); return true; }
            if (name == "torsoAnim") { torsoAnim = static_cast<int32_t>(value); return true; }
            if (name == "eFlags") { eFlags = static_cast<int32_t>(value); return true; }
            if (name == "externalEvent") { externalEvent = static_cast<int32_t>(value); return true; }
            if (name == "clientNum") { clientNum = static_cast<int32_t>(value); return true; }
            if (name == "ping") { ping = static_cast<int32_t>(value); return true; }

            if (name == "origin[0]") { origin.x = static_cast<int32_t>(value); return true; }
            if (name == "origin[1]") { origin.y = static_cast<int32_t>(value); return true; }
            if (name == "origin[2]") { origin.z = static_cast<int32_t>(value); return true; }

            if (name == "velocity[0]") { velocity.x = static_cast<int32_t>(value); return true; }
            if (name == "velocity[1]") { velocity.y = static_cast<int32_t>(value); return true; }
            if (name == "velocity[2]") { velocity.z = static_cast<int32_t>(value); return true; }

            if (name == "viewangles[0]") { viewangles.x = static_cast<int32_t>(value); return true; }
            if (name == "viewangles[1]") { viewangles.y = static_cast<int32_t>(value); return true; }
            if (name == "viewangles[2]") { viewangles.z = static_cast<int32_t>(value); return true; }

            extras[name] = value;
            return true;
        }
    };

    // ---- JSON (nlohmann) ----
    inline void to_json(nlohmann::json& j, const PlayerState& ps) {
        j = nlohmann::json{
            {"commandTime", ps.commandTime},
            {"pm_type", ps.pm_type},
            {"weapon", ps.weapon},
            {"groundEntityNum", ps.groundEntityNum},
            {"legsAnim", ps.legsAnim},
            {"torsoAnim", ps.torsoAnim},
            {"eFlags", ps.eFlags},
            {"externalEvent", ps.externalEvent},
            {"clientNum", ps.clientNum},
            {"ping", ps.ping},
            {"origin", {ps.origin.x, ps.origin.y, ps.origin.z}},
            {"velocity", {ps.velocity.x, ps.velocity.y, ps.velocity.z}},
            {"viewangles", {ps.viewangles.x, ps.viewangles.y, ps.viewangles.z}},
            {"stats", ps.stats},
            {"persistant", ps.persistant},
            {"ammo", ps.ammo},
            {"powerups", ps.powerups},
            {"extras", ps.extras}
        };
    }

    inline void from_json(const nlohmann::json& j, PlayerState& ps) {
        j.at("commandTime").get_to(ps.commandTime);
        j.at("pm_type").get_to(ps.pm_type);
        j.at("weapon").get_to(ps.weapon);
        j.at("groundEntityNum").get_to(ps.groundEntityNum);
        j.at("legsAnim").get_to(ps.legsAnim);
        j.at("torsoAnim").get_to(ps.torsoAnim);
        j.at("eFlags").get_to(ps.eFlags);
        j.at("externalEvent").get_to(ps.externalEvent);
        j.at("clientNum").get_to(ps.clientNum);
        j.at("ping").get_to(ps.ping);

        auto o = j.at("origin");   ps.origin = {o[0], o[1], o[2]};
        auto v = j.at("velocity"); ps.velocity = {v[0], v[1], v[2]};
        auto a = j.at("viewangles"); ps.viewangles = {a[0], a[1], a[2]};

        j.at("stats").get_to(ps.stats);
        j.at("persistant").get_to(ps.persistant);
        j.at("ammo").get_to(ps.ammo);
        j.at("powerups").get_to(ps.powerups);

        if (j.contains("extras")) j.at("extras").get_to(ps.extras);
    }
// ---- Factory : reconstruction à partir des netfields ----
    static PlayerState makeFromNetfieldPairs(const std::vector<std::pair<std::string,int64_t>>& pairs) {
        PlayerState ps;
        for (const auto& [name, value] : pairs) {
            ps.setByNetfieldName(name, value);
        }
        return ps;
    }
} // namespace jka

namespace std {
    template<>
    struct hash<DemoJKA::PlayerState> {
        std::size_t operator()(const DemoJKA::PlayerState& ps) const noexcept {
            std::size_t h = std::hash<int>{}(ps.clientNum);
            h ^= std::hash<jka::Vec3>{}(ps.origin);
            h ^= std::hash<jka::Vec3>{}(ps.velocity);
            h ^= std::hash<jka::Trajectory>{}(ps.viewAngles);
            return h;
        }
    };
}

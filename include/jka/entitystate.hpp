#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>

#include "Vec3.hpp"              // Vec3T<T>, Vec3, Vec3i
#include "trajectory.hpp"        // Trajectory moderne
#include "state.hpp"               // Définitions communes (legacy)

namespace jka {

    /// État d’une entité dans un snapshot (équivalent entityState_t en C)
    struct EntityState {
        int number{0};       ///< Numéro unique de l’entité (slot)
        int eType{0};        ///< Type (ET_PLAYER, ET_ITEM, etc.)
        int eFlags{0};       ///< Flags divers

        Trajectory pos;      ///< Trajectoire de la position
        Trajectory apos;     ///< Trajectoire de l’orientation

        Vec3i origin;        ///< Origine compressée (réseau, entiers)
        Vec3i origin2;       ///< Origine secondaire (ex. attach point)
        Vec3i angles;        ///< Angles compressés (réseau, entiers)
        Vec3i angles2;       ///< Angles secondaires

        int time{0};
        int time2{0};

        int otherEntityNum{0};
        int otherEntityNum2{0};

        int groundEntityNum{0};
        int loopSound{0};
        int constantLight{0};

        int modelindex{0};
        int modelindex2{0};
        int clientNum{0};
        int frame{0};

        int solid{0};
        int event{0};
        int eventParm{0};
        int powerups{0};
        int weapon{0};
        int legsAnim{0};
        int torsoAnim{0};

        int generic1{0};

        // ---- Helpers de conversion réseau/offline -----------------------------

        /// Retourne origin en flottant (offline/debug)
        inline Vec3 getOriginF() const noexcept { return origin.toFloat(); }
        inline Vec3 getOrigin2F() const noexcept { return origin2.toFloat(); }

        /// Retourne angles en flottant (offline/debug)
        inline Vec3 getAnglesF() const noexcept { return angles.toFloat(); }
        inline Vec3 getAngles2F() const noexcept { return angles2.toFloat(); }

        // ---- Conversion JSON (debug/export) -----------------------------------
        friend void to_json(nlohmann::json& j, const EntityState& s) {
            j = nlohmann::json{
                {"number", s.number},
                {"eType", s.eType},
                {"eFlags", s.eFlags},
                {"pos", s.pos},
                {"apos", s.apos},
                {"origin", s.origin},
                {"origin2", s.origin2},
                {"angles", s.angles},
                {"angles2", s.angles2},
                {"time", s.time},
                {"time2", s.time2},
                {"otherEntityNum", s.otherEntityNum},
                {"otherEntityNum2", s.otherEntityNum2},
                {"groundEntityNum", s.groundEntityNum},
                {"loopSound", s.loopSound},
                {"constantLight", s.constantLight},
                {"modelindex", s.modelindex},
                {"modelindex2", s.modelindex2},
                {"clientNum", s.clientNum},
                {"frame", s.frame},
                {"solid", s.solid},
                {"event", s.event},
                {"eventParm", s.eventParm},
                {"powerups", s.powerups},
                {"weapon", s.weapon},
                {"legsAnim", s.legsAnim},
                {"torsoAnim", s.torsoAnim},
                {"generic1", s.generic1}
            };
        }

        friend void from_json(const nlohmann::json& j, EntityState& s) {
            j.at("number").get_to(s.number);
            j.at("eType").get_to(s.eType);
            j.at("eFlags").get_to(s.eFlags);
            j.at("pos").get_to(s.pos);
            j.at("apos").get_to(s.apos);
            j.at("origin").get_to(s.origin);
            j.at("origin2").get_to(s.origin2);
            j.at("angles").get_to(s.angles);
            j.at("angles2").get_to(s.angles2);
            j.at("time").get_to(s.time);
            j.at("time2").get_to(s.time2);
            j.at("otherEntityNum").get_to(s.otherEntityNum);
            j.at("otherEntityNum2").get_to(s.otherEntityNum2);
            j.at("groundEntityNum").get_to(s.groundEntityNum);
            j.at("loopSound").get_to(s.loopSound);
            j.at("constantLight").get_to(s.constantLight);
            j.at("modelindex").get_to(s.modelindex);
            j.at("modelindex2").get_to(s.modelindex2);
            j.at("clientNum").get_to(s.clientNum);
            j.at("frame").get_to(s.frame);
            j.at("solid").get_to(s.solid);
            j.at("event").get_to(s.event);
            j.at("eventParm").get_to(s.eventParm);
            j.at("powerups").get_to(s.powerups);
            j.at("weapon").get_to(s.weapon);
            j.at("legsAnim").get_to(s.legsAnim);
            j.at("torsoAnim").get_to(s.torsoAnim);
            j.at("generic1").get_to(s.generic1);
        }

        // ---- Parsing depuis netfields (à compléter avec ton parser) ------------
        template <typename NetfieldMap>
        static EntityState makeFromNetfieldPairs(const NetfieldMap& fields) {
            EntityState s;
            // Exemple : à remplir depuis le parser réseau DM_26
            // if (auto it = fields.find("origin"); it != fields.end()) {
            //     s.origin = Vec3i::fromArray(it->second);
            // }
            return s;
        }
    };

} // namespace jka
namespace std {
    template<>
    struct hash<jka::EntityState> {
        std::size_t operator()(const DemoJKA::EntityState& es) const noexcept {
            std::size_t h = std::hash<int>{}(es.number);
            h ^= std::hash<jka::Vec3i>{}(es.origin) + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
            h ^= std::hash<jka::Vec3>{}(es.angles)  + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
            h ^= std::hash<jka::Trajectory>{}(es.pos);
            // tu peux rajouter d’autres champs si besoin (event, modelindex, etc.)
            return h;
        }
    };
}
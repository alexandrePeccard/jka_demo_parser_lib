/*#pragma once
// entitystate.hpp — Modern C++ representation of Quake3/OpenJK entityState_t
// - Strong typing (scoped enums), helpers (Vec3, Trajectory)
// - Exact field parity with legacy entityState_t for DM_26 demos
// - JSON (de)serialization using nlohmann::json
//
// This header is self-contained: no dependency on q_shared.h.
// If you already expose engine enums elsewhere, you can:
//   - map raw ints to your enums in adapters, OR
//   - specialize to_json/from_json for your own types.

#include <array>
#include <cstdint>
#include <type_traits>
#include <nlohmann/json.hpp>

namespace jka {

// ---------- Small math helpers ----------

struct Vec3 {
    float x{0}, y{0}, z{0};

    constexpr Vec3() = default;
    constexpr Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    std::array<float,3> as_array() const { return {x,y,z}; }
    static Vec3 from_array(const std::array<float,3>& a) { return {a[0], a[1], a[2]}; }
};

// JSON for Vec3: as an array [x, y, z]
inline void to_json(nlohmann::json& j, const Vec3& v) {
    j = nlohmann::json::array({v.x, v.y, v.z});
}
inline void from_json(const nlohmann::json& j, Vec3& v) {
    if (j.is_array() && j.size() == 3) {
        v.x = j[0].get<float>();
        v.y = j[1].get<float>();
        v.z = j[2].get<float>();
    } else {
        // Also accept object style { "x":.., "y":.., "z":.. }
        v.x = j.at("x").get<float>();
        v.y = j.at("y").get<float>();
        v.z = j.at("z").get<float>();
    }
}

// ---------- Trajectory (network interpolation container) ----------

enum class TrajectoryType : std::int32_t {
    STATIONARY   = 0,  // TR_STATIONARY
    INTERPOLATE  = 1,  // TR_INTERPOLATE
    LINEAR       = 2,  // TR_LINEAR
    SINE         = 3,  // TR_SINE
    LINEAR_STOP  = 4,  // TR_LINEAR_STOP
    GRAVITY      = 5,  // TR_GRAVITY
    // OpenJK/JA may add variants — raw integer is preserved separately if needed
};

struct Trajectory {
    TrajectoryType trType{TrajectoryType::STATIONARY};
    Vec3           trBase{};      // start position/orientation
    Vec3           trDelta{};     // velocity/angular velocity
    std::int32_t   trTime{0};     // start time (serverTime)
    std::int32_t   trDuration{0}; // duration (ms)
};

// JSON for Trajectory
inline void to_json(nlohmann::json& j, const Trajectory& t) {
    j = nlohmann::json{
        {"type",      static_cast<std::int32_t>(t.trType)},
        {"base",      t.trBase},
        {"delta",     t.trDelta},
        {"time",      t.trTime},
        {"duration",  t.trDuration}
    };
}
inline void from_json(const nlohmann::json& j, Trajectory& t) {
    t.trType    = static_cast<TrajectoryType>( j.at("type").get<std::int32_t>() );
    t.trBase    = j.at("base").get<Vec3>();
    t.trDelta   = j.at("delta").get<Vec3>();
    t.trTime    = j.at("time").get<std::int32_t>();
    t.trDuration= j.at("duration").get<std::int32_t>();
}

// ---------- Entity type & flags (scoped/bitmask) ----------

// NOTE: Values below follow Quake3/OpenJK conventions where known.
// Keep them minimal & non-breaking. Unknown/custom values are still supported
// because we store the raw integer for eType/eFlags.
enum class EntityType : std::int32_t {
    GENERAL  = 0,  // ET_GENERAL
    PLAYER   = 1,  // ET_PLAYER
    ITEM     = 2,  // ET_ITEM
    MISSILE  = 3,  // ET_MISSILE
    MOVER    = 4,  // ET_MOVER (plats/doors)
    BEAM     = 5,  // ET_BEAM / misc effect
    PORTAL   = 6,  // ET_PORTAL
    SENTRY   = 7,  // example extra (mods vary)
    // ... extend if you rely on specific engine constants
};

// Bitmask flags used in eFlags. Keep as strongly-typed enum:
enum class EFlags : std::uint32_t {
    NONE            = 0,
    TELEPORT_BIT    = 1u << 0, // EF_TELEPORT_BIT (example)
    MISSILE_STICK   = 1u << 1, // EF_MISSILE_STICK (mods)
    KAMIKAZE        = 1u << 2, // EF_KAMIKAZE
    // ... add flags you actually use; raw storage remains available
};

// Enable bitwise ops for EFlags
constexpr EFlags operator|(EFlags a, EFlags b) {
    return static_cast<EFlags>( static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b) );
}
constexpr EFlags operator&(EFlags a, EFlags b) {
    return static_cast<EFlags>( static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b) );
}
constexpr EFlags& operator|=(EFlags& a, EFlags b) { a = a | b; return a; }
constexpr EFlags& operator&=(EFlags& a, EFlags b) { a = a & b; return a; }
constexpr bool any(EFlags f) { return static_cast<std::uint32_t>(f) != 0; }

// JSON for EFlags as integer
inline void to_json(nlohmann::json& j, const EFlags& f) {
    j = static_cast<std::uint32_t>(f);
}
inline void from_json(const nlohmann::json& j, EFlags& f) {
    f = static_cast<EFlags>( j.get<std::uint32_t>() );
}

// ---------- Modern entityState (1:1 with legacy entityState_t) ----------

struct EntityState {
    // Core identity
    std::int32_t number{0};                 // entity index
    std::int32_t eTypeRaw{0};               // raw engine value
    EntityType   eType{EntityType::GENERAL};

    // Flags (both raw + typed)
    std::uint32_t eFlagsRaw{0};
    EFlags        eFlags{EFlags::NONE};

    // Interpolated motion
    Trajectory pos{};                        // trajectory for position
    Trajectory apos{};                       // trajectory for angles

    // Misc timings
    std::int32_t time{0};
    std::int32_t time2{0};

    // Direct positions/orientations (snapshots)
    Vec3 origin{};   // mirrors legacy vec3_t origin
    Vec3 origin2{};
    Vec3 angles{};
    Vec3 angles2{};

    // Entity links
    std::int32_t otherEntityNum{0};
    std::int32_t otherEntityNum2{0};
    std::int32_t groundEntityNum{0};

    // Sounds/lights/models
    std::int32_t loopSound{0};
    std::int32_t constantLight{0};          // packed RGB+intensity (legacy)
    std::int32_t modelindex{0};
    std::int32_t modelindex2{0};
    std::int32_t modelindex3{0};
    std::int32_t modelindex4{0};
    std::int32_t frame{0};

    // Collision/solid (encoded bbox in legacy)
    std::int32_t solid{0};

    // Events/gameplay
    std::int32_t event_{0};                 // 'event' is keyword in some toolchains
    std::int32_t eventParm{0};
    std::int32_t powerups{0};
    std::int32_t weapon{0};
    std::int32_t legsAnim{0};
    std::int32_t torsoAnim{0};
    std::int32_t generic1{0};

    // JK specific (e.g., OpenJK/JA): 4 style channels
    std::array<std::int32_t,4> constantLightStyles{ {0,0,0,0} };

    // --- helpers ---
    // pack/unpack for historic 'solid' encoding (bbox): optional, provided as placeholders.
    // Many demos rely on legacy 'solid' bitfield for client prediction.
    static constexpr std::int32_t solidEncode(std::int32_t x, std::int32_t yz, std::int32_t z) {
        // NOTE: This encoding varies by game; keep simple passthrough or implement your engine’s spec.
        // Placeholder (identity):
        (void)yz; (void)z;
        return x;
    }
};

// JSON for EntityState
inline void to_json(nlohmann::json& j, const EntityState& e) {
    j = nlohmann::json{
        {"number",            e.number},
        {"eTypeRaw",          e.eTypeRaw},
        {"eType",             static_cast<std::int32_t>(e.eType)},
        {"eFlagsRaw",         e.eFlagsRaw},
        {"eFlags",            e.eFlags},

        {"pos",               e.pos},
        {"apos",              e.apos},

        {"time",              e.time},
        {"time2",             e.time2},

        {"origin",            e.origin},
        {"origin2",           e.origin2},
        {"angles",            e.angles},
        {"angles2",           e.angles2},

        {"otherEntityNum",    e.otherEntityNum},
        {"otherEntityNum2",   e.otherEntityNum2},
        {"groundEntityNum",   e.groundEntityNum},

        {"loopSound",         e.loopSound},
        {"constantLight",     e.constantLight},
        {"modelindex",        e.modelindex},
        {"modelindex2",       e.modelindex2},
        {"modelindex3",       e.modelindex3},
        {"modelindex4",       e.modelindex4},
        {"frame",             e.frame},
        {"solid",             e.solid},

        {"event",             e.event_},
        {"eventParm",         e.eventParm},
        {"powerups",          e.powerups},
        {"weapon",            e.weapon},
        {"legsAnim",          e.legsAnim},
        {"torsoAnim",         e.torsoAnim},
        {"generic1",          e.generic1},

        {"constantLightStyles", e.constantLightStyles}
    };
}

inline void from_json(const nlohmann::json& j, EntityState& e) {
    e.number          = j.at("number").get<std::int32_t>();
    e.eTypeRaw        = j.at("eTypeRaw").get<std::int32_t>();
    e.eType           = static_cast<EntityType>( j.at("eType").get<std::int32_t>() );
    e.eFlagsRaw       = j.at("eFlagsRaw").get<std::uint32_t>();
    e.eFlags          = j.at("eFlags").get<EFlags>();

    e.pos             = j.at("pos").get<Trajectory>();
    e.apos            = j.at("apos").get<Trajectory>();

    e.time            = j.at("time").get<std::int32_t>();
    e.time2           = j.at("time2").get<std::int32_t>();

    e.origin          = j.at("origin").get<Vec3>();
    e.origin2         = j.at("origin2").get<Vec3>();
    e.angles          = j.at("angles").get<Vec3>();
    e.angles2         = j.at("angles2").get<Vec3>();

    e.otherEntityNum  = j.at("otherEntityNum").get<std::int32_t>();
    e.otherEntityNum2 = j.at("otherEntityNum2").get<std::int32_t>();
    e.groundEntityNum = j.at("groundEntityNum").get<std::int32_t>();

    e.loopSound       = j.at("loopSound").get<std::int32_t>();
    e.constantLight   = j.at("constantLight").get<std::int32_t>();
    e.modelindex      = j.at("modelindex").get<std::int32_t>();
    e.modelindex2     = j.at("modelindex2").get<std::int32_t>();
    e.modelindex3     = j.at("modelindex3").get<std::int32_t>();
    e.modelindex4     = j.at("modelindex4").get<std::int32_t>();
    e.frame           = j.at("frame").get<std::int32_t>();
    e.solid           = j.at("solid").get<std::int32_t>();

    e.event_          = j.at("event").get<std::int32_t>();
    e.eventParm       = j.at("eventParm").get<std::int32_t>();
    e.powerups        = j.at("powerups").get<std::int32_t>();
    e.weapon          = j.at("weapon").get<std::int32_t>();
    e.legsAnim        = j.at("legsAnim").get<std::int32_t>();
    e.torsoAnim       = j.at("torsoAnim").get<std::int32_t>();
    e.generic1        = j.at("generic1").get<std::int32_t>();

    if (auto it = j.find("constantLightStyles"); it != j.end()) {
        e.constantLightStyles = it->get<std::array<std::int32_t,4>>();
    }
}

// ---------- Optional: compact debug helpers ----------

inline const char* to_cstr(EntityType t) {
    switch (t) {
        case EntityType::GENERAL: return "GENERAL";
        case EntityType::PLAYER:  return "PLAYER";
        case EntityType::ITEM:    return "ITEM";
        case EntityType::MISSILE: return "MISSILE";
        case EntityType::MOVER:   return "MOVER";
        case EntityType::BEAM:    return "BEAM";
        case EntityType::PORTAL:  return "PORTAL";
        case EntityType::SENTRY:  return "SENTRY";
        default:                  return "UNKNOWN";
    }
}

} // namespace jka
*/


#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>

#include "Vec3.hpp"              // Vec3T<T>, Vec3, Vec3i
#include "trajectory.hpp"        // Trajectory moderne
#include "state.h"               // Définitions communes (legacy)

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
    struct hash<DemoJKA::EntityState> {
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


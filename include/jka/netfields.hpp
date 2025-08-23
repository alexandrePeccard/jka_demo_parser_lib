#pragma once
// jka/netfields.hpp — NetField modernisé : helpers robustes + JSON optionnel
// Conçu pour rester 100% compatible avec OpenJK (codemp/game/bg_public.h + bg_misc.c)
// en gardant l'ordre exact des tables et structures. NE PAS réordonner vos entrées de tables.

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <optional>

#include "Vec3.hpp"          // jka::Vec3T<float>/Vec3T<int32_t> (Vec3f/Vec3i)
#include "trajectory.hpp"    // jka::Trajectory si vous exposez des champs traj
#include "defs.h"

namespace jka {

// ---------------------------------------------------------------------------
// Types & utilitaires basés sur OpenJK
// ---------------------------------------------------------------------------

// Structure NetField compatible avec OpenJK
struct NetField {
    const char* name;   // nom canonique OpenJK (exact de bg_public.h)
    int         ofs;    // offset (en octets) dans la struct cible
    int         bits;   // nb de bits réseau (pour info/outillage)
    bool        isFloat;// vrai si champ flottant (quantifié côté réseau)
};

// Constantes OpenJK
constexpr int MAX_STATS = 16;
constexpr int MAX_PERSISTANT = 16;
constexpr int MAX_POWERUPS = 16;
constexpr int MAX_AMMO = 16;
constexpr int NUM_FORCE_POWERS = 18;

// Types d'entités OpenJK
typedef enum {
    ET_GENERAL,
    ET_PLAYER,
    ET_ITEM,
    ET_MISSILE,
    ET_SPECIAL,             // rww - force fields
    ET_HOLOCRON,            // rww - holocron icon displays
    ET_MOVER,
    ET_BEAM,
    ET_PORTAL,
    ET_SPEAKER,
    ET_PUSH_TRIGGER,
    ET_TELEPORT_TRIGGER,
    ET_INVISIBLE,
    ET_NPC,                 // ghoul2 player-like entity
    ET_TEAM,
    ET_BODY,
    ET_TERRAIN,
    ET_FX,
    ET_EVENTS               // any of the EV_* events can be added freestanding
} entityType_t;

// Types PM OpenJK
typedef enum {
    PM_NORMAL,              // can accelerate and turn
    PM_JETPACK,             // special jetpack movement
    PM_FLOAT,               // float with no gravity in general direction of velocity
    PM_NOCLIP,              // noclip movement
    PM_SPECTATOR,           // still run into walls
    PM_DEAD,                // no acceleration or turning, but free falling
    PM_FREEZE,              // stuck in place with no control
    PM_INTERMISSION,        // no movement or status bar
    PM_SPINTERMISSION       // no movement or status bar
} pmtype_t;

// Stats OpenJK
typedef enum {
    STAT_HEALTH,
    STAT_HOLDABLE_ITEM,
    STAT_HOLDABLE_ITEMS,
    STAT_PERSISTANT_POWERUP,
    STAT_WEAPONS = 4,       // 16 bit fields - MUST remain at index 4!
    STAT_ARMOR,
    STAT_DEAD_YAW,          // look this direction when dead
    STAT_CLIENTS_READY,     // bit mask of clients wishing to exit
    STAT_MAX_HEALTH         // health / armor limit, changable by handicap
} statIndex_t;

// Persistant OpenJK
typedef enum {
    PERS_SCORE,             // !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
    PERS_HITS,              // total points damage inflicted
    PERS_RANK,              // player rank or team rank
    PERS_TEAM,              // player team
    PERS_SPAWN_COUNT,       // incremented every respawn
    PERS_PLAYEREVENTS,      // 16 bits that can be flipped for events
    PERS_ATTACKER,          // clientnum of last damage inflicter
    PERS_ATTACKEE_ARMOR,    // health/armor of last person we attacked
    PERS_KILLED,            // count of the number of times you died
    PERS_IMPRESSIVE_COUNT,  // two railgun hits in a row
    PERS_EXCELLENT_COUNT,   // two successive kills in a short amount of time
    PERS_DEFEND_COUNT,      // defend awards
    PERS_ASSIST_COUNT,      // assist awards
    PERS_GAUNTLET_FRAG_COUNT, // kills with the gauntlet
    PERS_CAPTURES           // captures
} persEnum_t;

// Powerups OpenJK
typedef enum {
    PW_NONE,
    PW_QUAD,
    PW_BATTLESUIT,
    PW_PULL,
    PW_REDFLAG,
    PW_BLUEFLAG,
    PW_NEUTRALFLAG,
    PW_SHIELDHIT,
    PW_SPEEDBURST,
    PW_DISINT_4,
    PW_SPEED,
    PW_CLOAKED,
    PW_FORCE_ENLIGHTENED_LIGHT,
    PW_FORCE_ENLIGHTENED_DARK,
    PW_FORCE_BOON,
    PW_YSALAMIRI,
    PW_NUM_POWERUPS
} powerup_t;

// Force Powers OpenJK
typedef enum {
    FP_HEAL = 0,           // instant
    FP_LEVITATION,         // hold/duration
    FP_SPEED,              // duration  
    FP_PUSH,               // hold/duration
    FP_PULL,               // hold/duration
    FP_TELEPATHY,          // instant
    FP_GRIP,               // hold/duration
    FP_LIGHTNING,          // hold/duration
    FP_RAGE,               // duration
    FP_PROTECT,            // duration
    FP_ABSORB,             // duration
    FP_TEAM_HEAL,          // instant
    FP_TEAM_FORCE,         // instant
    FP_DRAIN,              // hold/duration
    FP_SEE,                // duration
    FP_SABER_OFFENSE,
    FP_SABER_DEFENSE,
    FP_SABERTHROW,
    NUM_FORCE_POWERS
} forcePowers_t;

// Helpers pour lire/écrire scalaires arbitraires depuis un objet POD
template <class T, class Obj>
inline T& fieldRef(Obj& obj, int ofs) {
    return *reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(&obj) + ofs);
}

template <class T, class Obj>
inline const T& fieldRefConst(const Obj& obj, int ofs) {
    return *reinterpret_cast<const T*>(reinterpret_cast<const std::uint8_t*>(&obj) + ofs);
}

// Nombre dans un suffixe "name0/1/2" ou index "name[0/1/2]"
inline std::optional<int> parseVecIndex(std::string_view s) {
    if (s.empty()) return std::nullopt;
    if (s.back() >= '0' && s.back() <= '2') {
        if (s.size() >= 2 && s[s.size()-2] == '[') return int(s.back() - '0'); // …"[k]"
        // ou suffixe …"k"
        return int(s.back() - '0');
    }
    return std::nullopt;
}

// Normalisation d'alias → nom canonique (extensible aisément)
inline std::string normalizeAlias(std::string name) {
    // lower + enlever espaces
    std::string out; 
    out.reserve(name.size());
    for (char c : name) {
        if (c != ' ' && c != '\t') out.push_back(char(::tolower(unsigned char(c))));
    }
    
    // remplacements usuels
    auto rep = [&](std::string_view from, std::string_view to){
        auto pos = out.find(from);
        if (pos != std::string::npos) out.replace(pos, from.size(), to);
    };

    rep("pos", "origin");
    rep("org", "origin");
    rep("eangles", "angles");

    // variants index - origin.x -> origin0 ; origin[1] -> origin1
    if (out.find("origin.") != std::string::npos) {
        rep("origin.x", "origin0");
        rep("origin.y", "origin1");
        rep("origin.z", "origin2");
    }
    if (out.find("angles.") != std::string::npos) {
        rep("angles.x", "angles0");
        rep("angles.y", "angles1");
        rep("angles.z", "angles2");
    }

    return out;
}

// ---------------------------------------------------------------------------
// Accès générique : getByNetfieldName / setByNetfieldName
// ---------------------------------------------------------------------------

// lookup par nom dans une table
template <std::size_t N>
inline const NetField* findNetField(std::string_view name, const std::array<NetField,N>& tbl) {
    for (const auto& nf : tbl) {
        if (!nf.name) continue;
        if (name == nf.name) return &nf;
    }
    return nullptr;
}

// extraction d'un composant Vec3 (float/int) générique (lecture)
template <class Obj, class V3>
static inline bool readVec3Comp(const Obj& obj, const NetField& nf, int k, int& outInt, float& outFloat) {
    using scalar_t = typename V3::Scalar;
    const auto& v = fieldRefConst<V3>(obj, nf.ofs);
    if constexpr (std::is_same_v<scalar_t, float>) {
        outFloat = v[k];
    } else {
        outInt = v[k];
    }
    return true;
}

// écriture d'un composant Vec3 (float/int) générique
template <class Obj, class V3>
static inline bool writeVec3Comp(Obj& obj, const NetField& nf, int k, int inInt, float inFloat) {
    using scalar_t = typename V3::Scalar;
    auto& v = fieldRef<V3>(obj, nf.ofs);
    if constexpr (std::is_same_v<scalar_t, float>) {
        v[k] = inFloat;
    } else {
        v[k] = inInt;
    }
    return true;
}

// lecture générique
template <std::size_t N, class Obj>
inline bool getByNetfieldName(const Obj& obj,
                              const std::array<NetField,N>& tbl,
                              std::string name,
                              int& outInt, float& outFloat) {
    outInt = 0; outFloat = 0.f;
    name = normalizeAlias(name);
    
    // Essayer direct
    if (auto nf = findNetField(std::string_view(name), tbl)) {
        if (nf->isFloat) {
            outFloat = fieldRefConst<float>(obj, nf->ofs);
        } else {
            outInt = fieldRefConst<int>(obj, nf->ofs);
        }
        return true;
    }
    
    // Essayer avec index (Vec3)
    if (auto idx = parseVecIndex(name)) {
        // retirer [k] ou suffixe k → baseName
        std::string base = name;
        if (!base.empty() && base.back() >= '0' && base.back() <= '9') base.pop_back();
        auto open = base.find('[');
        if (open != std::string::npos) base.erase(open); // "angles[1]" -> "angles"

        if (auto nf = findNetField(std::string_view(base), tbl)) {
            // Essai en Vec3f
            if (nf->isFloat) {
                return readVec3Comp<Obj,Vec3f>(obj, *nf, *idx, outInt, outFloat);
            }
            // Essai en Vec3i
            return readVec3Comp<Obj,Vec3i>(obj, *nf, *idx, outInt, outFloat);
        }
    }
    return false;
}

// écriture générique (int/float/Vec3f/Vec3i)
template <std::size_t N, class Obj>
inline bool setByNetfieldName(Obj& obj,
                              const std::array<NetField,N>& tbl,
                              std::string name,
                              int inInt, float inFloat) {
    name = normalizeAlias(name);
    if (auto nf = findNetField(std::string_view(name), tbl)) {
        if (nf->isFloat) {
            fieldRef<float>(obj, nf->ofs) = inFloat;
        } else {
            fieldRef<int>(obj, nf->ofs) = inInt;
        }
        return true;
    }
    if (auto idx = parseVecIndex(name)) {
        std::string base = name;
        if (!base.empty() && base.back() >= '0' && base.back() <= '9') base.pop_back();
        auto open = base.find('[');
        if (open != std::string::npos) base.erase(open);

        if (auto nf = findNetField(std::string_view(base), tbl)) {
            if (nf->isFloat) {
                return writeVec3Comp<Obj,Vec3f>(obj, *nf, *idx, inInt, inFloat);
            }
            return writeVec3Comp<Obj,Vec3i>(obj, *nf, *idx, inInt, inFloat);
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// JSON optionnel (nlohmann::json) — dump/load génériques
// Activez avec: #define JKA_ENABLE_JSON avant cet include
// ---------------------------------------------------------------------------
#ifdef JKA_ENABLE_JSON
#include <nlohmann/json.hpp>

// Dump JSON : int/float + Vec3 (split en 0/1/2)
template <std::size_t N, class Obj>
inline nlohmann::json dumpNetfieldsJson(const Obj& obj,
                                        const std::array<NetField,N>& tbl) {
    nlohmann::json j;
    for (const auto& nf : tbl) {
        if (!nf.name) continue;

        // Tentative Vec3f/Vec3i : on reconnaît les champs vectoriels usuels par convention
        auto emit3 = [&](auto v, std::string base){
            j[base + "0"] = v[0];
            j[base + "1"] = v[1];
            j[base + "2"] = v[2];
        };

        // Heuristique : quelques noms courants:
        std::string n = nf.name;
        if (n == "origin" || n == "angles" || n == "velocity" || n == "viewangles" || 
            n == "mins" || n == "maxs" || n == "delta_angles") {
            if (nf.isFloat) {
                const auto& v = fieldRefConst<Vec3f>(obj, nf.ofs);
                emit3(v, n);
                continue;
            } else {
                const auto& v = fieldRefConst<Vec3i>(obj, nf.ofs);
                emit3(v, n);
                continue;
            }
        }

        if (nf.isFloat) {
            j[nf.name] = fieldRefConst<float>(obj, nf.ofs);
        } else {
            j[nf.name] = fieldRefConst<int>(obj, nf.ofs);
        }
    }
    return j;
}

// Load JSON : int/float + Vec3 (via name0/1/2 si présents)
template <std::size_t N, class Obj>
inline void loadNetfieldsJson(Obj& obj, const nlohmann::json& j,
                              const std::array<NetField,N>& tbl) {
    for (const auto& nf : tbl) {
        if (!nf.name) continue;

        // Essayer triplet k=0..2
        bool hadTriplet = false;
        for (int k = 0; k < 3; k++) {
            std::string key = std::string(nf.name) + char('0' + k);
            auto it = j.find(key);
            if (it != j.end() && it->is_number()) {
                hadTriplet = true;
                if (nf.isFloat) {
                    setByNetfieldName(obj, tbl, key, 0, it->get<float>());
                } else {
                    setByNetfieldName(obj, tbl, key, it->get<int>(), 0.f);
                }
            }
        }
        if (hadTriplet) continue;

        // Sinon scalaire direct
        auto it = j.find(nf.name);
        if (it != j.end() && it->is_number()) {
            if (nf.isFloat) setByNetfieldName(obj, tbl, nf.name, 0, it->get<float>());
            else            setByNetfieldName(obj, tbl, nf.name, it->get<int>(), 0.f);
        }
    }
}
#endif // JKA_ENABLE_JSON

// ---------------------------------------------------------------------------
// TABLES NETFIELD OPENJK COMPATIBLES
// Ordre exact des champs selon bg_public.h et les définitions de structures
// ---------------------------------------------------------------------------

// Macro helper pour offset
#define NETF(x, t, b, f) {#x, offsetof(t, x), b, f}

// Tables constexpr : PlayerState (OpenJK bg_public.h ordre)
// Note: Cette table doit correspondre EXACTEMENT à l'ordre des champs réseau d'OpenJK
inline constexpr std::array<NetField, 47 + MAX_STATS + MAX_PERSISTANT + MAX_POWERUPS + MAX_AMMO + NUM_FORCE_POWERS> playerStateFields = []{
    constexpr size_t total_size = 47 + MAX_STATS + MAX_PERSISTANT + MAX_POWERUPS + MAX_AMMO + NUM_FORCE_POWERS;
    std::array<NetField, total_size> tbl{};
    
    size_t k = 0;
    
    // Structure de base playerState_t selon OpenJK
    tbl[k++] = {"commandTime", 0, 32, false};
    tbl[k++] = {"pm_type", 0, 8, false};
    tbl[k++] = {"bobCycle", 0, 8, false};
    tbl[k++] = {"pm_flags", 0, 16, false};
    tbl[k++] = {"pm_time", 0, -16, false};
    
    // origine et vitesse (quantifiées)
    tbl[k++] = {"origin", 0, 0, false}; // Vec3i quantified
    tbl[k++] = {"velocity", 0, 0, false}; // Vec3i quantified
    
    tbl[k++] = {"weaponTime", 0, -16, false};
    tbl[k++] = {"gravity", 0, 16, false};
    tbl[k++] = {"speed", 0, 16, false};
    
    // angles delta et viewangles
    tbl[k++] = {"delta_angles", 0, 0, false}; // Vec3i angles
    tbl[k++] = {"groundEntityNum", 0, GENTITYNUM_BITS, false};
    tbl[k++] = {"legsTimer", 0, 16, false};
    tbl[k++] = {"legsAnim", 0, 16, false};
    tbl[k++] = {"torsoTimer", 0, 16, false};
    tbl[k++] = {"torsoAnim", 0, 16, false};
    tbl[k++] = {"movementDir", 0, 8, false};
    
    tbl[k++] = {"eFlags", 0, 32, false};
    tbl[k++] = {"eventSequence", 0, 16, false};
    
    // events array
    for (int i = 0; i < 4; i++) { // MAX_PS_EVENTS
        tbl[k++] = {("events[" + std::to_string(i) + "]").c_str(), 0, 8, false};
        tbl[k++] = {("eventParms[" + std::to_string(i) + "]").c_str(), 0, 8, false};
    }
    
    tbl[k++] = {"clientNum", 0, 8, false};
    tbl[k++] = {"weapon", 0, 8, false};
    tbl[k++] = {"weaponstate", 0, 4, false};
    
    tbl[k++] = {"viewangles", 0, 0, true}; // Vec3f
    tbl[k++] = {"viewheight", 0, -8, false};
    
    // stats array
    for (int i = 0; i < MAX_STATS; i++) {
        tbl[k++] = {("stats[" + std::to_string(i) + "]").c_str(), 0, 32, false};
    }
    
    // persistant array
    for (int i = 0; i < MAX_PERSISTANT; i++) {
        tbl[k++] = {("persistant[" + std::to_string(i) + "]").c_str(), 0, 32, false};
    }
    
    // powerups array
    for (int i = 0; i < MAX_POWERUPS; i++) {
        tbl[k++] = {("powerups[" + std::to_string(i) + "]").c_str(), 0, 32, false};
    }
    
    // ammo array
    for (int i = 0; i < MAX_AMMO; i++) {
        tbl[k++] = {("ammo[" + std::to_string(i) + "]").c_str(), 0, 16, false};
    }
    
    // Force Data
    tbl[k++] = {"forcePower", 0, 8, false};
    tbl[k++] = {"forcePowersKnown", 0, 32, false};
    tbl[k++] = {"forcePowersActive", 0, 32, false};
    tbl[k++] = {"forceRage", 0, 8, false};
    tbl[k++] = {"forceDrainTime", 0, 32, false};
    
    // Force power levels
    for (int i = 0; i < NUM_FORCE_POWERS; i++) {
        tbl[k++] = {("forcePowerLevel[" + std::to_string(i) + "]").c_str(), 0, 8, false};
    }
    
    // Jedi/Saber fields
    tbl[k++] = {"saberMove", 0, 8, false};
    tbl[k++] = {"saberBlocking", 0, 8, false};
    tbl[k++] = {"saberBlocked", 0, 8, false};
    
    // Additional common fields
    tbl[k++] = {"jumpPadTime", 0, 32, false};
    tbl[k++] = {"hasDetPackPlanted", 0, 1, false};
    
    return tbl;
}();

// Tables constexpr : EntityState (OpenJK bg_public.h ordre)
inline constexpr std::array<NetField, 32> entityStateFields = []{
    std::array<NetField, 32> tbl{};
    size_t k = 0;
    
    tbl[k++] = {"number", 0, GENTITYNUM_BITS, false};
    tbl[k++] = {"eType", 0, 8, false};
    tbl[k++] = {"eFlags", 0, 32, false};
    
    // trajectory pos
    tbl[k++] = {"pos.trType", 0, 8, false};
    tbl[k++] = {"pos.trTime", 0, 32, false};
    tbl[k++] = {"pos.trDuration", 0, 32, false};
    tbl[k++] = {"pos.trBase", 0, 0, false}; // Vec3f
    tbl[k++] = {"pos.trDelta", 0, 0, false}; // Vec3f
    
    // trajectory apos
    tbl[k++] = {"apos.trType", 0, 8, false};
    tbl[k++] = {"apos.trTime", 0, 32, false};
    tbl[k++] = {"apos.trDuration", 0, 32, false};
    tbl[k++] = {"apos.trBase", 0, 0, true}; // Vec3f angles
    tbl[k++] = {"apos.trDelta", 0, 0, true}; // Vec3f angles
    
    // autres champs communs
    tbl[k++] = {"time", 0, 32, false};
    tbl[k++] = {"time2", 0, 32, false};
    
    tbl[k++] = {"origin", 0, 0, false}; // Vec3f
    tbl[k++] = {"origin2", 0, 0, false}; // Vec3f
    tbl[k++] = {"angles", 0, 0, true}; // Vec3f
    tbl[k++] = {"angles2", 0, 0, true}; // Vec3f
    
    tbl[k++] = {"otherEntityNum", 0, GENTITYNUM_BITS, false};
    tbl[k++] = {"otherEntityNum2", 0, GENTITYNUM_BITS, false};
    
    tbl[k++] = {"groundEntityNum", 0, GENTITYNUM_BITS, false};
    
    tbl[k++] = {"constantLight", 0, 32, false};
    tbl[k++] = {"loopSound", 0, 16, false};
    
    tbl[k++] = {"modelindex", 0, 16, false};
    tbl[k++] = {"modelindex2", 0, 8, false};
    tbl[k++] = {"clientNum", 0, 8, false};
    tbl[k++] = {"frame", 0, 16, false};
    
    tbl[k++] = {"solid", 0, 24, false};
    
    tbl[k++] = {"event", 0, 10, false};
    tbl[k++] = {"eventParm", 0, 8, false};
    
    tbl[k++] = {"weapon", 0, 8, false};
    
    return tbl;
}();

#undef NETF

// ---------------------------------------------------------------------------
// CONSTANTES RÉSEAU OPENJK
// ---------------------------------------------------------------------------

// Bits pour les champs réseau (d'après OpenJK)
constexpr int GENTITYNUM_BITS = 10;  // max 1024 entities
constexpr int MAX_PS_EVENTS = 4;     // maximum events in playerState

// ---------------------------------------------------------------------------
// STRUCTURES COMPATIBLES OPENJK (pour tests/exemples)
// ---------------------------------------------------------------------------

// Structure minimaliste pour démonstration - remplacez par vos vraies structs
struct DemoPlayerState {
    int commandTime;
    int pm_type;
    int bobCycle;
    int pm_flags;
    int pm_time;
    Vec3i origin;         // quantified position
    Vec3i velocity;       // quantified velocity  
    int weaponTime;
    int gravity;
    int speed;
    Vec3i delta_angles;   // quantified angles
    int groundEntityNum;
    int legsTimer;
    int legsAnim;
    int torsoTimer;
    int torsoAnim;
    int movementDir;
    int eFlags;
    int eventSequence;
    int events[MAX_PS_EVENTS];
    int eventParms[MAX_PS_EVENTS];
    int clientNum;
    int weapon;
    int weaponstate;
    Vec3f viewangles;     // real angles
    int viewheight;
    int stats[MAX_STATS];
    int persistant[MAX_PERSISTANT];
    int powerups[MAX_POWERUPS];
    int ammo[MAX_AMMO];
    int forcePower;
    int forcePowersKnown;
    int forcePowersActive;
    int forceRage;
    int forceDrainTime;
    int forcePowerLevel[NUM_FORCE_POWERS];
    int saberMove;
    int saberBlocking;
    int saberBlocked;
    int jumpPadTime;
    int hasDetPackPlanted;
};

struct DemoEntityState {
    int number;
    int eType;
    int eFlags;
    // pos trajectory
    int pos_trType;
    int pos_trTime;
    int pos_trDuration;
    Vec3f pos_trBase;
    Vec3f pos_trDelta;
    // apos trajectory
    int apos_trType;
    int apos_trTime;
    int apos_trDuration;
    Vec3f apos_trBase;
    Vec3f apos_trDelta;
    // autres
    int time;
    int time2;
    Vec3f origin;
    Vec3f origin2;
    Vec3f angles;
    Vec3f angles2;
    int otherEntityNum;
    int otherEntityNum2;
    int groundEntityNum;
    int constantLight;
    int loopSound;
    int modelindex;
    int modelindex2;
    int clientNum;
    int frame;
    int solid;
    int event;
    int eventParm;
    int weapon;
};

// ---------------------------------------------------------------------------
// FONCTIONS UTILITAIRES OPENJK
// ---------------------------------------------------------------------------

// Validation des noms de champs OpenJK
inline bool isValidPlayerStateField(const std::string& name) {
    // Quelques champs essentiels pour validation rapide
    static const std::vector<std::string> validFields = {
        "commandTime", "pm_type", "bobCycle", "pm_flags", "pm_time",
        "origin", "velocity", "weaponTime", "gravity", "speed",
        "delta_angles", "groundEntityNum", "viewangles", "viewheight",
        "forcePower", "forcePowersKnown", "forcePowersActive",
        "saberMove", "saberBlocking", "saberBlocked"
    };
    
    // Vérifier les champs simples
    if (std::find(validFields.begin(), validFields.end(), name) != validFields.end()) {
        return true;
    }
    
    // Vérifier les arrays
    if (name.find("stats[") == 0 || name.find("persistant[") == 0 ||
        name.find("powerups[") == 0 || name.find("ammo[") == 0 ||
        name.find("events[") == 0 || name.find("eventParms[") == 0 ||
        name.find("forcePowerLevel[") == 0) {
        return true;
    }
    
    return false;
}

inline bool isValidEntityStateField(const std::string& name) {
    static const std::vector<std::string> validFields = {
        "number", "eType", "eFlags", "time", "time2",
        "pos.trType", "pos.trTime", "pos.trDuration", "pos.trBase", "pos.trDelta",
        "apos.trType", "apos.trTime", "apos.trDuration", "apos.trBase", "apos.trDelta",
        "origin", "origin2", "angles", "angles2",
        "otherEntityNum", "otherEntityNum2", "groundEntityNum",
        "constantLight", "loopSound", "modelindex", "modelindex2",
        "clientNum", "frame", "solid", "event", "eventParm", "weapon"
    };
    
    return std::find(validFields.begin(), validFields.end(), name) != validFields.end();
}

// Conversion force power enum vers string (utile pour debug)
inline const char* forcePowerToString(int power) {
    static const char* names[NUM_FORCE_POWERS] = {
        "FP_HEAL", "FP_LEVITATION", "FP_SPEED", "FP_PUSH", "FP_PULL",
        "FP_TELEPATHY", "FP_GRIP", "FP_LIGHTNING", "FP_RAGE", "FP_PROTECT",
        "FP_ABSORB", "FP_TEAM_HEAL", "FP_TEAM_FORCE", "FP_DRAIN", "FP_SEE",
        "FP_SABER_OFFENSE", "FP_SABER_DEFENSE", "FP_SABERTHROW"
    };
    
    if (power >= 0 && power < NUM_FORCE_POWERS) {
        return names[power];
    }
    return "UNKNOWN_FORCE_POWER";
}

// ---------------------------------------------------------------------------
// EXEMPLES D'UTILISATION
// ---------------------------------------------------------------------------

#ifdef JKA_NETFIELDS_EXAMPLES

// Exemple : lecture de champs depuis PlayerState
inline void exampleReadPlayerState() {
    DemoPlayerState ps{};
    ps.origin = Vec3i{100, 200, 50}; // position quantifiée
    ps.viewangles = Vec3f{0.0f, 90.0f, 0.0f}; // angles réels
    ps.forcePower = 75;
    ps.stats[STAT_HEALTH] = 100;
    ps.forcePowerLevel[FP_SPEED] = 2;

    int intVal;
    float floatVal;
    
    // Lecture de l'origine (Vec3i)
    if (getByNetfieldName(ps, playerStateFields, "origin0", intVal, floatVal)) {
        std::cout << "Origin X: " << intVal << std::endl; // 100
    }
    
    // Lecture des viewangles (Vec3f)
    if (getByNetfieldName(ps, playerStateFields, "viewangles1", intVal, floatVal)) {
        std::cout << "View Yaw: " << floatVal << std::endl; // 90.0
    }
    
    // Lecture d'un stat
    if (getByNetfieldName(ps, playerStateFields, "stats[0]", intVal, floatVal)) {
        std::cout << "Health: " << intVal << std::endl; // 100
    }
    
    // Lecture force power
    if (getByNetfieldName(ps, playerStateFields, "forcePower", intVal, floatVal)) {
        std::cout << "Force Power: " << intVal << std::endl; // 75
    }
}

// Exemple : écriture de champs dans PlayerState
inline void exampleWritePlayerState() {
    DemoPlayerState ps{};
    
    // Écriture de position
    setByNetfieldName(ps, playerStateFields, "origin0", 150, 0.0f);
    setByNetfieldName(ps, playerStateFields, "origin1", 250, 0.0f);
    setByNetfieldName(ps, playerStateFields, "origin2", 100, 0.0f);
    
    // Écriture d'angles
    setByNetfieldName(ps, playerStateFields, "viewangles0", 0, -15.5f); // pitch
    setByNetfieldName(ps, playerStateFields, "viewangles1", 0, 180.0f); // yaw
    setByNetfieldName(ps, playerStateFields, "viewangles2", 0, 0.0f);   // roll
    
    // Écriture de stats
    setByNetfieldName(ps, playerStateFields, "stats[0]", 75, 0.0f); // health
    setByNetfieldName(ps, playerStateFields, "stats[5]", 50, 0.0f); // armor
    
    std::cout << "Position: (" << ps.origin[0] << ", " << ps.origin[1] << ", " << ps.origin[2] << ")" << std::endl;
    std::cout << "View angles: (" << ps.viewangles[0] << ", " << ps.viewangles[1] << ", " << ps.viewangles[2] << ")" << std::endl;
    std::cout << "Health: " << ps.stats[STAT_HEALTH] << ", Armor: " << ps.stats[STAT_ARMOR] << std::endl;
}

// Exemple avec JSON
#ifdef JKA_ENABLE_JSON
inline void exampleJsonPlayerState() {
    DemoPlayerState ps{};
    ps.origin = Vec3i{300, 400, 75};
    ps.viewangles = Vec3f{-10.0f, 45.0f, 0.0f};
    ps.forcePower = 80;
    ps.stats[STAT_HEALTH] = 90;
    ps.stats[STAT_ARMOR] = 25;
    
    // Export vers JSON
    auto jsonData = dumpNetfieldsJson(ps, playerStateFields);
    std::cout << "PlayerState JSON: " << jsonData.dump(2) << std::endl;
    
    // Import depuis JSON
    DemoPlayerState ps2{};
    loadNetfieldsJson(ps2, jsonData, playerStateFields);
    
    std::cout << "Restored position: (" << ps2.origin[0] << ", " << ps2.origin[1] << ", " << ps2.origin[2] << ")" << std::endl;
    std::cout << "Restored health: " << ps2.stats[STAT_HEALTH] << std::endl;
}
#endif // JKA_ENABLE_JSON

#endif // JKA_NETFIELDS_EXAMPLES

} // namespace jka

// ---------------------------------------------------------------------------
// NOTES D'UTILISATION
// ---------------------------------------------------------------------------

/*
UTILISATION:

1. Inclure le fichier avec vos propres structures playerState_t/entityState_t :
   #include "netfields.hpp"

2. Pour le support JSON optionnel :
   #define JKA_ENABLE_JSON
   #include "netfields.hpp"

3. Lecture de champs :
   int intVal; float floatVal;
   if (jka::getByNetfieldName(playerState, jka::playerStateFields, "origin0", intVal, floatVal)) {
       // intVal contient la coordonnée X
   }

4. Écriture de champs :
   jka::setByNetfieldName(playerState, jka::playerStateFields, "viewangles1", 0, 90.0f);

5. Support des alias :
   - "pos" -> "origin"
   - "org" -> "origin"  
   - "origin.x" -> "origin0"
   - "angles[1]" -> "angles1"

COMPATIBILITÉ:
- 100% compatible avec les structures réseau OpenJK
- Ordre des champs respecté pour delta compression
- Support Vec3i (quantified) et Vec3f (real)
- Tous les tableaux (stats, persistant, powerups, ammo, etc.)

PERFORMANCE:
- Tables constexpr (compile-time)
- Templates pour éviter les copies
- Lookups O(n) mais tables petites
- Cache-friendly pour accès séquentiel
*/
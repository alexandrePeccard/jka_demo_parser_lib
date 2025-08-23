#pragma once
// include/jka/entitystate.hpp — version moderne exhaustive (DM_26 / JKA)

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <type_traits>
#include <algorithm>
#include <cstring>

#include "defs.h"
#include "Vec3.hpp"
#include "trajectory.hpp"

DEMO_NAMESPACE_START
namespace jka {

// -----------------------------------------------------------------------------
// Utilities
// -----------------------------------------------------------------------------

namespace detail {

// Parse "name[idx]" or "name.x" / "name.0" → (baseName, optIndex)
struct ParsedName {
    std::string base;
    std::optional<int> index; // 0..2 for vec3 components
};

// very permissive: "origin[2]" | "origin.2" | "origin.z"
inline std::optional<ParsedName> parseIndexed(const std::string& s) {
    ParsedName out;
    out.base = s;
    out.index.reset();

    // bracket form: foo[1]
    if (auto lb = s.find('['); lb != std::string::npos) {
        auto rb = s.find(']', lb+1);
        if (rb != std::string::npos) {
            out.base = s.substr(0, lb);
            auto num = s.substr(lb+1, rb - (lb+1));
            try { out.index = std::stoi(num); } catch (...) { return std::nullopt; }
            return out;
        }
    }
    // dotted form: foo.1 / foo.x
    if (auto dot = s.rfind('.'); dot != std::string::npos) {
        auto base = s.substr(0, dot);
        auto comp = s.substr(dot+1);
        int idx = -1;
        if (comp == "x") idx = 0;
        else if (comp == "y") idx = 1;
        else if (comp == "z") idx = 2;
        else {
            try { idx = std::stoi(comp); } catch (...) { idx = -1; }
        }
        if (idx >= 0 && idx <= 2) {
            out.base = base;
            out.index = idx;
            return out;
        }
    }
    return out; // no index
}

inline bool ieq(const std::string& a, const std::string& b) {
    if (a.size()!=b.size()) return false;
    for (size_t i=0;i<a.size();++i) {
        char ca = (char)std::tolower((unsigned char)a[i]);
        char cb = (char)std::tolower((unsigned char)b[i]);
        if (ca!=cb) return false;
    }
    return true;
}

} // namespace detail

// -----------------------------------------------------------------------------
// EntityState (moderne)
// -----------------------------------------------------------------------------

struct EntityState {
    // ===== Identité / Class =====
    int32_t number{0};          ///< entity index
    int32_t eType{0};           ///< type (ET_*). Conservé en int pour compat protocole.
    int32_t eFlags{0};          ///< EF_* bits
    int32_t eFlags2{0};         ///< EF2_* (extension OpenJK fréquente)

    // ===== Modèle / Visuel =====
    int32_t modelindex{0};
    int32_t modelindex2{0};
    int32_t modelindex3{0};
    int32_t modelindex4{0};
    int32_t frame{0};           ///< frame d’anim modèle principal

    // ===== Physique (trajectoires + caches absolus) =====
    Trajectory pos;             ///< position (trBase/trDelta/trType/trTime/trDuration)
    Trajectory apos;            ///< angles (trajectoire angulaire)
    int32_t time{0};            ///< time marker
    int32_t time2{0};           ///< second marker
    Vec3f    origin{};          ///< cache absolu (souvent égal à pos.trBase échantillonné)
    Vec3f    origin2{};         ///< secondaire pour movers / spline
    Vec3f    angles{};          ///< yaw/pitch/roll (°)
    Vec3f    angles2{};         ///< secondaire (movers/doors)

    // ===== Collision / Appui =====
    int32_t groundEntityNum{-1};
    int32_t solid{0};           ///< SOLID_ENCODE

    // ===== Lumière / Son =====
    int32_t constantLight{0};   ///< packed r/g/b/radius (Q3)
    int32_t loopSound{0};       ///< SND index
    int32_t soundSetIndex{0};   ///< OpenJK: soundset (NPC/ent) si présent

    // ===== Ownership / Liaisons =====
    int32_t otherEntityNum{-1};
    int32_t otherEntityNum2{-1};
    int32_t clientNum{-1};      ///< ex. owner client / driver etc.

    // ===== Gameplay / Items / Arme / Anim =====
    int32_t powerups{0};        ///< bitfield (rare dans entState, mais présent côté protocol)
    int32_t weapon{0};          ///< current weapon index (missiles, players, items)
    int32_t generic1{0};        ///< usage libre (team, etc. suivant type)
    int32_t legsAnim{0};
    int32_t torsoAnim{0};

    // ===== Evénements =====
    int32_t event_{0};          ///< nom évite clash macro Windows
    int32_t eventParm{0};

    // ===== Extensions JKA courantes (non exhaustives) =======================
    // Champs observés dans OpenJK/MP pour certains ET_* (NPC, missiles, movers):
    int32_t iModelScale{0};     ///< échelle 0..255 (ou fixe) – scaler visuel
    int32_t surfacesOn{0};      ///< surface bits (render on)
    int32_t surfacesOff{0};     ///< surface bits (render off)
    int32_t ragAttach{0};       ///< ragdoll attach flags/id
    int32_t boltToPlayer{0};    ///< bolt to player (bool/index)
    int32_t bolt1{0};           ///< bolt index (attachment)
    int32_t bolt2{0};
    int32_t heldByClient{-1};   ///< ent tenu par un client ?
    int32_t isJediMaster{0};    ///< game mode flag
    int32_t isPortalEnt{0};     ///< portal surface ent
    int32_t NPC_class{0};       ///< classe NPC (si applicable)

    // Réservoir d’extensions non mappées (pour ne rien perdre)
    std::unordered_map<std::string, int32_t> extrasInt;
    std::unordered_map<std::string, float>   extrasFloat;

    // -------------------------------------------------------------------------
    // Helpers comparaisons / utils
    // -------------------------------------------------------------------------
    friend bool operator==(const EntityState& a, const EntityState& b) {
        return std::memcmp(&a.number, &b.number, sizeof(EntityState))==0; // POD-friendly (nous n’avons que des PODs & containers vides)
    }
    friend bool operator!=(const EntityState& a, const EntityState& b) { return !(a==b); }

    // Donne une idée d’un « hash » simple (pour debug/sets) — volontairement stable
    std::size_t simpleHash() const noexcept {
        auto hmix = [](std::size_t h, std::size_t v){ return h ^ (v + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2)); };
        std::size_t h = std::hash<int32_t>{}(number);
        h = hmix(h, std::hash<int32_t>{}(eType));
        h = hmix(h, std::hash<int32_t>{}(eFlags));
        h = hmix(h, std::hash<float>{}(origin.x));
        h = hmix(h, std::hash<float>{}(origin.y));
        h = hmix(h, std::hash<float>{}(origin.z));
        h = hmix(h, std::hash<int32_t>{}(modelindex));
        h = hmix(h, std::hash<int32_t>{}(event_));
        return h;
    }

    // -------------------------------------------------------------------------
    // NetField name-based setters/getters (robustes)
    // -------------------------------------------------------------------------

    // Défauts: si le nom n’est pas reconnu → stockage dans extrasInt/extrasFloat
    bool setByNetfieldName(const std::string& name, int32_t v) {
        using namespace detail;
        auto parsedOpt = parseIndexed(name);
        if (!parsedOpt) { extrasInt[name]=v; return false; }
        const auto& p = *parsedOpt;
        const auto& base = p.base;

        // ---- entiers simples connus
        auto setIntIf = [&](const char* n, int32_t& dst)->bool { if (ieq(base,n)) { dst = v; return true; } return false; };

        if ( setIntIf("number", number) ||
             setIntIf("eType", eType)   ||
             setIntIf("eFlags", eFlags) ||
             setIntIf("eFlags2", eFlags2) ||
             setIntIf("modelindex", modelindex) ||
             setIntIf("modelindex2", modelindex2) ||
             setIntIf("modelindex3", modelindex3) ||
             setIntIf("modelindex4", modelindex4) ||
             setIntIf("frame", frame) ||
             setIntIf("time", time) ||
             setIntIf("time2", time2) ||
             setIntIf("groundEntityNum", groundEntityNum) ||
             setIntIf("solid", solid) ||
             setIntIf("constantLight", constantLight) ||
             setIntIf("loopSound", loopSound) ||
             setIntIf("soundSetIndex", soundSetIndex) ||
             setIntIf("otherEntityNum", otherEntityNum) ||
             setIntIf("otherEntityNum2", otherEntityNum2) ||
             setIntIf("clientNum", clientNum) ||
             setIntIf("powerups", powerups) ||
             setIntIf("weapon", weapon) ||
             setIntIf("generic1", generic1) ||
             setIntIf("legsAnim", legsAnim) ||
             setIntIf("torsoAnim", torsoAnim) ||
             setIntIf("event", event_) ||
             setIntIf("eventParm", eventParm) ||
             setIntIf("iModelScale", iModelScale) ||
             setIntIf("surfacesOn", surfacesOn) ||
             setIntIf("surfacesOff", surfacesOff) ||
             setIntIf("ragAttach", ragAttach) ||
             setIntIf("boltToPlayer", boltToPlayer) ||
             setIntIf("bolt1", bolt1) ||
             setIntIf("bolt2", bolt2) ||
             setIntIf("heldByClient", heldByClient) ||
             setIntIf("isJediMaster", isJediMaster) ||
             setIntIf("isPortalEnt", isPortalEnt) ||
             setIntIf("NPC_class", NPC_class)
           ) return true;

        // ---- vec3
        auto setVec3Comp = [&](const char* n, Vec3f& v3)->bool {
            if (!ieq(base,n)) return false;
            if (!p.index) return false;
            int idx = *p.index;
            if (idx<0 || idx>2) return false;
            v3[idx] = static_cast<float>(v);
            return true;
        };
        if ( setVec3Comp("origin", origin) ||
             setVec3Comp("origin2", origin2) ||
             setVec3Comp("angles", angles) ||
             setVec3Comp("angles2", angles2) )
            return true;

        // ---- Trajectory slots (int) — sous-champs
        //  pos.trType / trTime / trDuration ; apos.*
        auto setTrajInt = [&](const char* prefix, Trajectory& tr)->bool {
            if (ieq(base, std::string(prefix)+".trType")) { tr.type = toModernEnum(v); return true; }
            if (ieq(base, std::string(prefix)+".trTime")) { tr.startTime = v; return true; }
            if (ieq(base, std::string(prefix)+".trDuration")) { tr.duration = v; return true; }
            return false;
        };
        if ( setTrajInt("pos", pos) || setTrajInt("apos", apos) ) return true;

        //  pos.trBase[idx] / pos.trDelta[idx]
        auto setTrajVec = [&](const char* prefix, Vec3f Trajectory::* member)->bool {
            if (!p.index) return false;
            if (ieq(base, std::string(prefix)+".trBase"))  { (pos.*member)[*p.index] = static_cast<float>(v); return true; }
            if (ieq(base, std::string(prefix)+".trDelta")) { (pos.*member)[*p.index] = static_cast<float>(v); return true; }
            return false;
        };
        // handle both pos/apos, base/delta
        if (p.index) {
            if (ieq(base, "pos.trBase"))  { pos.base[ *p.index ]  = static_cast<float>(v); return true; }
            if (ieq(base, "pos.trDelta")) { pos.delta[*p.index ]  = static_cast<float>(v); return true; }
            if (ieq(base, "apos.trBase")) { apos.base[*p.index ]  = static_cast<float>(v); return true; }
            if (ieq(base, "apos.trDelta")){ apos.delta[*p.index ] = static_cast<float>(v); return true; }
        }

        // inconnu → extras
        extrasInt[name]=v;
        return false;
    }

    bool setByNetfieldName(const std::string& name, float f) {
        using namespace detail;
        auto parsedOpt = parseIndexed(name);
        if (!parsedOpt) { extrasFloat[name]=f; return false; }
        const auto& p = *parsedOpt;
        const auto& base = p.base;

        auto setVec3Comp = [&](const char* n, Vec3f& v3)->bool {
            if (!ieq(base,n)) return false;
            if (!p.index) return false;
            int idx = *p.index;
            if (idx<0 || idx>2) return false;
            v3[idx] = f;
            return true;
        };
        if ( setVec3Comp("origin", origin) ||
             setVec3Comp("origin2", origin2) ||
             setVec3Comp("angles", angles) ||
             setVec3Comp("angles2", angles2) )
            return true;

        // Traj base/delta
        if (p.index) {
            if (ieq(base, "pos.trBase"))   { pos.base[*p.index] = f; return true; }
            if (ieq(base, "pos.trDelta"))  { pos.delta[*p.index]= f; return true; }
            if (ieq(base, "apos.trBase"))  { apos.base[*p.index]= f; return true; }
            if (ieq(base, "apos.trDelta")) { apos.delta[*p.index]= f; return true; }
        }

        // Allow int slots tolerant conversion
        auto setIntAlias = [&](const char* n, int32_t& dst)->bool {
            if (detail::ieq(base,n)) { dst = static_cast<int32_t>(f); return true; }
            return false;
        };
        if ( setIntAlias("number", number) ||
             setIntAlias("eType", eType) || setIntAlias("eFlags", eFlags) ||
             setIntAlias("eFlags2", eFlags2) || setIntAlias("frame", frame) )
            return true;

        extrasFloat[name]=f;
        return false;
    }

    // returns (found?, asInt/asFloat)
    std::optional<int32_t>  getIntByNetfieldName(const std::string& name) const {
        using namespace detail;
        auto parsedOpt = parseIndexed(name);
        if (!parsedOpt) return std::nullopt;
        const auto& p = *parsedOpt; const auto& base = p.base;

        auto getIntIf = [&](const char* n, int32_t v)->std::optional<int32_t>{
            if (ieq(base,n)) return v; return std::nullopt;
        };

        if (auto r = getIntIf("number", number)) return r;
        if (auto r = getIntIf("eType", eType)) return r;
        if (auto r = getIntIf("eFlags", eFlags)) return r;
        if (auto r = getIntIf("eFlags2", eFlags2)) return r;
        if (auto r = getIntIf("modelindex", modelindex)) return r;
        if (auto r = getIntIf("modelindex2", modelindex2)) return r;
        if (auto r = getIntIf("modelindex3", modelindex3)) return r;
        if (auto r = getIntIf("modelindex4", modelindex4)) return r;
        if (auto r = getIntIf("frame", frame)) return r;
        if (auto r = getIntIf("time", time)) return r;
        if (auto r = getIntIf("time2", time2)) return r;
        if (auto r = getIntIf("groundEntityNum", groundEntityNum)) return r;
        if (auto r = getIntIf("solid", solid)) return r;
        if (auto r = getIntIf("constantLight", constantLight)) return r;
        if (auto r = getIntIf("loopSound", loopSound)) return r;
        if (auto r = getIntIf("soundSetIndex", soundSetIndex)) return r;
        if (auto r = getIntIf("otherEntityNum", otherEntityNum)) return r;
        if (auto r = getIntIf("otherEntityNum2", otherEntityNum2)) return r;
        if (auto r = getIntIf("clientNum", clientNum)) return r;
        if (auto r = getIntIf("powerups", powerups)) return r;
        if (auto r = getIntIf("weapon", weapon)) return r;
        if (auto r = getIntIf("generic1", generic1)) return r;
        if (auto r = getIntIf("legsAnim", legsAnim)) return r;
        if (auto r = getIntIf("torsoAnim", torsoAnim)) return r;
        if (auto r = getIntIf("event", event_)) return r;
        if (auto r = getIntIf("eventParm", eventParm)) return r;

        if (auto it = extrasInt.find(name); it != extrasInt.end()) return it->second;
        return std::nullopt;
    }

    std::optional<float> getFloatByNetfieldName(const std::string& name) const {
        using namespace detail;
        auto parsedOpt = parseIndexed(name);
        if (!parsedOpt) return std::nullopt;
        const auto& p = *parsedOpt; const auto& base = p.base;

        auto getVec3Comp = [&](const char* n, const Vec3f& v3)->std::optional<float>{
            if (!ieq(base,n) || !p.index) return std::nullopt;
            int idx = *p.index; if (idx<0||idx>2) return std::nullopt;
            return v3[idx];
        };
        if (auto r = getVec3Comp("origin", origin)) return r;
        if (auto r = getVec3Comp("origin2", origin2)) return r;
        if (auto r = getVec3Comp("angles", angles)) return r;
        if (auto r = getVec3Comp("angles2", angles2)) return r;

        // Traj components
        if (p.index) {
            if (ieq(base,"pos.trBase"))   return pos.base[*p.index];
            if (ieq(base,"pos.trDelta"))  return pos.delta[*p.index];
            if (ieq(base,"apos.trBase"))  return apos.base[*p.index];
            if (ieq(base,"apos.trDelta")) return apos.delta[*p.index];
        }

        if (auto it = extrasFloat.find(name); it != extrasFloat.end()) return it->second;
        return std::nullopt;
    }
};

// -----------------------------------------------------------------------------
// Adaptateurs ⇆ C (q_shared.h)
// -----------------------------------------------------------------------------

#ifdef JKA_HAVE_Q_SHARED_H
extern "C" {
  #include "q_shared.h" // doit fournir entityState_t, trajectory_t, vec3_t, trType_t
}

inline EntityState fromLegacy(const entityState_t& in) {
    EntityState o{};

    o.number = in.number;
    o.eType  = in.eType;
    o.eFlags = in.eFlags;

    o.pos      = fromLegacy(in.pos);
    o.apos     = fromLegacy(in.apos);
    o.time     = in.time;
    o.time2    = in.time2;
    o.origin   = { in.origin[0],  in.origin[1],  in.origin[2] };
    o.origin2  = { in.origin2[0], in.origin2[1], in.origin2[2] };
    o.angles   = { in.angles[0],  in.angles[1],  in.angles[2] };
    o.angles2  = { in.angles2[0], in.angles2[1], in.angles2[2] };

    o.groundEntityNum = in.groundEntityNum;
    o.solid           = in.solid;

    o.constantLight = in.constantLight;
    o.loopSound     = in.loopSound;

    o.modelindex  = in.modelindex;
    o.modelindex2 = in.modelindex2;
    // extensions courantes (si struct fournie par ton q_shared a ces champs)
    #ifdef HAVE_ENTITYSTATE_MODELINDEX34
    o.modelindex3 = in.modelindex3;
    o.modelindex4 = in.modelindex4;
    #endif

    o.otherEntityNum  = in.otherEntityNum;
    o.otherEntityNum2 = in.otherEntityNum2;
    o.clientNum       = in.clientNum;

    o.powerups  = in.powerups;
    o.weapon    = in.weapon;
    o.generic1  = in.generic1;
    o.legsAnim  = in.legsAnim;
    o.torsoAnim = in.torsoAnim;
    o.event_    = in.event;
    o.eventParm = in.eventParm;

    // extensions OpenJK si disponibles (guardés pour compat)
    #ifdef HAVE_ENTITYSTATE_EFLAGS2
    o.eFlags2 = in.eFlags2;
    #endif
    #ifdef HAVE_ENTITYSTATE_SOUNDSET
    o.soundSetIndex = in.soundSetIndex;
    #endif
    #ifdef HAVE_ENTITYSTATE_IMODELSCALE
    o.iModelScale = in.iModelScale;
    #endif
    #ifdef HAVE_ENTITYSTATE_SURFACESONOFF
    o.surfacesOn  = in.surfacesOn;
    o.surfacesOff = in.surfacesOff;
    #endif
    #ifdef HAVE_ENTITYSTATE_RAG_ATTACH
    o.ragAttach = in.ragAttach;
    #endif
    #ifdef HAVE_ENTITYSTATE_BOLT_TO_PLAYER
    o.boltToPlayer = in.boltToPlayer;
    #endif
    #ifdef HAVE_ENTITYSTATE_BOLTS
    o.bolt1 = in.bolt1; o.bolt2 = in.bolt2;
    #endif
    #ifdef HAVE_ENTITYSTATE_HELDBYCLIENT
    o.heldByClient = in.heldByClient;
    #endif
    #ifdef HAVE_ENTITYSTATE_ISJEDIMASTER
    o.isJediMaster = in.isJediMaster;
    #endif
    #ifdef HAVE_ENTITYSTATE_ISPORTALENT
    o.isPortalEnt = in.isPortalEnt;
    #endif
    #ifdef HAVE_ENTITYSTATE_NPC_CLASS
    o.NPC_class = in.NPC_class;
    #endif

    return o;
}

inline entityState_t toLegacy(const EntityState& in) {
    entityState_t o{};
    o.number = in.number;
    o.eType  = in.eType;
    o.eFlags = in.eFlags;

    o.pos  = toLegacy(in.pos);
    o.apos = toLegacy(in.apos);
    o.time  = in.time;
    o.time2 = in.time2;

    o.origin[0]=in.origin.x; o.origin[1]=in.origin.y; o.origin[2]=in.origin.z;
    o.origin2[0]=in.origin2.x; o.origin2[1]=in.origin2.y; o.origin2[2]=in.origin2.z;
    o.angles[0]=in.angles.x; o.angles[1]=in.angles.y; o.angles[2]=in.angles.z;
    o.angles2[0]=in.angles2.x; o.angles2[1]=in.angles2.y; o.angles2[2]=in.angles2.z;

    o.groundEntityNum = in.groundEntityNum;
    o.solid           = in.solid;
    o.constantLight   = in.constantLight;
    o.loopSound       = in.loopSound;

    o.modelindex  = in.modelindex;
    o.modelindex2 = in.modelindex2;
    #ifdef HAVE_ENTITYSTATE_MODELINDEX34
    o.modelindex3 = in.modelindex3; o.modelindex4=in.modelindex4;
    #endif

    o.otherEntityNum  = in.otherEntityNum;
    o.otherEntityNum2 = in.otherEntityNum2;
    o.clientNum       = in.clientNum;

    o.powerups  = in.powerups;
    o.weapon    = in.weapon;
    o.generic1  = in.generic1;
    o.legsAnim  = in.legsAnim;
    o.torsoAnim = in.torsoAnim;
    o.event     = in.event_;
    o.eventParm = in.eventParm;

    #ifdef HAVE_ENTITYSTATE_EFLAGS2
    o.eFlags2 = in.eFlags2;
    #endif
    #ifdef HAVE_ENTITYSTATE_SOUNDSET
    o.soundSetIndex = in.soundSetIndex;
    #endif
    #ifdef HAVE_ENTITYSTATE_IMODELSCALE
    o.iModelScale = in.iModelScale;
    #endif
    #ifdef HAVE_ENTITYSTATE_SURFACESONOFF
    o.surfacesOn  = in.surfacesOn;
    o.surfacesOff = in.surfacesOff;
    #endif
    #ifdef HAVE_ENTITYSTATE_RAG_ATTACH
    o.ragAttach = in.ragAttach;
    #endif
    #ifdef HAVE_ENTITYSTATE_BOLT_TO_PLAYER
    o.boltToPlayer = in.boltToPlayer;
    #endif
    #ifdef HAVE_ENTITYSTATE_BOLTS
    o.bolt1 = in.bolt1; o.bolt2 = in.bolt2;
    #endif
    #ifdef HAVE_ENTITYSTATE_HELDBYCLIENT
    o.heldByClient = in.heldByClient;
    #endif
    #ifdef HAVE_ENTITYSTATE_ISJEDIMASTER
    o.isJediMaster = in.isJediMaster;
    #endif
    #ifdef HAVE_ENTITYSTATE_ISPORTALENT
    o.isPortalEnt = in.isPortalEnt;
    #endif
    #ifdef HAVE_ENTITYSTATE_NPC_CLASS
    o.NPC_class = in.NPC_class;
    #endif

    return o;
}
#endif // JKA_HAVE_Q_SHARED_H

// -----------------------------------------------------------------------------
// JSON (optionnel)
// -----------------------------------------------------------------------------
#ifdef JKA_ENABLE_JSON
  #include <nlohmann/json.hpp>
  inline void to_json(nlohmann::json& j, const EntityState& s) {
      j = nlohmann::json{
          {"number", s.number}, {"eType", s.eType}, {"eFlags", s.eFlags}, {"eFlags2", s.eFlags2},
          {"modelindex", s.modelindex}, {"modelindex2", s.modelindex2},
          {"modelindex3", s.modelindex3}, {"modelindex4", s.modelindex4},
          {"frame", s.frame},
          {"pos", s.pos}, {"apos", s.apos},
          {"time", s.time}, {"time2", s.time2},
          {"origin", {s.origin.x,s.origin.y,s.origin.z}},
          {"origin2",{s.origin2.x,s.origin2.y,s.origin2.z}},
          {"angles", {s.angles.x,s.angles.y,s.angles.z}},
          {"angles2",{s.angles2.x,s.angles2.y,s.angles2.z}},
          {"groundEntityNum", s.groundEntityNum},
          {"solid", s.solid},
          {"constantLight", s.constantLight},
          {"loopSound", s.loopSound},
          {"soundSetIndex", s.soundSetIndex},
          {"otherEntityNum", s.otherEntityNum},
          {"otherEntityNum2", s.otherEntityNum2},
          {"clientNum", s.clientNum},
          {"powerups", s.powerups},
          {"weapon", s.weapon},
          {"generic1", s.generic1},
          {"legsAnim", s.legsAnim},
          {"torsoAnim", s.torsoAnim},
          {"event", s.event_},
          {"eventParm", s.eventParm},
          {"iModelScale", s.iModelScale},
          {"surfacesOn", s.surfacesOn},
          {"surfacesOff", s.surfacesOff},
          {"ragAttach", s.ragAttach},
          {"boltToPlayer", s.boltToPlayer},
          {"bolt1", s.bolt1},
          {"bolt2", s.bolt2},
          {"heldByClient", s.heldByClient},
          {"isJediMaster", s.isJediMaster},
          {"isPortalEnt", s.isPortalEnt},
          {"NPC_class", s.NPC_class},
      };
      if (!s.extrasInt.empty())  j["extrasInt"]  = s.extrasInt;
      if (!s.extrasFloat.empty()) j["extrasFloat"] = s.extrasFloat;
  }

  inline void from_json(const nlohmann::json& j, EntityState& s) {
      auto get = [&](const char* k, auto& ref){
          if (j.contains(k)) j.at(k).get_to(ref);
      };
      get("number", s.number); get("eType", s.eType); get("eFlags", s.eFlags); get("eFlags2", s.eFlags2);
      get("modelindex", s.modelindex); get("modelindex2", s.modelindex2);
      get("modelindex3", s.modelindex3); get("modelindex4", s.modelindex4);
      get("frame", s.frame);
      get("pos", s.pos); get("apos", s.apos);
      get("time", s.time); get("time2", s.time2);

      if (j.contains("origin")) {
          auto a = j.at("origin");
          s.origin = { a.at(0).get<float>(), a.at(1).get<float>(), a.at(2).get<float>() };
      }
      if (j.contains("origin2")) {
          auto a = j.at("origin2");
          s.origin2 = { a.at(0).get<float>(), a.at(1).get<float>(), a.at(2).get<float>() };
      }
      if (j.contains("angles")) {
          auto a = j.at("angles");
          s.angles = { a.at(0).get<float>(), a.at(1).get<float>(), a.at(2).get<float>() };
      }
      if (j.contains("angles2")) {
          auto a = j.at("angles2");
          s.angles2 = { a.at(0).get<float>(), a.at(1).get<float>(), a.at(2).get<float>() };
      }

      get("groundEntityNum", s.groundEntityNum);
      get("solid", s.solid);
      get("constantLight", s.constantLight);
      get("loopSound", s.loopSound);
      get("soundSetIndex", s.soundSetIndex);
      get("otherEntityNum", s.otherEntityNum);
      get("otherEntityNum2", s.otherEntityNum2);
      get("clientNum", s.clientNum);
      get("powerups", s.powerups);
      get("weapon", s.weapon);
      get("generic1", s.generic1);
      get("legsAnim", s.legsAnim);
      get("torsoAnim", s.torsoAnim);
      get("event", s.event_);
      get("eventParm", s.eventParm);
      get("iModelScale", s.iModelScale);
      get("surfacesOn", s.surfacesOn);
      get("surfacesOff", s.surfacesOff);
      get("ragAttach", s.ragAttach);
      get("boltToPlayer", s.boltToPlayer);
      get("bolt1", s.bolt1);
      get("bolt2", s.bolt2);
      get("heldByClient", s.heldByClient);
      get("isJediMaster", s.isJediMaster);
      get("isPortalEnt", s.isPortalEnt);
      get("NPC_class", s.NPC_class);

      if (j.contains("extrasInt"))  s.extrasInt  = j.at("extrasInt").get<decltype(s.extrasInt)>();
      if (j.contains("extrasFloat")) s.extrasFloat = j.at("extrasFloat").get<decltype(s.extrasFloat)>();
  }
#endif // JKA_ENABLE_JSON

} // namespace jka
DEMO_NAMESPACE_END

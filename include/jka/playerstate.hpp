#pragma once
// jka/playerstate.hpp — PlayerState JKA (DM_26) exhaustif + helpers modernes

#include <cstdint>
#include <array>
#include <unordered_map>
#include <string>
#include <string_view>
#include <algorithm>
#include <type_traits>
#include <optional>

#include "Vec3.hpp"              // Vec3T<T>, Vec3f, Vec3i
#include "trajectory.hpp"        // jka::Trajectory
#include "trajectory_types.hpp"  // enums DM_26 / DM_25 si besoin de conversions
#include "defs.h"                // si tu exposes déjà des tailles MAX_* (optionnel)

namespace jka {

// ---------------------------------------------------------------------------
// Tailles par défaut (écrasées si déjà définies dans defs.h)
// ---------------------------------------------------------------------------
#ifndef JKA_MAX_STATS
#define JKA_MAX_STATS 16
#endif
#ifndef JKA_MAX_PERSISTANT
#define JKA_MAX_PERSISTANT 16
#endif
#ifndef JKA_MAX_POWERUPS
#define JKA_MAX_POWERUPS 16
#endif
#ifndef JKA_MAX_AMMO
#define JKA_MAX_AMMO 32
#endif
#ifndef JKA_MAX_WEAPONS
#define JKA_MAX_WEAPONS 32
#endif
#ifndef JKA_MAX_HOLOCRONS
#define JKA_MAX_HOLOCRONS 16
#endif

// ---------------------------------------------------------------------------
// PlayerState — DM_26 exhaustif (organisé en sections logiques)
// ---------------------------------------------------------------------------
struct PlayerState {
    // --- Identité / base ----------------------------------------------------
    int clientNum{0};
    int pm_type{0};         // type de déplacement (PM_NORMAL/NOCLIP/etc.)
    int pm_time{0};
    int pm_flags{0};
    int bobCycle{0};
    int gravity{800};       // convention JKA
    int speed{0};
    int movementDir{0};     // direction discretisée (-1..7)

    // --- Position, vitesse, angles -----------------------------------------
    Vec3f origin{0.f,0.f,0.f};
    Vec3f velocity{0.f,0.f,0.f};
    int   delta_angles[3]{0,0,0};         // offsets entiers (réseau)
    int   viewheight{0};                  // hauteur caméra
    int   groundEntityNum{-1};            // index entité sol (ou -1)
    Vec3f mins{0.f,0.f,0.f};
    Vec3f maxs{0.f,0.f,0.f};
    Vec3f pm_gravityDir{0.f,0.f,-1.f};    // quelques mods exposent une dir gravité

    // Trajectoires caméra (selon implémentations)
    Trajectory viewAngles;  // NOTE : certains mods stockent au format entier ; on garde Trajectory moderne
    Trajectory viewOffset;  // offset caméra (tremblements, zoom smoothed, etc.)

    // --- View / input -------------------------------------------------------
    int   viewAnglesFixed[3]{0,0,0};     // si version entière du réseau exposée
    int   viewFlags{0};                  // flags vue
    int   movementKey{0};                // derniers inputs bruts (option)
    int   buttons{0};                    // boutons courants (bitmask)
    int   weapon{0};                     // arme active
    int   weaponstate{0};                // state arme (recharge/tir)
    int   zoomMode{0};                   // mode zoom (0=off)
    int   zoomTime{0};                   // timer zoom
    int   zoomLocked{0};                 // aim-lock en zoom ?
    int   zoomLockEnt{-1};               // cible lockée
    int   zoomFov{0};                    // fov zoom actif

    // --- Animations/Timers --------------------------------------------------
    int legsAnim{0};
    int torsoAnim{0};
    int legsTimer{0};
    int torsoTimer{0};

    // --- Stats / persistant / powerups / ammo / weapons ---------------------
    int stats[JKA_MAX_STATS]{};
    int persistant[JKA_MAX_PERSISTANT]{};
    int powerups[JKA_MAX_POWERUPS]{};
    int ammo[JKA_MAX_AMMO]{};
    int genericEnemyIndex{0};            // certains mods exposent un index « ennemi »

    // Côté armes / inventaire
    int weaponTime{0};                   // temps de cooldown arme
    int weaponCharge{0};                 // charge (rail/rockets lock)
    int weaponAmmo[JKA_MAX_WEAPONS]{};   // si exposé par mod
    int weaponDisable{0};                // disable mask
    int saberInFlight{0};                // sabre lancé ?

    // --- Damage -------------------------------------------------------------
    int  damageYaw{0};
    int  damagePitch{0};
    int  damageCount{0};
    int  damageEvent{0};

    // --- Events / external --------------------------------------------------
    int  pmove_framecount{0};
    int  onGround{0};                    // bool-like (mod)
    int  externalEvent{0};
    int  externalEventParm{0};
    int  eventSequence{0};
    int  events[2]{0,0};
    int  eventParms[2]{0,0};

    // --- Grapple / Hook -----------------------------------------------------
    int   grappleState{0};
    Vec3f grapplePoint{0.f,0.f,0.f};

    // --- Force / Saber (JKA spécifiques) -----------------------------------
    // (liste étendue pour couvrir mods/vanilla)
    int forceHandExtend{0};
    int forceHandExtendTime{0};
    int saberHolstered{0};
    int saberEntityNum{-1};
    int saberMove{0};
    int saberBlocked{0};
    int saberLockTime{0};
    int saberLockEnemy{-1};
    int saberLockFrame{0};
    int saberInFlightTime{0};
    int saberThrowDelay{0};
    int forcePowersKnown{0};      // bitmask des pouvoirs appris
    int forcePowerLevel[16]{};    // niveau par pouvoir (taille usuelle)
    int forcePower{0};            // réserve/énergie
    int forceRageRecoveryTime{0};
    int forceGripEntityNum{-1};
    int forceGripDamageDebounceTime{0};
    int forceDrainEntityNum{-1};
    int forceDrainTime{0};
    int forceHoldLegs{0};
    int forceHoldTorso{0};
    int forceDodgeAnim{0};
    int emplacedIndex{-1};
    int emplacedTime{0};

    // --- Duel / Team / Score ------------------------------------------------
    int duelIndex{-1};
    int duelTime{0};
    int duelInProgress{0};
    int teamNum{0};
    int isJediMaster{0};
    int isSurrendering{0};

    // --- Locks / Tracking ---------------------------------------------------
    int rocketLockIndex{-1};
    int rocketLockTime{0};
    int rocketLockTimeDebounce{0};
    int rocketTargetTime{0};
    int hasDetPacked{0};
    int detpackPlaced{0};

    // --- Holocrons / Items spéciaux ----------------------------------------
    int holocronBits{0};                   // bitmask holocrons possédés
    int activeHolocron{-1};                // holocron actif
    int holocronHeld[JKA_MAX_HOLOCRONS]{}; // timers/flags par holocron
    int eFlags{0};                         // flags entité player

    // --- Gear / Vehicle / Jetpack / Ragdoll --------------------------------
    int hasJetpack{0};
    int jetpackFuel{0};
    int jetpackToggleTime{0};
    int usingJetpack{0};

    int vehicleID{-1};
    int m_iVehicleNum{-1};                // compat divers mods
    int usingVehicle{0};
    int vehicleTime{0};

    int ragdollState{0};
    int ragdollTimer{0};

    // --- Divers étendus (sécurité de couverture) ----------------------------
    // Pour ne R I E N perdre si un champ exotiques arrive via une netfield
    // non référencée : on stocke dans "extras".
    std::unordered_map<std::string, std::int64_t> extrasInt;
    std::unordered_map<std::string, double>       extrasFloat;

    // -----------------------------------------------------------------------
    // Helpers : recherche « tolérante » (alias OpenJK/usuel)
    // -----------------------------------------------------------------------
    static bool iequals(std::string_view a, std::string_view b) {
        if (a.size() != b.size()) return false;
        for (size_t i=0;i<a.size();++i) {
            char ca = (char)std::tolower((unsigned char)a[i]);
            char cb = (char)std::tolower((unsigned char)b[i]);
            if (ca != cb) return false;
        }
        return true;
    }

    static bool matches(std::string_view name, std::initializer_list<std::string_view> alts) {
        for (auto alt : alts) if (iequals(name, alt)) return true;
        return false;
    }

    // -----------------------------------------------------------------------
    // setByNetfieldName / get*ByNetfieldName — robustes & tolérants
    // - gèrent int/float + index d’array
    // - couvrent les alias usuels d’OpenJK
    // -----------------------------------------------------------------------
    bool setByNetfieldName(std::string_view name, std::int64_t v, std::optional<int> index = std::nullopt) {
        // ---- Arrays usuels -------------------------------------------------
        if (matches(name, {"stats"})) {
            if (index && *index>=0 && *index < (int)JKA_MAX_STATS) { stats[*index] = (int)v; return true; }
            return false;
        }
        if (matches(name, {"persistant", "persistent"})) {
            if (index && *index>=0 && *index < (int)JKA_MAX_PERSISTANT) { persistant[*index] = (int)v; return true; }
            return false;
        }
        if (matches(name, {"powerups"})) {
            if (index && *index>=0 && *index < (int)JKA_MAX_POWERUPS) { powerups[*index] = (int)v; return true; }
            return false;
        }
        if (matches(name, {"ammo"})) {
            if (index && *index>=0 && *index < (int)JKA_MAX_AMMO) { ammo[*index] = (int)v; return true; }
            return false;
        }
        if (matches(name, {"weaponAmmo","weapon_ammo"})) {
            if (index && *index>=0 && *index < (int)JKA_MAX_WEAPONS) { weaponAmmo[*index] = (int)v; return true; }
            return false;
        }
        if (matches(name, {"forcePowerLevel","force_level"})) {
            if (index && *index>=0 && *index < 16) { forcePowerLevel[*index] = (int)v; return true; }
            return false;
        }
        if (matches(name, {"holocronHeld"})) {
            if (index && *index>=0 && *index < (int)JKA_MAX_HOLOCRONS) { holocronHeld[*index] = (int)v; return true; }
            return false;
        }
        if (matches(name, {"delta_angles","deltaAngles"})) {
            if (index && *index>=0 && *index<3) { delta_angles[*index] = (int)v; return true; }
            return false;
        }

        // ---- Scalars connus ------------------------------------------------
        auto S = [&](std::string_view key, int& ref)->bool {
            if (iequals(name, key)) { ref = (int)v; return true; }
            return false;
        };

        if (S("clientNum", clientNum) || S("pm_type", pm_type) || S("pm_time", pm_time) ||
            S("pm_flags", pm_flags) || S("bobCycle", bobCycle) || S("gravity", gravity) ||
            S("speed", speed) || S("movementDir", movementDir) || S("viewheight", viewheight) ||
            S("groundEntityNum", groundEntityNum) || S("viewFlags", viewFlags) || S("movementKey", movementKey) ||
            S("buttons", buttons) || S("weapon", weapon) || S("weaponstate", weaponstate) ||
            S("zoomMode", zoomMode) || S("zoomTime", zoomTime) || S("zoomLocked", zoomLocked) ||
            S("zoomLockEnt", zoomLockEnt) || S("zoomFov", zoomFov) ||
            S("legsAnim", legsAnim) || S("torsoAnim", torsoAnim) || S("legsTimer", legsTimer) || S("torsoTimer", torsoTimer) ||
            S("weaponTime", weaponTime) || S("weaponCharge", weaponCharge) || S("weaponDisable", weaponDisable) ||
            S("saberInFlight", saberInFlight) ||
            S("damageYaw", damageYaw) || S("damagePitch", damagePitch) || S("damageCount", damageCount) || S("damageEvent", damageEvent) ||
            S("pmove_framecount", pmove_framecount) || S("onGround", onGround) ||
            S("externalEvent", externalEvent) || S("externalEventParm", externalEventParm) ||
            S("eventSequence", eventSequence) ||
            S("events0", events[0]) || S("events1", events[1]) ||
            S("eventParms0", eventParms[0]) || S("eventParms1", eventParms[1]) ||
            S("grappleState", grappleState) ||
            S("forceHandExtend", forceHandExtend) || S("forceHandExtendTime", forceHandExtendTime) ||
            S("saberHolstered", saberHolstered) || S("saberEntityNum", saberEntityNum) ||
            S("saberMove", saberMove) || S("saberBlocked", saberBlocked) ||
            S("saberLockTime", saberLockTime) || S("saberLockEnemy", saberLockEnemy) || S("saberLockFrame", saberLockFrame) ||
            S("saberInFlightTime", saberInFlightTime) || S("saberThrowDelay", saberThrowDelay) ||
            S("forcePowersKnown", forcePowersKnown) || S("forcePower", forcePower) ||
            S("forceRageRecoveryTime", forceRageRecoveryTime) || S("forceGripEntityNum", forceGripEntityNum) ||
            S("forceGripDamageDebounceTime", forceGripDamageDebounceTime) || S("forceDrainEntityNum", forceDrainEntityNum) ||
            S("forceDrainTime", forceDrainTime) || S("forceHoldLegs", forceHoldLegs) || S("forceHoldTorso", forceHoldTorso) ||
            S("forceDodgeAnim", forceDodgeAnim) || S("emplacedIndex", emplacedIndex) || S("emplacedTime", emplacedTime) ||
            S("duelIndex", duelIndex) || S("duelTime", duelTime) || S("duelInProgress", duelInProgress) ||
            S("teamNum", teamNum) || S("isJediMaster", isJediMaster) || S("isSurrendering", isSurrendering) ||
            S("rocketLockIndex", rocketLockIndex) || S("rocketLockTime", rocketLockTime) ||
            S("rocketLockTimeDebounce", rocketLockTimeDebounce) || S("rocketTargetTime", rocketTargetTime) ||
            S("hasDetPacked", hasDetPacked) || S("detpackPlaced", detpackPlaced) ||
            S("holocronBits", holocronBits) || S("activeHolocron", activeHolocron) ||
            S("eFlags", eFlags) ||
            S("hasJetpack", hasJetpack) || S("jetpackFuel", jetpackFuel) || S("jetpackToggleTime", jetpackToggleTime) || S("usingJetpack", usingJetpack) ||
            S("vehicleID", vehicleID) || S("m_iVehicleNum", m_iVehicleNum) || S("usingVehicle", usingVehicle) || S("vehicleTime", vehicleTime) ||
            S("ragdollState", ragdollState) || S("ragdollTimer", ragdollTimer) ||
            S("genericEnemyIndex", genericEnemyIndex)) {
            return true;
        }

        // vecteurs (intentionnellement pas convertis depuis int ici)
        // -> on range dans extras si demande « origin[0] » côté int
        extrasInt[std::string(name)] = v;
        return false;
    }

    bool setByNetfieldName(std::string_view name, double f, std::optional<int> index = std::nullopt) {
        auto V3 = [&](std::string_view key, Vec3f& ref)->bool {
            if (iequals(name, key)) { /* set complet non ambigu → en JSON uniquement */ return false; }
            if (index) {
                if (iequals(name, std::string(key) + "[0]") || iequals(name, std::string(key) + ".x")) { ref.x = static_cast<float>(f); return true; }
                if (iequals(name, std::string(key) + "[1]") || iequals(name, std::string(key) + ".y")) { ref.y = static_cast<float>(f); return true; }
                if (iequals(name, std::string(key) + "[2]") || iequals(name, std::string(key) + ".z")) { ref.z = static_cast<float>(f); return true; }
            }
            return false;
        };
        if (V3("origin", origin) || V3("velocity", velocity) || V3("mins", mins) || V3("maxs", maxs) ||
            V3("grapplePoint", grapplePoint) || V3("pm_gravityDir", pm_gravityDir)) {
            return true;
        }
        extrasFloat[std::string(name)] = f;
        return false;
    }

    // get* : utile pour debug/export
    std::optional<std::int64_t> getIntByNetfieldName(std::string_view name, std::optional<int> idx = std::nullopt) const {
        auto GArr = [&](std::string_view key, const int* arr, int N)->std::optional<std::int64_t> {
            if (iequals(name, key) && idx && *idx>=0 && *idx<N) return arr[*idx];
            return std::nullopt;
        };
        if (auto v=GArr("stats", stats, JKA_MAX_STATS)) return v;
        if (auto v=GArr("persistant", persistant, JKA_MAX_PERSISTANT)) return v;
        if (auto v=GArr("persistent", persistant, JKA_MAX_PERSISTANT)) return v;
        if (auto v=GArr("powerups", powerups, JKA_MAX_POWERUPS)) return v;
        if (auto v=GArr("ammo", ammo, JKA_MAX_AMMO)) return v;
        if (auto v=GArr("weaponAmmo", weaponAmmo, JKA_MAX_WEAPONS)) return v;
        if (auto v=GArr("forcePowerLevel", forcePowerLevel, 16)) return v;
        if (auto v=GArr("holocronHeld", holocronHeld, JKA_MAX_HOLOCRONS)) return v;
        if (auto v=GArr("delta_angles", delta_angles, 3)) return v;

        // scalars
        auto S = [&](std::string_view key, int val)->std::optional<std::int64_t> {
            if (iequals(name, key)) return val;
            return std::nullopt;
        };
        #define JKA_SRET(k) if (auto r=S(#k,k)) return r
        JKA_SRET(clientNum); JKA_SRET(pm_type); JKA_SRET(pm_time); JKA_SRET(pm_flags); JKA_SRET(bobCycle);
        JKA_SRET(gravity); JKA_SRET(speed); JKA_SRET(movementDir); JKA_SRET(viewheight); JKA_SRET(groundEntityNum);
        JKA_SRET(viewFlags); JKA_SRET(movementKey); JKA_SRET(buttons); JKA_SRET(weapon); JKA_SRET(weaponstate);
        JKA_SRET(zoomMode); JKA_SRET(zoomTime); JKA_SRET(zoomLocked); JKA_SRET(zoomLockEnt); JKA_SRET(zoomFov);
        JKA_SRET(legsAnim); JKA_SRET(torsoAnim); JKA_SRET(legsTimer); JKA_SRET(torsoTimer);
        JKA_SRET(weaponTime); JKA_SRET(weaponCharge); JKA_SRET(weaponDisable); JKA_SRET(saberInFlight);
        JKA_SRET(damageYaw); JKA_SRET(damagePitch); JKA_SRET(damageCount); JKA_SRET(damageEvent);
        JKA_SRET(pmove_framecount); JKA_SRET(onGround); JKA_SRET(externalEvent); JKA_SRET(externalEventParm);
        JKA_SRET(eventSequence); JKA_SRET(eFlags);
        JKA_SRET(events[0]); JKA_SRET(events[1]); JKA_SRET(eventParms[0]); JKA_SRET(eventParms[1]);
        JKA_SRET(grappleState);
        JKA_SRET(forceHandExtend); JKA_SRET(forceHandExtendTime); JKA_SRET(saberHolstered); JKA_SRET(saberEntityNum);
        JKA_SRET(saberMove); JKA_SRET(saberBlocked); JKA_SRET(saberLockTime); JKA_SRET(saberLockEnemy);
        JKA_SRET(saberLockFrame); JKA_SRET(saberInFlightTime); JKA_SRET(saberThrowDelay); JKA_SRET(forcePowersKnown);
        JKA_SRET(forcePower); JKA_SRET(forceRageRecoveryTime); JKA_SRET(forceGripEntityNum);
        JKA_SRET(forceGripDamageDebounceTime); JKA_SRET(forceDrainEntityNum); JKA_SRET(forceDrainTime);
        JKA_SRET(forceHoldLegs); JKA_SRET(forceHoldTorso); JKA_SRET(forceDodgeAnim);
        JKA_SRET(emplacedIndex); JKA_SRET(emplacedTime);
        JKA_SRET(duelIndex); JKA_SRET(duelTime); JKA_SRET(duelInProgress); JKA_SRET(teamNum);
        JKA_SRET(isJediMaster); JKA_SRET(isSurrendering);
        JKA_SRET(rocketLockIndex); JKA_SRET(rocketLockTime); JKA_SRET(rocketLockTimeDebounce); JKA_SRET(rocketTargetTime);
        JKA_SRET(hasDetPacked); JKA_SRET(detpackPlaced);
        JKA_SRET(holocronBits); JKA_SRET(activeHolocron);
        JKA_SRET(hasJetpack); JKA_SRET(jetpackFuel); JKA_SRET(jetpackToggleTime); JKA_SRET(usingJetpack);
        JKA_SRET(vehicleID); JKA_SRET(m_iVehicleNum); JKA_SRET(usingVehicle); JKA_SRET(vehicleTime);
        JKA_SRET(ragdollState); JKA_SRET(ragdollTimer);
        JKA_SRET(genericEnemyIndex);
        #undef JKA_SRET

        // extras
        if (auto it = extrasInt.find(std::string(name)); it != extrasInt.end())
            return it->second;
        return std::nullopt;
    }

    std::optional<double> getFloatByNetfieldName(std::string_view name, std::optional<int> index = std::nullopt) const {
        auto G3 = [&](std::string_view key, const Vec3f& v)->std::optional<double> {
            if (!index) return std::nullopt;
            if (iequals(name, std::string(key)+ "[0]") || iequals(name, std::string(key)+ ".x")) return v.x;
            if (iequals(name, std::string(key)+ "[1]") || iequals(name, std::string(key)+ ".y")) return v.y;
            if (iequals(name, std::string(key)+ "[2]") || iequals(name, std::string(key)+ ".z")) return v.z;
            return std::nullopt;
        };
        if (auto r=G3("origin", origin)) return r;
        if (auto r=G3("velocity", velocity)) return r;
        if (auto r=G3("mins", mins)) return r;
        if (auto r=G3("maxs", maxs)) return r;
        if (auto r=G3("grapplePoint", grapplePoint)) return r;
        if (auto r=G3("pm_gravityDir", pm_gravityDir)) return r;

        if (auto it = extrasFloat.find(std::string(name)); it != extrasFloat.end())
            return it->second;
        return std::nullopt;
    }
};

} // namespace jka


// ---------------------------------------------------------------------------
// JSON optionnel (activé uniquement si JKA_ENABLE_JSON est défini)
// ---------------------------------------------------------------------------
#ifdef JKA_ENABLE_JSON
  #include <nlohmann/json.hpp>
  namespace nlohmann {
    template<>
    struct adl_serializer<jka::PlayerState> {
      static void to_json(json& j, const jka::PlayerState& p) {
        j = json{
          // Base
          {"clientNum", p.clientNum}, {"pm_type", p.pm_type}, {"pm_time", p.pm_time},
          {"pm_flags", p.pm_flags}, {"bobCycle", p.bobCycle}, {"gravity", p.gravity},
          {"speed", p.speed}, {"movementDir", p.movementDir},

          // Vects/angles
          {"origin", {p.origin.x, p.origin.y, p.origin.z}},
          {"velocity", {p.velocity.x, p.velocity.y, p.velocity.z}},
          {"delta_angles", {p.delta_angles[0],p.delta_angles[1],p.delta_angles[2]}},
          {"viewheight", p.viewheight}, {"groundEntityNum", p.groundEntityNum},
          {"mins", {p.mins.x,p.mins.y,p.mins.z}}, {"maxs", {p.maxs.x,p.maxs.y,p.maxs.z}},
          {"pm_gravityDir",{p.pm_gravityDir.x,p.pm_gravityDir.y,p.pm_gravityDir.z}},

          // View / input
          {"viewFlags", p.viewFlags},{"movementKey",p.movementKey},{"buttons",p.buttons},
          {"weapon",p.weapon},{"weaponstate",p.weaponstate},
          {"zoomMode",p.zoomMode},{"zoomTime",p.zoomTime},{"zoomLocked",p.zoomLocked},
          {"zoomLockEnt",p.zoomLockEnt},{"zoomFov",p.zoomFov},

          // Anim
          {"legsAnim",p.legsAnim},{"torsoAnim",p.torsoAnim},{"legsTimer",p.legsTimer},{"torsoTimer",p.torsoTimer},

          // Arrays
          {"stats", p.stats}, {"persistant", p.persistant},
          {"powerups", p.powerups}, {"ammo", p.ammo}, {"weaponAmmo", p.weaponAmmo},

          // Weapons / misc
          {"weaponTime", p.weaponTime},{"weaponCharge",p.weaponCharge},{"weaponDisable",p.weaponDisable},
          {"saberInFlight", p.saberInFlight},

          // Damage
          {"damageYaw",p.damageYaw},{"damagePitch",p.damagePitch},{"damageCount",p.damageCount},{"damageEvent",p.damageEvent},

          // Events/ext
          {"pmove_framecount",p.pmove_framecount},{"onGround",p.onGround},
          {"externalEvent",p.externalEvent},{"externalEventParm",p.externalEventParm},
          {"eventSequence",p.eventSequence},
          {"events",{p.events[0],p.events[1]}},{"eventParms",{p.eventParms[0],p.eventParms[1]}},

          // Grapple
          {"grappleState",p.grappleState},{"grapplePoint",{p.grapplePoint.x,p.grapplePoint.y,p.grapplePoint.z}},

          // Force/Saber
          {"forceHandExtend",p.forceHandExtend},{"forceHandExtendTime",p.forceHandExtendTime},
          {"saberHolstered",p.saberHolstered},{"saberEntityNum",p.saberEntityNum},
          {"saberMove",p.saberMove},{"saberBlocked",p.saberBlocked},
          {"saberLockTime",p.saberLockTime},{"saberLockEnemy",p.saberLockEnemy},{"saberLockFrame",p.saberLockFrame},
          {"saberInFlightTime",p.saberInFlightTime},{"saberThrowDelay",p.saberThrowDelay},
          {"forcePowersKnown",p.forcePowersKnown},{"forcePower",p.forcePower},
          {"forceRageRecoveryTime",p.forceRageRecoveryTime},{"forceGripEntityNum",p.forceGripEntityNum},
          {"forceGripDamageDebounceTime",p.forceGripDamageDebounceTime},{"forceDrainEntityNum",p.forceDrainEntityNum},
          {"forceDrainTime",p.forceDrainTime},{"forceHoldLegs",p.forceHoldLegs},{"forceHoldTorso",p.forceHoldTorso},
          {"forceDodgeAnim",p.forceDodgeAnim},{"emplacedIndex",p.emplacedIndex},{"emplacedTime",p.emplacedTime},
          {"forcePowerLevel", p.forcePowerLevel},

          // Duel/Team
          {"duelIndex",p.duelIndex},{"duelTime",p.duelTime},{"duelInProgress",p.duelInProgress},
          {"teamNum",p.teamNum},{"isJediMaster",p.isJediMaster},{"isSurrendering",p.isSurrendering},

          // Locks
          {"rocketLockIndex",p.rocketLockIndex},{"rocketLockTime",p.rocketLockTime},
          {"rocketLockTimeDebounce",p.rocketLockTimeDebounce},{"rocketTargetTime",p.rocketTargetTime},
          {"hasDetPacked",p.hasDetPacked},{"detpackPlaced",p.detpackPlaced},

          // Holocrons / flags
          {"holocronBits",p.holocronBits},{"activeHolocron",p.activeHolocron},
          {"holocronHeld", p.holocronHeld},{"eFlags",p.eFlags},

          // Gear / vehicle / ragdoll
          {"hasJetpack",p.hasJetpack},{"jetpackFuel",p.jetpackFuel},{"jetpackToggleTime",p.jetpackToggleTime},{"usingJetpack",p.usingJetpack},
          {"vehicleID",p.vehicleID},{"m_iVehicleNum",p.m_iVehicleNum},{"usingVehicle",p.usingVehicle},{"vehicleTime",p.vehicleTime},
          {"ragdollState",p.ragdollState},{"ragdollTimer",p.ragdollTimer},

          // Divers
          {"genericEnemyIndex",p.genericEnemyIndex}
        };

        // extras
        if (!p.extrasInt.empty())  j["extrasInt"]  = p.extrasInt;
        if (!p.extrasFloat.empty()) j["extrasFloat"] = p.extrasFloat;

        // Trajectories (en JSON « riche » si désiré)
        // On exporte uniquement les bases vectorielles (déjà couvert par origin/velocity),
        // mais tu peux ajouter viewAngles/viewOffset ici avec un adl_serializer<Trajectory>.
      }

      static void from_json(const json& j, jka::PlayerState& p) {
        auto Jget = [&](auto key, auto& out) {
          if (j.contains(key)) j.at(key).get_to(out);
        };

        Jget("clientNum", p.clientNum); Jget("pm_type", p.pm_type); Jget("pm_time", p.pm_time);
        Jget("pm_flags", p.pm_flags); Jget("bobCycle", p.bobCycle); Jget("gravity", p.gravity);
        Jget("speed", p.speed); Jget("movementDir", p.movementDir);

        // vectors
        if (j.contains("origin")) {
          p.origin = jka::Vec3f(j["origin"].at(0).get<float>(), j["origin"].at(1).get<float>(), j["origin"].at(2).get<float>());
        }
        if (j.contains("velocity")) {
          p.velocity = jka::Vec3f(j["velocity"].at(0).get<float>(), j["velocity"].at(1).get<float>(), j["velocity"].at(2).get<float>());
        }
        if (j.contains("mins")) {
          p.mins = jka::Vec3f(j["mins"].at(0).get<float>(), j["mins"].at(1).get<float>(), j["mins"].at(2).get<float>());
        }
        if (j.contains("maxs")) {
          p.maxs = jka::Vec3f(j["maxs"].at(0).get<float>(), j["maxs"].at(1).get<float>(), j["maxs"].at(2).get<float>());
        }
        if (j.contains("pm_gravityDir")) {
          p.pm_gravityDir = jka::Vec3f(j["pm_gravityDir"].at(0).get<float>(), j["pm_gravityDir"].at(1).get<float>(), j["pm_gravityDir"].at(2).get<float>());
        }

        Jget("delta_angles", p.delta_angles);
        Jget("viewheight", p.viewheight); Jget("groundEntityNum", p.groundEntityNum);
        Jget("viewFlags", p.viewFlags); Jget("movementKey", p.movementKey); Jget("buttons", p.buttons);
        Jget("weapon", p.weapon); Jget("weaponstate", p.weaponstate);
        Jget("zoomMode", p.zoomMode); Jget("zoomTime", p.zoomTime); Jget("zoomLocked", p.zoomLocked);
        Jget("zoomLockEnt", p.zoomLockEnt); Jget("zoomFov", p.zoomFov);

        Jget("legsAnim", p.legsAnim); Jget("torsoAnim", p.torsoAnim);
        Jget("legsTimer", p.legsTimer); Jget("torsoTimer", p.torsoTimer);

        Jget("stats", p.stats); Jget("persistant", p.persistant);
        Jget("powerups", p.powerups); Jget("ammo", p.ammo); Jget("weaponAmmo", p.weaponAmmo);

        Jget("weaponTime", p.weaponTime); Jget("weaponCharge", p.weaponCharge);
        Jget("weaponDisable", p.weaponDisable); Jget("saberInFlight", p.saberInFlight);

        Jget("damageYaw", p.damageYaw); Jget("damagePitch", p.damagePitch);
        Jget("damageCount", p.damageCount); Jget("damageEvent", p.damageEvent);

        Jget("pmove_framecount", p.pmove_framecount); Jget("onGround", p.onGround);
        Jget("externalEvent", p.externalEvent); Jget("externalEventParm", p.externalEventParm);
        Jget("eventSequence", p.eventSequence);
        Jget("events", p.events); Jget("eventParms", p.eventParms);

        Jget("grappleState", p.grappleState);
        if (j.contains("grapplePoint")) {
          p.grapplePoint = jka::Vec3f(j["grapplePoint"].at(0).get<float>(), j["grapplePoint"].at(1).get<float>(), j["grapplePoint"].at(2).get<float>());
        }

        Jget("forceHandExtend", p.forceHandExtend); Jget("forceHandExtendTime", p.forceHandExtendTime);
        Jget("saberHolstered", p.saberHolstered); Jget("saberEntityNum", p.saberEntityNum);
        Jget("saberMove", p.saberMove); Jget("saberBlocked", p.saberBlocked);
        Jget("saberLockTime", p.saberLockTime); Jget("saberLockEnemy", p.saberLockEnemy); Jget("saberLockFrame", p.saberLockFrame);
        Jget("saberInFlightTime", p.saberInFlightTime); Jget("saberThrowDelay", p.saberThrowDelay);
        Jget("forcePowersKnown", p.forcePowersKnown); Jget("forcePower", p.forcePower);
        Jget("forceRageRecoveryTime", p.forceRageRecoveryTime); Jget("forceGripEntityNum", p.forceGripEntityNum);
        Jget("forceGripDamageDebounceTime", p.forceGripDamageDebounceTime); Jget("forceDrainEntityNum", p.forceDrainEntityNum);
        Jget("forceDrainTime", p.forceDrainTime); Jget("forceHoldLegs", p.forceHoldLegs); Jget("forceHoldTorso", p.forceHoldTorso);
        Jget("forceDodgeAnim", p.forceDodgeAnim); Jget("emplacedIndex", p.emplacedIndex); Jget("emplacedTime", p.emplacedTime);
        Jget("forcePowerLevel", p.forcePowerLevel);

        Jget("duelIndex", p.duelIndex); Jget("duelTime", p.duelTime); Jget("duelInProgress", p.duelInProgress);
        Jget("teamNum", p.teamNum); Jget("isJediMaster", p.isJediMaster); Jget("isSurrendering", p.isSurrendering);

        Jget("rocketLockIndex", p.rocketLockIndex); Jget("rocketLockTime", p.rocketLockTime);
        Jget("rocketLockTimeDebounce", p.rocketLockTimeDebounce); Jget("rocketTargetTime", p.rocketTargetTime);
        Jget("hasDetPacked", p.hasDetPacked); Jget("detpackPlaced", p.detpackPlaced);

        Jget("holocronBits", p.holocronBits); Jget("activeHolocron", p.activeHolocron);
        Jget("holocronHeld", p.holocronHeld); Jget("eFlags", p.eFlags);

        Jget("hasJetpack", p.hasJetpack); Jget("jetpackFuel", p.jetpackFuel);
        Jget("jetpackToggleTime", p.jetpackToggleTime); Jget("usingJetpack", p.usingJetpack);

        Jget("vehicleID", p.vehicleID); Jget("m_iVehicleNum", p.m_iVehicleNum);
        Jget("usingVehicle", p.usingVehicle); Jget("vehicleTime", p.vehicleTime);

        Jget("ragdollState", p.ragdollState); Jget("ragdollTimer", p.ragdollTimer);

        Jget("genericEnemyIndex", p.genericEnemyIndex);

        if (j.contains("extrasInt"))  j.at("extrasInt").get_to(p.extrasInt);
        if (j.contains("extrasFloat")) j.at("extrasFloat").get_to(p.extrasFloat);
      }
    };
  } // namespace nlohmann
#endif // JKA_ENABLE_JSON

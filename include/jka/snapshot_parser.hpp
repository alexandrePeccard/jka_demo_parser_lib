#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <type_traits>

#include "defs.h"
#include "messagebuffer.hpp"     // primitives de lecture binaire
#include "snapshot.hpp"          // modèle de données moderne (Snapshot)
#include "playerstate.hpp"
#include "entitystate.hpp"
#include "usercmd.hpp"           // modèle moderne des inputs
#include "netfields.hpp"         // tables NetField DM_26 (player, vehicle, entity)
#include "TrajectoryEvaluator.hpp" // pour décoder correctement les trTypes si besoin

namespace jka {

// ===============================
// Options & hooks d’extension
// ===============================

/// Protocole supporté (priorité : DM_26).
enum class Protocol : std::uint8_t { DM_26 = 26, DM_25 = 25 };

/// Options de parsing (peuvent évoluer).
struct SnapshotParseOptions {
  Protocol protocol { Protocol::DM_26 };
  bool     strictAreaMask { false };  ///< si vrai: erreur si areaMask out-of-bounds
  bool     keepUnknownNetfields { true }; ///< si vrai: ignore proprement les champs inconnus
  int      maxEntities { 1024 };      ///< borne sécurité
  int      maxClients  { 64 };        ///< borne sécurité pour usercmds
};

/// Hooks facultatifs pour instrumentation/extension.
struct SnapshotParseHooks {
  // Appelé juste avant parse d’un snapshot.
  std::function<void(const MessageBuffer&)> onBeforeSnapshot;

  // Appelé après parse complet d’un snapshot.
  std::function<void(const Snapshot&)> onAfterSnapshot;

  // Appelé lorsqu’un NetField inconnu est rencontré (ex: mod custom).
  std::function<void(const char* tableName, const char* fieldName)> onUnknownNetfield;

  // Appelé pour journaliser un delta appliqué (debug).
  std::function<void(const char* tableName, const char* fieldName)> onDeltaApplied;
};

// ===============================
// Baselines internes
// ===============================

/// Baseline par client pour UserCmd.
struct UserCmdBaseline {
  // baseline simple ; dans DM_26 on fait du delta usercmd côté réseau
  std::vector<UserCmd> byClient; // indexé par clientNum

  explicit UserCmdBaseline(int maxClients = 64) : byClient(static_cast<size_t>(maxClients)) {}
};

/// Baselines pour les states.
struct StateBaselines {
  PlayerState player;                        // baseline playerState
  PlayerState vehicle;                       // baseline véhicule (JKA)
  std::unordered_map<int, EntityState> ent;  // baseline entites par entityNum
};

// ===============================
// Parseur de snapshot (DM_26)
// ===============================

class SnapshotParser {
public:
  explicit SnapshotParser(SnapshotParseOptions opts = {}, SnapshotParseHooks hooks = {})
  : opts_(opts), hooks_(hooks), usercmdBaseline_(opts.maxClients) {}

  /// Parse un snapshot complet depuis `msg`, met à jour `baselines_` et remplit `out`.
  /// Retourne false si fin de flux/aucun snapshot lisible.
  bool parse(MessageBuffer& msg, Snapshot& out) {
    if (hooks_.onBeforeSnapshot) hooks_.onBeforeSnapshot(msg);
    const std::size_t startPos = msg.tell();

    try {
      // 1) En-tête (ordre et tailles inspirés d’OpenJK — DM_26)
      // Les implémentations réelles lisent souvent avec MSG_* ; ici on s’appuie sur MessageBuffer.
      out.messageNum             = msg.readLong();   // int32
      out.serverCommandSequence  = msg.readLong();   // int32
      out.serverTime             = msg.readLong();   // int32
      out.deltaNum               = msg.readByte();   // int8
      out.flags                  = msg.readByte();   // int8

      // 2) AreaMask
      const int areaMaskLen = msg.readByte();
      out.areaMask.resize(static_cast<std::size_t>(areaMaskLen));
      for (int i=0;i<areaMaskLen;i++) {
        if (opts_.strictAreaMask && i >= 32) {
          throw std::runtime_error("areaMask too large");
        }
        out.areaMask[static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(msg.readByte());
      }

      // 3) PlayerState (delta vs baseline)
      readDelta_PlayerState(msg, out.playerState, baselines_.player);

      // 4) VehicleState (spécifique JKA) — même logique que player
      readDelta_PlayerState(msg, out.vehicleState, baselines_.vehicle);

      // 5) Entity states (liste compressée : removed/unchanged/updated/added)
      readDelta_Entities(msg, out.entities);

      // 6) UserCmd(s) agrégés (par client) — si présents dans ce message/snapshot
      //    -> suivant protocole serveur : ici on propose un bloc optionnel.
      if (msg.peekControlBit(/*custom marker/heuristic*/)) {
        readDelta_UserCmds(msg, out.usercmds);
      }

      // 7) MAJ des baselines après succès
      baselines_.player  = out.playerState;
      baselines_.vehicle = out.vehicleState;
      // entities baseline mise à jour dans readDelta_Entities()

      if (hooks_.onAfterSnapshot) hooks_.onAfterSnapshot(out);
      return true;

    } catch (const std::exception& e) {
      // En cas d’erreur, on remet le curseur pour permettre une tentative de resync plus haut
      msg.seek(startPos);
      (void)e; // journalisable
      return false;
    }
  }

  // Accès aux baselines (pour debug/outils)
  const StateBaselines& baselines() const noexcept { return baselines_; }
  const UserCmdBaseline& usercmdBaseline() const noexcept { return usercmdBaseline_; }

private:
  SnapshotParseOptions opts_;
  SnapshotParseHooks   hooks_;
  StateBaselines       baselines_;
  UserCmdBaseline      usercmdBaseline_;

  // ---------- Helpers de lecture générique pour NetField ----------

  template <typename T>
  static void assignDeltaField(T& dst, const T& val) {
    dst = val;
  }

  static void assignDeltaField(Vec3& dst, const Vec3& val) {
    dst = val;
  }
  static void assignDeltaField(Vec3i& dst, const Vec3i& val) {
    dst = val;
  }

  // Lecture scalaire de base (selon NetField::Type)
  inline int   readInt(MessageBuffer& msg)   { return msg.readLong(); }
  inline float readFloat(MessageBuffer& msg) { return msg.readFloat(); }
  inline int   readShort(MessageBuffer& msg) { return msg.readShort(); }
  inline int   readByte(MessageBuffer& msg)  { return msg.readByte(); }

  inline Vec3  readVec3(MessageBuffer& msg)  {
    return Vec3{ msg.readFloat(), msg.readFloat(), msg.readFloat() };
  }
  inline Vec3i readVec3i(MessageBuffer& msg) {
    // DM_26 encode généralement en int16 (ou int32 selon champs) avec facteur (ex. ANGLE2SHORT etc.)
    // Ici on lit en int32 pour rester générique ; l’adapter décidera du scaling lors de la reconstruction.
    return Vec3i{ msg.readLong(), msg.readLong(), msg.readLong() };
  }

  // ---------- PlayerState delta ----------

  void readDelta_PlayerState(MessageBuffer& msg,
                             PlayerState& out,
                             const PlayerState& base) {
    using Table = netfields::PlayerStateTable; // fourni par netfields.hpp
    const auto& tbl = Table::fields();

    // Flag “changed” par champ (schéma OpenJK : bitmask + payloads)
    for (const auto& nf : tbl) {
      if (!msg.readBit()) {
        // pas de changement -> copie du baseline
        applyBaseline_PlayerStateField(out, base, nf.name);
        continue;
      }

      // changement -> lecture selon type
      if (hooks_.onDeltaApplied) hooks_.onDeltaApplied("playerState", nf.name);

      switch (nf.type) {
        case NetFieldType::Int:
          writePlayerField(out, nf.name, readInt(msg));
          break;
        case NetFieldType::Short:
          writePlayerField(out, nf.name, readShort(msg));
          break;
        case NetFieldType::Byte:
          writePlayerField(out, nf.name, readByte(msg));
          break;
        case NetFieldType::Float:
          writePlayerField(out, nf.name, readFloat(msg));
          break;
        case NetFieldType::Vec3:
          writePlayerField(out, nf.name, readVec3(msg));
          break;
        case NetFieldType::Vec3i:
          writePlayerField(out, nf.name, readVec3i(msg));
          break;
        default:
          // extension / custom
          if (hooks_.onUnknownNetfield) hooks_.onUnknownNetfield("playerState", nf.name);
          skipUnknown(msg, nf);
          break;
      }
    }
  }

  // Copie baseline → champ ciblé (par nom symbolique)
  static void applyBaseline_PlayerStateField(PlayerState& out,
                                             const PlayerState& base,
                                             const char* fieldName);

  // Écrit un champ du PlayerState par nom (surcharges)
  static void writePlayerField(PlayerState& out, const char* fieldName, int v);
  static void writePlayerField(PlayerState& out, const char* fieldName, float v);
  static void writePlayerField(PlayerState& out, const char* fieldName, Vec3 v);
  static void writePlayerField(PlayerState& out, const char* fieldName, Vec3i v);

  // ---------- Entity delta list ----------

  void readDelta_Entities(MessageBuffer& msg,
                          std::unordered_map<int, EntityState>& outMap) {
    // Liste compressée : on lit entityNum (croissant), puis delta/removed/added.
    // Inspiré du schéma Q3/JK : entityNum = msg.readShort(); 0xFFFF => fin.
    int lastNum = -1;
    for (;;) {
      int num = msg.readShort();
      if (num == 0xFFFF) break; // terminator

      if (num < 0 || num >= opts_.maxEntities) {
        throw std::runtime_error("entityNum out of bounds");
      }
      if (num <= lastNum) {
        // entités doivent arriver triées ascendant ; si pas le cas, on accepte quand même
        lastNum = num;
      } else {
        lastNum = num;
      }

      bool remove = msg.readBit();
      if (remove) {
        outMap.erase(num);
        baselines_.ent.erase(num);
        continue;
      }

      EntityState& dst = outMap[num];               // upsert
      const auto itBase = baselines_.ent.find(num);
      const EntityState* base = (itBase != baselines_.ent.end()) ? &itBase->second : nullptr;

      readDelta_EntityState(msg, dst, base);
      baselines_.ent[num] = dst;                    // maj baseline
    }
  }

  void readDelta_EntityState(MessageBuffer& msg,
                             EntityState& out,
                             const EntityState* baseOpt) {
    using Table = netfields::EntityStateTable;
    const auto& tbl = Table::fields();

    const EntityState& base = baseOpt ? *baseOpt : EntityState{};
    for (const auto& nf : tbl) {
      if (!msg.readBit()) {
        applyBaseline_EntityField(out, base, nf.name);
        continue;
      }
      if (hooks_.onDeltaApplied) hooks_.onDeltaApplied("entityState", nf.name);

      switch (nf.type) {
        case NetFieldType::Int:   writeEntityField(out, nf.name, readInt(msg));   break;
        case NetFieldType::Short: writeEntityField(out, nf.name, readShort(msg)); break;
        case NetFieldType::Byte:  writeEntityField(out, nf.name, readByte(msg));  break;
        case NetFieldType::Float: writeEntityField(out, nf.name, readFloat(msg)); break;
        case NetFieldType::Vec3:  writeEntityField(out, nf.name, readVec3(msg));  break;
        case NetFieldType::Vec3i: writeEntityField(out, nf.name, readVec3i(msg)); break;
        default:
          if (hooks_.onUnknownNetfield) hooks_.onUnknownNetfield("entityState", nf.name);
          skipUnknown(msg, nf);
          break;
      }
    }
  }

  // Copie baseline → champ ciblé (EntityState)
  static void applyBaseline_EntityField(EntityState& out,
                                        const EntityState& base,
                                        const char* fieldName);

  // Écrit un champ de l’EntityState par nom (surcharges)
  static void writeEntityField(EntityState& out, const char* fieldName, int v);
  static void writeEntityField(EntityState& out, const char* fieldName, float v);
  static void writeEntityField(EntityState& out, const char* fieldName, Vec3 v);
  static void writeEntityField(EntityState& out, const char* fieldName, Vec3i v);

  // ---------- UserCmd delta list ----------

  void readDelta_UserCmds(MessageBuffer& msg, std::vector<UserCmd>& out) {
    // Bloc optionnel : N commandes (clientNum, delta vs baseline[clientNum])
    const int count = msg.readByte();
    if (count <= 0) return;

    out.clear();
    out.reserve(static_cast<std::size_t>(count));

    for (int i=0;i<count;i++) {
      const int clientNum = msg.readByte();
      if (clientNum < 0 || clientNum >= opts_.maxClients) {
        // On skippe proprement si le serveur envoie un client hors borne
        skipUserCmd(msg);
        continue;
      }

      UserCmd& dst = usercmdBaseline_.byClient[static_cast<size_t>(clientNum)];
      readDelta_UserCmd(msg, dst);     // maj baseline[clientNum]
      out.push_back(dst);              // agrégation dans le snapshot courant
    }
  }

  void readDelta_UserCmd(MessageBuffer& msg, UserCmd& io) {
    // Schéma inspiré de MSG_ReadDeltaUsercmdKey :
    // - un bit par champ (changed?)
    // - payload selon type (byte/short/int)
    if (msg.readBit()) io.serverTime   = msg.readLong();
    if (msg.readBit()) io.angles[0]    = msg.readShort();
    if (msg.readBit()) io.angles[1]    = msg.readShort();
    if (msg.readBit()) io.angles[2]    = msg.readShort();
    if (msg.readBit()) io.forwardmove  = msg.readByte();
    if (msg.readBit()) io.rightmove    = msg.readByte();
    if (msg.readBit()) io.upmove       = msg.readByte();
    if (msg.readBit()) io.buttons      = static_cast<std::uint32_t>(msg.readLong());
    if (msg.readBit()) io.weapon       = msg.readByte();
    if (msg.readBit()) io.generic_cmd  = msg.readByte();
  }

  void skipUserCmd(MessageBuffer& msg) {
    // Consommation conservative : on lit le bitfield et on saute les payloads
    auto skipIf = [&](auto reader){ if (msg.readBit()) { (void)reader(); } };
    skipIf([&]{ return msg.readLong();  }); // serverTime
    skipIf([&]{ return msg.readShort(); }); // angles[0]
    skipIf([&]{ return msg.readShort(); }); // angles[1]
    skipIf([&]{ return msg.readShort(); }); // angles[2]
    skipIf([&]{ return msg.readByte();  }); // forwardmove
    skipIf([&]{ return msg.readByte();  }); // rightmove
    skipIf([&]{ return msg.readByte();  }); // upmove
    skipIf([&]{ return msg.readLong();  }); // buttons
    skipIf([&]{ return msg.readByte();  }); // weapon
    skipIf([&]{ return msg.readByte();  }); // generic_cmd
  }

  // ---------- NetField unknown / skip helper ----------

  static void skipUnknown(MessageBuffer& msg, const NetField& nf) {
    // Approche conservative : on consomme selon une taille “raisonnable”.
    // Idéalement, les NetField customs préciseront leurs tailles.
    switch (nf.type) {
      case NetFieldType::Int:   (void)msg.readLong();  break;
      case NetFieldType::Short: (void)msg.readShort(); break;
      case NetFieldType::Byte:  (void)msg.readByte();  break;
      case NetFieldType::Float: (void)msg.readFloat(); break;
      case NetFieldType::Vec3:  (void)msg.readFloat(); (void)msg.readFloat(); (void)msg.readFloat(); break;
      case NetFieldType::Vec3i: (void)msg.readLong();  (void)msg.readLong();  (void)msg.readLong();  break;
      default: break;
    }
  }
};

// ===============================
// Implémentations « par nom »
// (PlayerState / EntityState)
// ===============================
//
// NOTE IMPORTANTE :
// -----------------
// Les fonctions ci-dessous mappent les noms symboliques de NetField vers
// vos champs concrets (modernes) dans PlayerState / EntityState.
// Adaptez exactement aux champs de vos structs (origin -> origin_i, etc.)
// et aux types (Vec3i vs Vec3), en miroir de vos NetField tables DM_26.
//

// -------- PlayerState : baseline copy per-field
inline void SnapshotParser::applyBaseline_PlayerStateField(PlayerState& out,
                                                           const PlayerState& base,
                                                           const char* name) {
  // Exemple (à compléter selon votre PlayerState moderne & NetFields)
  if (std::strcmp(name,"pm_type")==0)          { out.pm_type = base.pm_type; return; }
  if (std::strcmp(name,"origin")==0)           { out.origin_i = base.origin_i; return; }
  if (std::strcmp(name,"velocity")==0)         { out.velocity_i = base.velocity_i; return; }
  if (std::strcmp(name,"viewangles")==0)       { out.viewangles_i = base.viewangles_i; return; }
  if (std::strcmp(name,"weapon")==0)           { out.weapon = base.weapon; return; }
  if (std::strcmp(name,"groundEntityNum")==0)  { out.groundEntityNum = base.groundEntityNum; return; }
  // ... listez tous les champs NetField DM_26 de playerstate
}

// -------- PlayerState : write per-field (overloads)

inline void SnapshotParser::writePlayerField(PlayerState& out, const char* name, int v) {
  if (std::strcmp(name,"pm_type")==0)         { out.pm_type = v; return; }
  if (std::strcmp(name,"weapon")==0)          { out.weapon  = v; return; }
  if (std::strcmp(name,"groundEntityNum")==0) { out.groundEntityNum = v; return; }
  // ...
}
inline void SnapshotParser::writePlayerField(PlayerState& out, const char* name, float v) {
  // rares dans PS, mais possible (timers, friction, etc. si float pur)
  // mappez si nécessaire
}
inline void SnapshotParser::writePlayerField(PlayerState& out, const char* name, Vec3 v) {
  if (std::strcmp(name,"viewangles_f")==0) { out.viewangles = v; return; } // si vous exposez une version float
  // ...
}
inline void SnapshotParser::writePlayerField(PlayerState& out, const char* name, Vec3i v) {
  if (std::strcmp(name,"origin")==0)     { out.origin_i     = v; return; }
  if (std::strcmp(name,"velocity")==0)   { out.velocity_i   = v; return; }
  if (std::strcmp(name,"viewangles")==0) { out.viewangles_i = v; return; }
  // ...
}

// -------- EntityState : baseline copy per-field

inline void SnapshotParser::applyBaseline_EntityField(EntityState& out,
                                                      const EntityState& base,
                                                      const char* name) {
  if (std::strcmp(name,"number")==0)         { out.number = base.number; return; }
  if (std::strcmp(name,"origin")==0)         { out.origin_i = base.origin_i; return; }
  if (std::strcmp(name,"angles")==0)         { out.angles_i = base.angles_i; return; }
  if (std::strcmp(name,"pos.trType")==0)     { out.pos.type = base.pos.type; return; }
  if (std::strcmp(name,"pos.trTime")==0)     { out.pos.startTime = base.pos.startTime; return; }
  if (std::strcmp(name,"pos.trDuration")==0) { out.pos.duration  = base.pos.duration;  return; }
  if (std::strcmp(name,"pos.trBase")==0)     { out.pos.base  = base.pos.base; return; }
  if (std::strcmp(name,"pos.trDelta")==0)    { out.pos.delta = base.pos.delta; return; }
  // ... complétez pour tous les champs NetField d’entityState DM_26
}

// -------- EntityState : write per-field (overloads)

inline void SnapshotParser::writeEntityField(EntityState& out, const char* name, int v) {
  if (std::strcmp(name,"number")==0) { out.number = v; return; }
  // mappez d'autres ints : eType, eFlags, modelindex, solid, time2, etc.
}
inline void SnapshotParser::writeEntityField(EntityState& out, const char* name, float v) {
  // s’il existe des champs float bruts dans EntityState moderne (rare), mappez-les ici
}
inline void SnapshotParser::writeEntityField(EntityState& out, const char* name, Vec3 v) {
  if (std::strcmp(name,"pos.trBase_f")==0)  { out.pos.base = v;  return; }
  if (std::strcmp(name,"pos.trDelta_f")==0) { out.pos.delta = v; return; }
}
inline void SnapshotParser::writeEntityField(EntityState& out, const char* name, Vec3i v) {
  if (std::strcmp(name,"origin")==0) { out.origin_i = v; return; }
  if (std::strcmp(name,"angles")==0) { out.angles_i = v; return; }
  // Si vos NetFields encodent la trajectoire en entiers (cas réseau), décideurs :
  if (std::strcmp(name,"pos.trBase")==0)  { out.pos.base = v.toFloat();  return; } // ex: si table encode vec3i
  if (std::strcmp(name,"pos.trDelta")==0) { out.pos.delta = v.toFloat(); return; }
}

} // namespace jka

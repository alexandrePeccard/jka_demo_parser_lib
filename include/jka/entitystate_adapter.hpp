#pragma once
// entitystate_adapter.hpp — Bridge entre legacy entityState_t (q_shared.h)
// et moderne jka::EntityState (entitystate.hpp avec Vec3T + Trajectory)

#include "entitystate.hpp"
#include "trajectory.hpp"
#include "q_shared.h"   // doit fournir entityState_t et trajectory_t

namespace jka {

    inline EntityState from_legacy(const entityState_t& in) {
        EntityState out;

        out.number      = in.number;
        out.eTypeRaw    = in.eType;
        out.eType       = static_cast<EntityType>(in.eType);

        out.eFlagsRaw   = in.eFlags;
        out.eFlags      = static_cast<EFlags>(in.eFlags);

        // Trajectoires (pos / apos)
        out.pos  = fromLegacy(in.pos);
        out.apos = fromLegacy(in.apos);

        // Times
        out.time  = in.time;
        out.time2 = in.time2;

        // Positions / angles (réseau → Vec3i)
        out.origin  = Vec3i(in.origin[0],  in.origin[1],  in.origin[2]);
        out.origin2 = Vec3i(in.origin2[0], in.origin2[1], in.origin2[2]);
        out.angles  = Vec3i(in.angles[0],  in.angles[1],  in.angles[2]);
        out.angles2 = Vec3i(in.angles2[0], in.angles2[1], in.angles2[2]);

        // Liens entités
        out.otherEntityNum  = in.otherEntityNum;
        out.otherEntityNum2 = in.otherEntityNum2;
        out.groundEntityNum = in.groundEntityNum;

        // Assets
        out.loopSound      = in.loopSound;
        out.constantLight  = in.constantLight;
        out.modelindex     = in.modelindex;
        out.modelindex2    = in.modelindex2;
        out.modelindex3    = in.modelindex3;
        out.modelindex4    = in.modelindex4;
        out.frame          = in.frame;
        out.solid          = in.solid;

        // Events & gameplay
        out.event_      = in.event;
        out.eventParm   = in.eventParm;
        out.powerups    = in.powerups;
        out.weapon      = in.weapon;
        out.legsAnim    = in.legsAnim;
        out.torsoAnim   = in.torsoAnim;
        out.generic1    = in.generic1;

    #if defined(CONSTANT_LIGHT_STYLES) // OpenJK extension
        for (int i=0; i<4; ++i)
            out.constantLightStyles[i] = in.constantLightStyles[i];
    #endif

        return out;
    }

    inline entityState_t to_legacy(const EntityState& in) {
        entityState_t out{};

        out.number  = in.number;
        out.eType   = in.eTypeRaw;
        out.eFlags  = in.eFlagsRaw;

        // Trajectoires
        out.pos  = toLegacy(in.pos);
        out.apos = toLegacy(in.apos);

        // Times
        out.time  = in.time;
        out.time2 = in.time2;

        // Positions / angles
        out.origin[0]  = in.origin.x;  out.origin[1]  = in.origin.y;  out.origin[2]  = in.origin.z;
        out.origin2[0] = in.origin2.x; out.origin2[1] = in.origin2.y; out.origin2[2] = in.origin2.z;
        out.angles[0]  = in.angles.x;  out.angles[1]  = in.angles.y;  out.angles[2]  = in.angles.z;
        out.angles2[0] = in.angles2.x; out.angles2[1] = in.angles2.y; out.angles2[2] = in.angles2.z;

        // Liens entités
        out.otherEntityNum  = in.otherEntityNum;
        out.otherEntityNum2 = in.otherEntityNum2;
        out.groundEntityNum = in.groundEntityNum;

        // Assets
        out.loopSound      = in.loopSound;
        out.constantLight  = in.constantLight;
        out.modelindex     = in.modelindex;
        out.modelindex2    = in.modelindex2;
        out.modelindex3    = in.modelindex3;
        out.modelindex4    = in.modelindex4;
        out.frame          = in.frame;
        out.solid          = in.solid;

        // Events & gameplay
        out.event       = in.event_;
        out.eventParm   = in.eventParm;
        out.powerups    = in.powerups;
        out.weapon      = in.weapon;
        out.legsAnim    = in.legsAnim;
        out.torsoAnim   = in.torsoAnim;
        out.generic1    = in.generic1;

    #if defined(CONSTANT_LIGHT_STYLES) // OpenJK extension
        for (int i=0; i<4; ++i)
            out.constantLightStyles[i] = in.constantLightStyles[i];
    #endif

        return out;
    }

} // namespace jka

#pragma once
// playerstate_adapter.hpp — Bridge between legacy playerState_t (q_shared.h)
// and modern jka::PlayerState (playerstate.hpp)

#include "playerstate.hpp"
#include "q_shared.h"   // must provide playerState_t

namespace jka {

    inline PlayerState from_legacy(const playerState_t& in) {
        PlayerState out;

        out.commandTime = in.commandTime;
        out.pm_type     = static_cast<PMType>(in.pm_type);

        for (int i = 0; i < 3; i++) {
            out.origin[i] = in.origin[i];
            out.velocity[i] = in.velocity[i];
        }

        out.pm_flags = in.pm_flags;
        out.gravity  = in.gravity;
        out.speed    = in.speed;
        out.delta_angles = { in.delta_angles[0], in.delta_angles[1], in.delta_angles[2] };

        out.groundEntityNum = in.groundEntityNum;
        out.legsTimer       = in.legsTimer;
        out.torsoTimer      = in.torsoTimer;
        out.legsAnim        = in.legsAnim;
        out.torsoAnim       = in.torsoAnim;

        out.movementDir     = in.movementDir;
        out.viewangles      = { in.viewangles[0], in.viewangles[1], in.viewangles[2] };

        out.viewheight      = in.viewheight;
        out.deltaTime       = in.deltaTime;
        out.damageEvent     = in.damageEvent;
        out.damageYaw       = in.damageYaw;
        out.damagePitch     = in.damagePitch;
        out.damageCount     = in.damageCount;

        out.stats.assign(in.stats, in.stats + MAX_STATS);
        out.persistant.assign(in.persistant, in.persistant + MAX_PERSISTANT);
        out.powerups.assign(in.powerups, in.powerups + MAX_POWERUPS);
        out.ammo.assign(in.ammo, in.ammo + MAX_WEAPONS);

        out.generic1 = in.generic1;
        out.loopSound = in.loopSound;
        out.jumppad_ent = in.jumppad_ent;

        out.pm_time = in.pm_time;
        out.eventSequence = in.eventSequence;
        for (int i=0; i<MAX_PS_EVENTS; ++i) {
            out.events[i] = in.events[i];
            out.eventParms[i] = in.eventParms[i];
        }

        out.externalEvent = in.externalEvent;
        out.externalEventParm = in.externalEventParm;
        out.clientNum = in.clientNum;
        out.weapon    = in.weapon;
        out.weaponstate = in.weaponstate;
        out.eFlags    = in.eFlags;

        out.speed = in.speed;

        out.gravity = in.gravity;
        out.groundEntityNum = in.groundEntityNum;

        return out;
    }

    inline playerState_t to_legacy(const PlayerState& in) {
        playerState_t out{};

        out.commandTime = in.commandTime;
        out.pm_type     = static_cast<int>(in.pm_type);

        for (int i = 0; i < 3; i++) {
            out.origin[i] = in.origin[i];
            out.velocity[i] = in.velocity[i];
        }

        out.pm_flags = in.pm_flags;
        out.gravity  = in.gravity;
        out.speed    = in.speed;
        out.delta_angles[0] = in.delta_angles[0];
        out.delta_angles[1] = in.delta_angles[1];
        out.delta_angles[2] = in.delta_angles[2];

        out.groundEntityNum = in.groundEntityNum;
        out.legsTimer       = in.legsTimer;
        out.torsoTimer      = in.torsoTimer;
        out.legsAnim        = in.legsAnim;
        out.torsoAnim       = in.torsoAnim;

        out.movementDir     = in.movementDir;
        out.viewangles[0]   = in.viewangles[0];
        out.viewangles[1]   = in.viewangles[1];
        out.viewangles[2]   = in.viewangles[2];

        out.viewheight      = in.viewheight;
        out.deltaTime       = in.deltaTime;
        out.damageEvent     = in.damageEvent;
        out.damageYaw       = in.damageYaw;
        out.damagePitch     = in.damagePitch;
        out.damageCount     = in.damageCount;

        std::copy(in.stats.begin(), in.stats.end(), out.stats);
        std::copy(in.persistant.begin(), in.persistant.end(), out.persistant);
        std::copy(in.powerups.begin(), in.powerups.end(), out.powerups);
        std::copy(in.ammo.begin(), in.ammo.end(), out.ammo);

        out.generic1 = in.generic1;
        out.loopSound = in.loopSound;
        out.jumppad_ent = in.jumppad_ent;

        out.pm_time = in.pm_time;
        out.eventSequence = in.eventSequence;
        for (int i=0; i<MAX_PS_EVENTS; ++i) {
            out.events[i] = in.events[i];
            out.eventParms[i] = in.eventParms[i];
        }

        out.externalEvent = in.externalEvent;
        out.externalEventParm = in.externalEventParm;
        out.clientNum = in.clientNum;
        out.weapon    = in.weapon;
        out.weaponstate = in.weaponstate;
        out.eFlags    = in.eFlags;

        out.speed = in.speed;
        out.gravity = in.gravity;
        out.groundEntityNum = in.groundEntityNum;

        return out;
    }
} // namespace jka

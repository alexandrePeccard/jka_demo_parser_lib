#pragma once
#include "usercmd.hpp"
#include "q_shared.h"   // pour struct usercmd_t legacy

namespace jka {

/// Conversion usercmd_t (C legacy) → UserCommand (C++ moderne)
inline UserCommand fromLegacy(const usercmd_t& legacy) {
    UserCommand cmd;
    cmd.serverTime   = legacy.serverTime;
    cmd.angles       = Vec3T<int32_t>{ legacy.angles[0], legacy.angles[1], legacy.angles[2] };
    cmd.forwardmove  = legacy.forwardmove;
    cmd.rightmove    = legacy.rightmove;
    cmd.upmove       = legacy.upmove;
    cmd.weapon       = legacy.weapon;
    cmd.buttons      = legacy.buttons;
    return cmd;
}

/// Conversion UserCommand (C++ moderne) → usercmd_t (C legacy)
inline usercmd_t toLegacy(const UserCommand& cmd) {
    usercmd_t legacy{};
    legacy.serverTime = cmd.serverTime;
    legacy.angles[0]  = cmd.angles.x;
    legacy.angles[1]  = cmd.angles.y;
    legacy.angles[2]  = cmd.angles.z;
    legacy.forwardmove = cmd.forwardmove;
    legacy.rightmove   = cmd.rightmove;
    legacy.upmove      = cmd.upmove;
    legacy.weapon      = cmd.weapon;
    legacy.buttons     = cmd.buttons;
    return legacy;
}

} // namespace jka

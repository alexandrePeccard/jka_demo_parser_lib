#pragma once
// jka/trajectory.hpp — Trajectory (moderne) + adaptateurs vers C (q_shared.h)

#include <cstdint>
#include <limits>
#include <algorithm>
#include "Vec3.hpp"
#include "trajectory_types.hpp"

namespace jka {

// ---- Types de base ---------------------------------------------------------

using MilliSeconds = int; // cohérent avec q_shared.h

// ---- Trajectory (moderne) --------------------------------------------------

struct Trajectory {
    TrajectoryType type {TrajectoryType::Stationary};
    MilliSeconds   startTime {0};       // équiv. trTime
    MilliSeconds   duration  {0};       // équiv. trDuration (0 => infini)
    Vec3           base {0.f,0.f,0.f};  // équiv. trBase
    Vec3           delta{0.f,0.f,0.f};  // équiv. trDelta (vitesse, amplitude, cible*, etc.)

    // Helpers temporels
    constexpr bool hasFiniteEnd() const noexcept { return duration > 0; }
    constexpr MilliSeconds endTime() const noexcept { 
        return hasFiniteEnd() ? (startTime + duration) 
                              : std::numeric_limits<MilliSeconds>::max(); 
    }
    constexpr bool isOver(MilliSeconds t) const noexcept { 
        return hasFiniteEnd() && t >= endTime(); 
    }
    constexpr float phase01(MilliSeconds t) const noexcept {
        if (!hasFiniteEnd()) return 0.f;
        const float num = static_cast<float>(t - startTime);
        const float den = static_cast<float>(duration);
        return std::clamp(num / std::max(den, 1.f), 0.f, 1.f);
    }

    // Wrappers pratiques (branchés sur TrajectoryEvaluator.hpp)
    Vec3 positionAt(MilliSeconds t, float gravity = 800.f) const;
    Vec3 velocityAt(MilliSeconds t, float gravity = 800.f) const;

    // Egalité structurale
    friend bool operator==(const Trajectory& a, const Trajectory& b) {
        return a.type == b.type && a.startTime == b.startTime && a.duration == b.duration &&
               a.base == b.base && a.delta == b.delta;
    }
    friend bool operator!=(const Trajectory& a, const Trajectory& b) { return !(a==b); }
};

// ---- Conversion enum (constexpr) -------------------------------------------

constexpr inline TrajectoryType toModernEnum(int legacy_trType) {
    switch (legacy_trType) {
        case 0: return TrajectoryType::Stationary;
        case 1: return TrajectoryType::Interpolate;
        case 2: return TrajectoryType::Linear;
        case 3: return TrajectoryType::LinearStop;
        case 4: return TrajectoryType::NonLinearStop; // extension JKA
        case 5: return TrajectoryType::Sine;
        case 6: return TrajectoryType::Gravity;
        default: return TrajectoryType::Stationary;
    }
}

constexpr inline int toLegacyEnum(TrajectoryType t) {
    switch (t) {
        case TrajectoryType::Stationary:   return 0;
        case TrajectoryType::Interpolate:  return 1;
        case TrajectoryType::Linear:       return 2;
        case TrajectoryType::LinearStop:   return 3;
        case TrajectoryType::NonLinearStop:return 4;
        case TrajectoryType::Sine:         return 5;
        case TrajectoryType::Gravity:      return 6;
    }
    return 0;
}

// ---- Adaptateurs ⇆ C (q_shared.h) ------------------------------------------

#ifdef JKA_HAVE_Q_SHARED_H
    extern "C" {
      #include "q_shared.h"  // doit fournir trType_t, trajectory_t, vec3_t
    }

    inline Trajectory fromLegacy(const trajectory_t& in) {
        Trajectory out;
        out.type      = toModernEnum(static_cast<int>(in.trType));
        out.startTime = in.trTime;
        out.duration  = in.trDuration;
        out.base      = Vec3{ in.trBase[0],  in.trBase[1],  in.trBase[2]  };
        out.delta     = Vec3{ in.trDelta[0], in.trDelta[1], in.trDelta[2] };
        return out;
    }

    inline trajectory_t toLegacy(const Trajectory& in) {
        trajectory_t out{};
        out.trType     = static_cast<trType_t>(toLegacyEnum(in.type));
        out.trTime     = in.startTime;
        out.trDuration = in.duration;
        out.trBase[0]  = in.base.x;  out.trBase[1]  = in.base.y;  out.trBase[2]  = in.base.z;
        out.trDelta[0] = in.delta.x; out.trDelta[1] = in.delta.y; out.trDelta[2] = in.delta.z;
        return out;
    }
    #endif // JKA_HAVE_Q_SHARED_H

} // namespace jka

// Implémentations inline des wrappers pratiques
#include "TrajectoryEvaluator.hpp"

namespace jka {
    inline Vec3 Trajectory::positionAt(MilliSeconds t, float gravity) const {
        return EvaluateTrajectory(*this, t, gravity);
    }
    inline Vec3 Trajectory::velocityAt(MilliSeconds t, float gravity) const {
        return EvaluateTrajectoryDelta(*this, t, gravity);
    }
} // namespace jka
namespace std {
    template<>
    struct hash<jka::Trajectory> {
        std::size_t operator()(const jka::Trajectory& t) const noexcept {
            std::size_t h = std::hash<int>{}(static_cast<int>(t.type));
            h ^= std::hash<int>{}(t.startTime) + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
            h ^= std::hash<int>{}(t.duration)  + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
            h ^= std::hash<jka::Vec3>{}(t.base) + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
            h ^= std::hash<jka::Vec3>{}(t.delta)+ 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
            return h;
        }
    };
}

#pragma once
#include <cstdint>

namespace jka {

    /// Trajectoire moderne unifiée (superset, utilisée dans Trajectory)
    /// => On se base sur le protocole le plus riche (OpenJK DM_26).
    enum class TrajectoryType : std::uint8_t {
        Stationary = 0,    ///< TR_STATIONARY : pas de mouvement
        Interpolate,       ///< TR_INTERPOLATE : interpole entre base et delta
        Linear,            ///< TR_LINEAR : vitesse constante
        LinearStop,        ///< TR_LINEAR_STOP : linéaire avec arrêt à t0+duration
        NonLinearStop,     ///< TR_NONLINEAR_STOP : easing (JKA spécifique)
        Sine,              ///< TR_SINE : oscillation sinus
        Gravity            ///< TR_GRAVITY : mouvement avec gravité (z-)
    };

    /// Types de trajectoires OpenJK (DM_26)
    enum class TrajectoryTypeJKA : std::uint8_t {
        STATIONARY     = 0,
        INTERPOLATE    = 1,
        LINEAR         = 2,
        LINEAR_STOP    = 3,
        NONLINEAR_STOP = 4,
        SINE           = 5,
        GRAVITY        = 6
    };

    /// Types de trajectoires Quake3/ioquake3 (DM_25)
    enum class TrajectoryTypeQ3 : std::uint8_t {
        STATIONARY   = 0,
        INTERPOLATE  = 1,
        LINEAR       = 2,
        SINE         = 3,
        GRAVITY      = 4
    };

    // -----------------------------------------------------------------------------
    // Conversion helpers
    // -----------------------------------------------------------------------------

    /// Conversion DM_26 → moderne
    constexpr TrajectoryType fromJKA(TrajectoryTypeJKA t) {
        switch (t) {
            case TrajectoryTypeJKA::STATIONARY:     return TrajectoryType::Stationary;
            case TrajectoryTypeJKA::INTERPOLATE:    return TrajectoryType::Interpolate;
            case TrajectoryTypeJKA::LINEAR:         return TrajectoryType::Linear;
            case TrajectoryTypeJKA::LINEAR_STOP:    return TrajectoryType::LinearStop;
            case TrajectoryTypeJKA::NONLINEAR_STOP: return TrajectoryType::NonLinearStop;
            case TrajectoryTypeJKA::SINE:           return TrajectoryType::Sine;
            case TrajectoryTypeJKA::GRAVITY:        return TrajectoryType::Gravity;
        }
        return TrajectoryType::Stationary;
    }

    /// Conversion moderne → DM_26
    constexpr TrajectoryTypeJKA toJKA(TrajectoryType t) {
        switch (t) {
            case TrajectoryType::Stationary:   return TrajectoryTypeJKA::STATIONARY;
            case TrajectoryType::Interpolate:  return TrajectoryTypeJKA::INTERPOLATE;
            case TrajectoryType::Linear:       return TrajectoryTypeJKA::LINEAR;
            case TrajectoryType::LinearStop:   return TrajectoryTypeJKA::LINEAR_STOP;
            case TrajectoryType::NonLinearStop:return TrajectoryTypeJKA::NONLINEAR_STOP;
            case TrajectoryType::Sine:         return TrajectoryTypeJKA::SINE;
            case TrajectoryType::Gravity:      return TrajectoryTypeJKA::GRAVITY;
        }
        return TrajectoryTypeJKA::STATIONARY;
    }

    /// Conversion DM_25 → moderne
    constexpr TrajectoryType fromQ3(TrajectoryTypeQ3 t) {
        switch (t) {
            case TrajectoryTypeQ3::STATIONARY:  return TrajectoryType::Stationary;
            case TrajectoryTypeQ3::INTERPOLATE: return TrajectoryType::Interpolate;
            case TrajectoryTypeQ3::LINEAR:      return TrajectoryType::Linear;
            case TrajectoryTypeQ3::SINE:        return TrajectoryType::Sine;
            case TrajectoryTypeQ3::GRAVITY:     return TrajectoryType::Gravity;
        }
        return TrajectoryType::Stationary;
    }

    /// Conversion moderne → DM_25 (approx : les types non existants deviennent STATIONARY ou LINEAR)
    constexpr TrajectoryTypeQ3 toQ3(TrajectoryType t) {
        switch (t) {
            case TrajectoryType::Stationary:   return TrajectoryTypeQ3::STATIONARY;
            case TrajectoryType::Interpolate:  return TrajectoryTypeQ3::INTERPOLATE;
            case TrajectoryType::Linear:       return TrajectoryTypeQ3::LINEAR;
            case TrajectoryType::LinearStop:   return TrajectoryTypeQ3::LINEAR; // fallback
            case TrajectoryType::NonLinearStop:return TrajectoryTypeQ3::LINEAR; // fallback
            case TrajectoryType::Sine:         return TrajectoryTypeQ3::SINE;
            case TrajectoryType::Gravity:      return TrajectoryTypeQ3::GRAVITY;
        }
        return TrajectoryTypeQ3::STATIONARY;
    }
}
#pragma once
// jka/TrajectoryEvaluator.hpp — logique BG_EvaluateTrajectory* modernisée

#include <cmath>
#include "trajectory.hpp"
#include "Vec3.hpp"

namespace jka {

    /// Accélération de gravité par défaut dans JKA (unités/s²)
    constexpr float DefaultGravity = 800.0f;

    /// Évalue la position de la trajectoire au temps `atTime`
    /// (équivalent BG_EvaluateTrajectory)
    inline Vec3 EvaluateTrajectory(const Trajectory& tr, MilliSeconds atTime, float gravity = DefaultGravity) {
        const float deltaTimeSec = static_cast<float>(atTime - tr.startTime) / 1000.0f;

        switch (tr.type) {
            case TrajectoryType::Stationary:
            default:
                return tr.base;

            case TrajectoryType::Interpolate: {
                // interpolation linéaire base → delta (interpreted as endpoint)
                float frac = tr.phase01(atTime);
                return tr.base + (tr.delta - tr.base) * frac;
            }

            case TrajectoryType::Linear:
                return tr.base + tr.delta * deltaTimeSec;

            case TrajectoryType::LinearStop: {
                if (tr.duration > 0 && atTime > tr.startTime + tr.duration) {
                    float tSec = static_cast<float>(tr.duration) / 1000.0f;
                    return tr.base + tr.delta * tSec;
                }
                return tr.base + tr.delta * deltaTimeSec;
            }

            case TrajectoryType::Sine: {
                float phase = std::sin((deltaTimeSec / (tr.duration / 1000.0f)) * 2.0f * static_cast<float>(M_PI));
                return tr.base + tr.delta * phase;
            }

            case TrajectoryType::Gravity: {
                Vec3 result = tr.base + tr.delta * deltaTimeSec;
                result.z -= 0.5f * gravity * deltaTimeSec * deltaTimeSec;
                return result;
            }

            case TrajectoryType::NonLinearStop: {
                // OpenJK: easing non-linéaire sur la vitesse (rarement utilisé)
                if (tr.duration > 0 && atTime > tr.startTime + tr.duration) {
                    return tr.base + tr.delta * (static_cast<float>(tr.duration) / 1000.0f);
                }
                float frac = tr.phase01(atTime);
                float eased = 1.0f - (1.0f - frac) * (1.0f - frac); // quadratic ease-out
                float totalT = static_cast<float>(tr.duration) / 1000.0f;
                return tr.base + tr.delta * (totalT * eased);
            }
        }
    }

    /// Évalue la vitesse instantanée de la trajectoire au temps `atTime`
    /// (équivalent BG_EvaluateTrajectoryDelta)
    inline Vec3 EvaluateTrajectoryDelta(const Trajectory& tr, MilliSeconds atTime, float gravity = DefaultGravity) {
        const float deltaTimeSec = static_cast<float>(atTime - tr.startTime) / 1000.0f;

        switch (tr.type) {
            case TrajectoryType::Stationary:
            default:
                return Vec3{0.f,0.f,0.f};

            case TrajectoryType::Interpolate: {
                if (tr.duration <= 0) return Vec3{};
                Vec3 total = tr.delta - tr.base;
                float v = 1.0f / (tr.duration / 1000.0f); // [1/s]
                return total * v;
            }

            case TrajectoryType::Linear:
                return tr.delta;

            case TrajectoryType::LinearStop:
                if (tr.duration > 0 && atTime > tr.startTime + tr.duration)
                    return Vec3{};
                return tr.delta;

            case TrajectoryType::Sine: {
                if (tr.duration <= 0) return Vec3{};
                float w = 2.0f * static_cast<float>(M_PI) / (tr.duration / 1000.0f); // rad/s
                float cosv = std::cos(deltaTimeSec * w);
                return tr.delta * (w * cosv);
            }

            case TrajectoryType::Gravity: {
                Vec3 vel = tr.delta;
                vel.z -= gravity * deltaTimeSec;
                return vel;
            }

            case TrajectoryType::NonLinearStop: {
                if (tr.duration <= 0) return tr.delta;
                float frac = tr.phase01(atTime);
                float dfdts = 1.0f / (static_cast<float>(tr.duration)); // per ms
                float dfdts_sec = dfdts * 1000.0f; // -> per s
                float deased = 2.f * (1.f - frac) * dfdts_sec;
                float totalT = static_cast<float>(tr.duration) / 1000.0f;
                return tr.delta * (totalT * deased);
            }
        }
    }
}

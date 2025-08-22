#pragma once
// jka/Vec3.hpp — Vecteur 3D modulaire réseau⇆moteur
//
// - Vec3T<T> : gabarit commun (T = float pour moteur, T = int32_t pour réseau)
// - Vec3     : alias de Vec3T<float>
// - Vec3i    : alias de Vec3T<std::int32_t>
//
// Objectifs :
//  * Séparer clairement les données réseau (entiers) des données moteur (floats)
//  * Offrir une API identique (opérateurs + helpers) quel que soit T
//  * Conversions explicites via scaled(scale) documentées
//
// Options :
//  * Définir JKA_HAVE_Q_SHARED_H avant l’inclusion pour activer les adaptateurs q_shared.h
//  * Définir JKA_VEC3_JSON pour activer (nlohmann::json) les (de)serializers

#include <cstdint>
#include <cmath>
#include <type_traits>
#include <functional>
#include <utility>
#include <limits>
#include <ostream>

namespace jka {

// ------------------------- Détection flottant/entier -------------------------

namespace detail {
template <class T>
using is_float = std::is_floating_point<T>;
template <class T>
using is_int = std::bool_constant<std::is_integral<T>::value && !std::is_same<T, bool>::value>;
} // namespace detail

// -------------------------------- Vec3T<T> -----------------------------------

template <class T>
struct Vec3T {
  static_assert(std::is_arithmetic<T>::value, "Vec3T<T> requires arithmetic T");

  T x{}, y{}, z{};

  // ---- Ctors ---------------------------------------------------------------
  constexpr Vec3T() = default;
  constexpr Vec3T(T X, T Y, T Z) : x(X), y(Y), z(Z) {}

  template <class U, typename = std::enable_if_t<std::is_arithmetic<U>::value>>
  constexpr explicit Vec3T(const Vec3T<U>& o)
  : x(static_cast<T>(o.x)), y(static_cast<T>(o.y)), z(static_cast<T>(o.z)) {}

  // ---- Accès indexé --------------------------------------------------------
  constexpr T& operator[](std::size_t i)       { return i==0?x:(i==1?y:z); }
  constexpr const T& operator[](std::size_t i) const { return i==0?x:(i==1?y:z); }

  // ---- Egalité -------------------------------------------------------------
  friend constexpr bool operator==(const Vec3T& a, const Vec3T& b) {
    return a.x==b.x && a.y==b.y && a.z==b.z;
  }
  friend constexpr bool operator!=(const Vec3T& a, const Vec3T& b) { return !(a==b); }

  // ---- Arithmétique élémentaire -------------------------------------------
  friend constexpr Vec3T operator+(const Vec3T& a, const Vec3T& b) { return {T(a.x+b.x), T(a.y+b.y), T(a.z+b.z)}; }
  friend constexpr Vec3T operator-(const Vec3T& a, const Vec3T& b) { return {T(a.x-b.x), T(a.y-b.y), T(a.z-b.z)}; }
  friend constexpr Vec3T operator-(const Vec3T& v) { return {T(-v.x), T(-v.y), T(-v.z)}; }

  friend constexpr Vec3T operator*(const Vec3T& a, T s) { return {T(a.x*s), T(a.y*s), T(a.z*s)}; }
  friend constexpr Vec3T operator*(T s, const Vec3T& a) { return a*s; }

  // Division scalaire : prudence si T entier
  friend constexpr Vec3T operator/(const Vec3T& a, T s) {
    return {T(a.x/s), T(a.y/s), T(a.z/s)};
  }

  Vec3T& operator+=(const Vec3T& b) { x+=b.x; y+=b.y; z+=b.z; return *this; }
  Vec3T& operator-=(const Vec3T& b) { x-=b.x; y-=b.y; z-=b.z; return *this; }
  Vec3T& operator*=(T s)            { x*=s;   y*=s;   z*=s;   return *this; }
  Vec3T& operator/=(T s)            { x/=s;   y/=s;   z/=s;   return *this; }

  // ---- Produits/Normes -----------------------------------------------------
  static constexpr T dot(const Vec3T& a, const Vec3T& b) { return T(a.x*b.x + a.y*b.y + a.z*b.z); }

  static constexpr Vec3T cross(const Vec3T& a, const Vec3T& b) {
    // Note: si T est entier, cette op reste entière ; pour une version flottante, caster au préalable
    return Vec3T{
      T(a.y*b.z - a.z*b.y),
      T(a.z*b.x - a.x*b.z),
      T(a.x*b.y - a.y*b.x)
    };
  }

  // Norme au carré : sûre pour T entier
  static constexpr auto lengthSquared(const Vec3T& v)
    -> std::conditional_t<detail::is_int<T>::value, std::int64_t, T>
  {
    using RT = std::conditional_t<detail::is_int<T>::value, std::int64_t, T>;
    return RT(v.x)*RT(v.x) + RT(v.y)*RT(v.y) + RT(v.z)*RT(v.z);
  }

  // Norme (si T entier, renvoie double)
  static inline auto length(const Vec3T& v)
    -> std::conditional_t<detail::is_int<T>::value, double, T>
  {
    using RT = std::conditional_t<detail::is_int<T>::value, double, T>;
    return RT(std::sqrt(static_cast<RT>(lengthSquared(v))));
  }

  // Normalisation (renvoie Vec3T<U> pour permettre int->float)
  template <class U = T>
  static inline Vec3T<U> normalized(const Vec3T& v) {
    using RT = std::conditional_t<detail::is_int<T>::value, double, T>;
    const RT len = length(v);
    if (len <= RT(0)) {
      return Vec3T<U>{U(0),U(0),U(0)};
    }
    return Vec3T<U>{ static_cast<U>(v.x/len), static_cast<U>(v.y/len), static_cast<U>(v.z/len) };
  }

  // ---- Interpolation -------------------------------------------------------
  template <class U=T, typename = std::enable_if_t<detail::is_float<U>::value>>
  static constexpr Vec3T lerp(const Vec3T& a, const Vec3T& b, U t) {
    return a + (b - a) * t;
  }

  // ---- Clamp / min / max ---------------------------------------------------
  template <class U=T>
  static constexpr Vec3T clamp(const Vec3T& v, const Vec3T& lo, const Vec3T& hi) {
    auto c = [](U x, U l, U h) { return (x<l?l:(x>h?h:x)); };
    return Vec3T{ c(v.x, lo.x, hi.x), c(v.y, lo.y, hi.y), c(v.z, lo.z, hi.z) };
  }

  // ---- Conversions réseau ⇆ moteur ----------------------------------------
  // scaled<float>(1/8.f) : passe d’entiers réseau (1/8 unité) à floats moteur
  // scaled<int>(8)       : passe de floats moteur à entiers réseau
  template <class U, typename = std::enable_if_t<std::is_arithmetic<U>::value>>
  constexpr Vec3T<U> scaled(double scale) const {
    return Vec3T<U>{
      static_cast<U>(x * scale),
      static_cast<U>(y * scale),
      static_cast<U>(z * scale)
    };
  }

  // ---- Helpers de « quasi-égalité » (pour floats) --------------------------
  template <class U=T, typename = std::enable_if_t<detail::is_float<U>::value>>
  static inline bool nearlyEquals(const Vec3T& a, const Vec3T& b, U eps = U(1e-5)) {
    return std::fabs(a.x-b.x) <= eps &&
           std::fabs(a.y-b.y) <= eps &&
           std::fabs(a.z-b.z) <= eps;
  }
};

// ---- Alias standards : moteur (float) et réseau (int32) ---------------------
using Vec3  = Vec3T<float>;
using Vec3i = Vec3T<std::int32_t>;

// ---- IO Streams (debug) -----------------------------------------------------
template <class T>
inline std::ostream& operator<<(std::ostream& os, const Vec3T<T>& v) {
  return (os << '(' << v.x << ',' << v.y << ',' << v.z << ')');
}

// ---- Hash support (pour unordered_map/set) ----------------------------------
} // namespace jka

namespace std {
template <class T>
struct hash<jka::Vec3T<T>> {
  size_t operator()(const jka::Vec3T<T>& v) const noexcept {
    // hash combine simple
    auto h1 = std::hash<T>{}(v.x);
    auto h2 = std::hash<T>{}(v.y);
    auto h3 = std::hash<T>{}(v.z);
    size_t h = h1;
    h ^= h2 + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
    h ^= h3 + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
    return h;
  }
};
} // namespace std

// ----------------------------- Adaptateurs legacy ----------------------------
// Activez-les en définissant JKA_HAVE_Q_SHARED_H avant l’inclusion.

#ifdef JKA_HAVE_Q_SHARED_H
extern "C" {
  #include "q_shared.h" // doit fournir vec3_t = float[3]
}

namespace jka {

inline Vec3 fromLegacy(const vec3_t in) {
  return Vec3{ in[0], in[1], in[2] };
}

inline void toLegacy(const Vec3& v, vec3_t out) {
  out[0] = v.x; out[1] = v.y; out[2] = v.z;
}

} // namespace jka
#endif // JKA_HAVE_Q_SHARED_H

// ----------------------------- Sérialisation JSON ----------------------------
// Activez-les en définissant JKA_VEC3_JSON (et en ayant nlohmann/json.hpp)

#ifdef JKA_VEC3_JSON
  #include <nlohmann/json.hpp>
  namespace nlohmann {
    template <class T>
    struct adl_serializer<jka::Vec3T<T>> {
      static void to_json(json& j, const jka::Vec3T<T>& v) {
        j = json{ v.x, v.y, v.z };
      }
      static void from_json(const json& j, jka::Vec3T<T>& v) {
        v.x = j.at(0).get<T>();
        v.y = j.at(1).get<T>();
        v.z = j.at(2).get<T>();
      }
    };
  } // namespace nlohmann
#endif // JKA_VEC3_JSON
  
namespace std {
    template<typename T>
    struct hash<jka::Vec3T<T>> {
        std::size_t operator()(const jka::Vec3T<T>& v) const noexcept {
            std::size_t h1 = std::hash<T>{}(v.x);
            std::size_t h2 = std::hash<T>{}(v.y);
            std::size_t h3 = std::hash<T>{}(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

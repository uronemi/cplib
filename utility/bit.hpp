#pragma once
#if (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(_M_X64))
  #pragma GCC target("avx2,bmi,bmi2,lzcnt,popcnt")
#endif

namespace internal {

#if defined(__GNUC__) || defined(__clang__)
  #define ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
  #define ALWAYS_INLINE __forceinline
#else
  #define ALWAYS_INLINE inline
#endif

ALWAYS_INLINE constexpr int popcount(std::uint32_t x) noexcept {
  return __builtin_popcount(x);
}

ALWAYS_INLINE constexpr int popcount(std::uint64_t x) noexcept {
  return __builtin_popcountll(x);
}

ALWAYS_INLINE constexpr int popcount(__uint128_t x) noexcept {
  return __builtin_popcountll(static_cast<std::uint64_t>(x)) +
         __builtin_popcountll(static_cast<std::uint64_t>(x >> 64));
}

template <typename T>
ALWAYS_INLINE constexpr int popcount(T x) noexcept {
  return popcount(static_cast<std::make_unsigned_t<T>>(x));
}

ALWAYS_INLINE constexpr int ctz(std::uint32_t x) noexcept {
  return x ? __builtin_ctz(x) : 32;
}

ALWAYS_INLINE constexpr int ctz(std::uint64_t x) noexcept {
  return x ? __builtin_ctzll(x) : 64;
}

ALWAYS_INLINE constexpr int ctz(__uint128_t x) noexcept {
  std::uint64_t l = static_cast<std::uint64_t>(x);
  if (l) return __builtin_ctzll(l);
  std::uint64_t h = static_cast<std::uint64_t>(x >> 64);
  return h ? 64 + __builtin_ctzll(h) : 128;
}

template <typename T>
ALWAYS_INLINE constexpr int ctz(T x) noexcept {
  return ctz(static_cast<std::make_unsigned_t<T>>(x));
}

ALWAYS_INLINE constexpr int clz(std::uint32_t x) noexcept {
  return x ? __builtin_clz(x) : 32;
}

ALWAYS_INLINE constexpr int clz(std::uint64_t x) noexcept {
  return x ? __builtin_clzll(x) : 64;
}

ALWAYS_INLINE constexpr int clz(__uint128_t x) noexcept {
  std::uint64_t h = static_cast<std::uint64_t>(x >> 64);
  if (h) return __builtin_clzll(h);
  std::uint64_t l = static_cast<std::uint64_t>(x);
  return l ? 64 + __builtin_clzll(l) : 128;
}

template <typename T>
ALWAYS_INLINE constexpr int clz(T x) noexcept {
  return clz(static_cast<std::make_unsigned_t<T>>(x));
}

template <typename T>
ALWAYS_INLINE constexpr int msb(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  U u = static_cast<U>(x);
  return u ? static_cast<int>(sizeof(U) * 8 - 1 - clz(u)) : -1;
}

template <typename T>
ALWAYS_INLINE constexpr int lsb(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  U u = static_cast<U>(x);
  return u ? ctz(u) : -1;
}

template <typename T>
ALWAYS_INLINE constexpr T lsb_val(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  return static_cast<T>(static_cast<U>(x) & -static_cast<U>(x));
}

template <typename T>
ALWAYS_INLINE constexpr T bit_floor(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  U u = static_cast<U>(x);
  return u ? (U(1) << msb(u)) : U(0);
}

template <typename T>
ALWAYS_INLINE constexpr T bit_ceil(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  U u = static_cast<U>(x);
  if (u <= 1) return U(1);
  constexpr int bw = sizeof(U) * 8;
  return U(1) << (bw - clz(static_cast<U>(u - 1)));
}

template <typename T>
ALWAYS_INLINE constexpr int ceil_log2(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  U u = static_cast<U>(x);
  return u <= 1 ? 0 : std::bit_width(static_cast<U>(u - 1));
}

#undef ALWAYS_INLINE

};
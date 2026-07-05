#pragma once
#include "../utility/XorShift.hpp"

// Rolling Hash
//
// 文字列とハッシュ値を対応付けていろいろ計算する
class RollingHash {
  private:
  using u64 = unsigned long long;
  using i128 = __int128_t;
  static constexpr const u64 MOD = (1uL << 61) - 1;
  inline static const u64 base = xor_shift() % (MOD - 2) + 2;
  static constexpr const u64 mask30 = (1uL << 30) - 1, mask31 = (1uL << 31) - 1;

  std::string S;
  std::vector<u64> hash_, p;

  u64 mul(i128 a, i128 b) const {
    i128 t = a * b;
    t = (t >> 61) + (t & MOD);
    return t >= MOD ? t - MOD : t;
  }

  unsigned int shift(unsigned int x) const {
    return x ^= x << 13, x ^= x >> 17, x ^= x << 5;
  }

  public:
  RollingHash() : S() {}

  // 文字列 s から Rolling Hash を構築する
  // O(s) time
  explicit RollingHash(const std::string& s) : S(s), hash_(s.size() + 1, 0), p(s.size() + 1, 1) {
    for (std::size_t i = 0; i < s.size(); ++i) {
      hash_[i + 1] = mul(hash_[i], base) + shift(s[i] + 1);
      p[i + 1] = mul(p[i], base);
      if (hash_[i + 1] >= MOD) hash_[i + 1] -= MOD;
    }
  }

  // 文字列全体のハッシュ値を返す
  // O(1) time
  u64 hash() const { return hash_.back(); }

  // S[l, r) のハッシュ値を返す
  // - l <= r <= |S|
  // O(1) time
  u64 hash(std::size_t l, std::size_t r) const {
    assert(l <= r && r <= S.size());
    u64 res = MOD + hash_[r] - mul(hash_[l], p[r - l]);
    return res >= MOD ? res - MOD : res;
  }
};
#pragma once
#include "../utility/XorShift.hpp"

// Dynamic Rolling Hash
//
// 動的な Rolling Hash
class DynamicRollingHash {
  private:
  using u64 = unsigned long long;
  using i128 = __int128_t;
  using size_type = std::size_t;
  static constexpr const u64 MOD = (1uL << 61) - 1;
  inline static const u64 base = xor_shift() % (MOD - 2) + 2;
  static constexpr const u64 mask30 = (1uL << 30) - 1, mask31 = (1uL << 31) - 1;

  std::string S;
  std::vector<u64> dat, p;
  std::vector<size_type> len;
  size_type sz;

  u64 add(u64 a, u64 b) const { return a + b >= MOD ? a + b - MOD : a + b; }

  u64 mul(i128 a, i128 b) const {
    i128 t = a * b;
    t = (t >> 61) + (t & MOD);
    return t >= MOD ? t - MOD : t;
  }

  unsigned int shift(unsigned int x) const { return x ^= x << 13, x ^= x >> 17, x ^= x << 5; }

  void pushup(size_type i) {
    len[i] = len[i << 1] + len[i << 1 | 1];
    dat[i] = add(dat[i << 1], mul(dat[i << 1 | 1], p[len[i << 1]]));
  }

  public:
  DynamicRollingHash() {}

  // 文字列 S で Rolling Hash を構築
  // O(|S|) time
  explicit DynamicRollingHash(const std::string& s) : S(s), p(s.size() + 1, 1) {
    sz = std::bit_ceil(s.size());
    dat.assign(sz * 2, 0);
    len.assign(sz * 2, 0);

    for (size_type i = 0; i < s.size(); ++i) {
      dat[sz + i] = shift(s[i] + 1);
      p[i + 1] = mul(p[i], base);
      len[sz + i] = 1;
    }
    for (size_type i = sz - 1; i >= 1; --i) pushup(i);
  }

  // 指定した位置の文字を変更
  // - idx < |S|
  // O(log|S|) time
  void set(size_type idx, char c) {
    assert(idx < S.size());
    S[idx] = c;
    idx += sz;
    dat[idx] = shift(c + 1);
    while (idx > 1) {
      idx >>= 1;
      pushup(idx);
    }
  }

  // S[l, r) のハッシュ値を返す
  // - l <= r <= |S|
  // O(log|S|) time
  u64 hash(size_type l, size_type r) const {
    assert(l <= r && r <= S.size());
    u64 resl = 0, resr = 0;
    size_type lenl = 0, lenr = 0;

    for (l += sz, r += sz; l < r; l >>= 1, r >>= 1) {
      if (l & 1) {
        resl = add(resl, mul(dat[l], p[lenl]));
        lenl += len[l];
        l++;
      }
      if (r & 1) {
        r--;
        resr = add(dat[r], mul(resr, p[len[r]]));
        lenr += len[r];
      }
    }

    return add(resl, mul(resr, p[lenl]));
  }

  // 文字列全体のハッシュ値を返す
  // O(1) time
  u64 hash() const { return dat[1]; }

  // 現在の文字列を返す
  // O(1) time
  const std::string& get_string() const { return S; }
};
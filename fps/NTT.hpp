#pragma once
#include "../utility/Template.hpp"
#include "../math/Functions.hpp"

namespace internal {

template <class Mint>
struct ntt_info {
  static constexpr int rank2 = __builtin_ctzll(Mint::get_mod() - 1);
  std::array<Mint, rank2> dw, dy;

  ntt_info() {
    static constexpr int G = primitive_root_constexpr(Mint::get_mod());

    Mint w[rank2], y[rank2];
    w[rank2 - 1] = Mint(G).pow((Mint::get_mod() - 1) >> rank2);
    y[rank2 - 1] = w[rank2 - 1].inv();
    for (int i = rank2 - 2; i >= 0; --i) {
      w[i] = w[i + 1] * w[i + 1];
      y[i] = y[i + 1] * y[i + 1];
    } 

    dw[1] = w[1], dy[1] = y[1], dw[2] = w[2], dy[2] = y[2];
    for (int i = 3; i < rank2; ++i) {
      dw[i] = dw[i - 1] * y[i - 2] * w[i];
      dy[i] = dy[i - 1] * w[i - 2] * y[i];
    }
  }
};

template <class Mint>
void butterfly(std::vector<Mint>& a) {
  int k = __builtin_ctzll(a.size());
  if (k == 1) {
    Mint a1 = a[1];
    a[1] = a[0] - a[1];
    a[0] = a[0] + a1;
    return;
  }
  static const ntt_info<Mint> info;

  if (k & 1) {
    int v = 1 << (k - 1);
    for (int i = 0; i < v; ++i) {
      Mint val = a[v + i];
      a[v + i] = a[i] - val;
      a[i] += val;
    }
  }

  Mint imag = info.dw[1];
  for (int u = 1 << (2 + (k & 1)), v = 1 << (k - 2 - (k & 1)); v; u <<= 2, v >>= 2) {
    for (int i = 0; i < v; ++i) {
      Mint t0 = a[i], t1 = a[v + i], t2 = a[2 * v + i], t3 = a[3 * v + i];
      Mint t0p2 = t0 + t2, t1p3 = t1 + t3, t0m2 = t0 - t2, t1m3 = (t1 - t3) * imag;
      a[i] = t0p2 + t1p3, a[v + i] = t0p2 - t1p3, a[2 * v + i] = t0m2 + t1m3, a[3 * v + i] = t0m2 - t1m3;
    }
    Mint rot = 1;
    for (unsigned int i = 4; i < u; i += 4) {
      rot *= info.dw[__builtin_ctzll(i)];
      Mint r2 = rot * rot, r3 = r2 * rot;
      for (int j = 0; j < v; ++j) {
        int idx = i * v + j;
        Mint t0 = a[idx], t1 = a[v + idx] * rot, t2 = a[2 * v + idx] * r2, t3 = a[3 * v + idx] * r3;
        Mint t0p2 = t0 + t2, t1p3 = t1 + t3, t0m2 = t0 - t2, t1m3 = (t1 - t3) * imag;
        a[idx] = t0p2 + t1p3, a[v + idx] = t0p2 - t1p3, a[2 * v + idx] = t0m2 + t1m3, a[3 * v + idx] = t0m2 - t1m3;
      }
    }
  }
}

template <class Mint>
void butterfly_inv(std::vector<Mint>& a) {
  int k = __builtin_ctzll(a.size());
  if (k == 1) {
    Mint a1 = a[1];
    a[1] = a[0] - a[1];
    a[0] = a[0] + a1;
    return;
  }
  static const ntt_info<Mint> info;
  Mint imag = info.dy[1];

  for (int u = 1 << (k - 2), v = 1; u; u >>= 4, v <<= 2) {
    for (int i = 0; i < v; ++i) {
      Mint t0 = a[i], t1 = a[v + i], t2 = a[2 * v + i], t3 = a[3 * v + i];
      Mint t0p1 = t0 + t1, t2p3 = t2 + t3, t0m1 = t0 - t1, t2m3 = (t2 - t3) * imag;
      a[i] = t0p1 + t2p3, a[v + i] = t0m1 + t2m3, a[2 * v + i] = t0p1 - t2p3, a[3 * v + i] = t0m1 - t2m3;
    }
    Mint irot = 1;
    u <<= 2;
    for (unsigned int i = 4; i < u; i += 4) {
      irot *= info.dy[__builtin_ctzll(i)];
      Mint r2 = irot * irot, r3 = r2 * irot;
      for (int j = 0; j < v; ++j) {
        int idx = i * v + j;
        Mint t0 = a[idx], t1 = a[v + idx], t2 = a[2 * v + idx], t3 = a[3 * v + idx];
        Mint t0p1 = t0 + t1, t2p3 = t2 + t3, t0m1 = (t0 - t1) * irot, t2m3 = (t2 - t3) * (irot * imag);
        a[idx] = t0p1 + t2p3, a[v + idx] = t0m1 + t2m3, a[2 * v + idx] = (t0p1 - t2p3) * r2, a[3 * v + idx] = (t0m1 - t2m3) * r2;
      }
    }
  }

  if (k & 1) {
    int h = 1 << (k - 1);
    for (int i = 0; i < h; ++i) {
      Mint v1 = a[i], v2 = a[h + i];
      a[i] = v1 + v2, a[h + i] = v1 - v2;
    }
  }
}

template <class Mint>
std::vector<Mint> convolution_ntt(std::vector<Mint> a, std::vector<Mint> b) {
  int N = a.size(), M = b.size();
  if (!N || !M) return {};

  int sz = std::bit_ceil((unsigned int)(N + M - 1));

  a.resize(sz), butterfly(a);
  b.resize(sz), butterfly(b);
  for (int i = 0; i < sz; ++i) a[i] *= b[i];
  butterfly_inv(a);
  a.resize(N + M - 1);

  Mint isz = Mint(sz).inv();
  for (int i = 0; i < N + M - 1; ++i) a[i] *= isz;
  return a;
}

template <class Mint>
std::vector<Mint> convolution_naive(const std::vector<Mint>& a, const std::vector<Mint>& b) {
  int N = a.size(), M = b.size();
  std::vector<Mint> res(N + M - 1);
  
  if (N < M) {
    for (int j = 0; j < M; ++j) {
      for (int i = 0; i < N; ++i) res[i + j] += a[i] * b[j];
    }
  }
  else {
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < M; ++j) res[i + j] += a[i] * b[j];
    }
  }

  return res;
}

};


// 数列 a, b の畳み込みを計算する
// O(NlogN) time
template <class Mint>
std::vector<Mint> convolution(std::vector<Mint>&& a, std::vector<Mint>&& b) {
  int N = a.size(), M = b.size();
  if (!N || !M) return {};

  int sz = std::bit_ceil((unsigned int)(N + M - 1));
  assert((Mint::get_mod() - 1) % sz == 0);

  if (std::min(N, M) <= 60) return internal::convolution_naive(std::move(a), std::move(b));
  return internal::convolution_ntt(std::move(a), std::move(b));
}

template <class Mint>
std::vector<Mint> convolution(const std::vector<Mint>& a, const std::vector<Mint>& b) {
  int N = a.size(), M = b.size();
  if (!N || !M) return {};

  int sz = std::bit_ceil((unsigned int)(N + M - 1));
  assert((Mint::get_mod() - 1) % sz == 0);

  if (std::min(N, M) <= 60) return internal::convolution_naive(a, b);
  return internal::convolution_ntt(a, b);
}

template <
  unsigned int m = 998244353,
  class T
>
std::vector<T> convolution(const std::vector<T>& a, const std::vector<T>& b) {
  int N = a.size(), M = b.size();
  if (!N || !M) return {};

  using Mint = Modint<m>;

  int sz = std::bit_ceil((unsigned int)(N + M - 1));
  assert((Mint::get_mod() - 1) % sz == 0);

  std::vector<Modint<m>> a2(N), b2(M);
  for (int i = 0; i < N; ++i) a2[i] = Mint(a[i]);
  for (int i = 0; i < M; ++i) b2[i] = Mint(b[i]);

  auto res2 = convolution(std::move(a2), std::move(b2));

  std::vector<T> res (N + M - 1);
  for (int i = 0; i < N + M - 1; ++i) res[i] = res2[i].val();
  return res;
}

// 64bit 整数列の畳み込みを計算する
// O(NlogN)
std::vector<long long> convolution_ll(const std::vector<long long>& a, const std::vector<long long>& b) {
  int N = a.size(), M = b.size();
  if (!N || !M) return {};

  static constexpr unsigned long long MOD1 = 754974721, MOD2 = 167772161, MOD3 = 469762049;
  static constexpr unsigned long long M1M2 = MOD1 * MOD2, M2M3 = MOD2 * MOD3, M3M1 = MOD3 * MOD1, M1M2M3 = MOD1 * MOD2 * MOD3;
  static constexpr unsigned long long i1 = inv_gcd(MOD2 * MOD3, MOD1).second;
  static constexpr unsigned long long i2 = inv_gcd(MOD3 * MOD1, MOD2).second;
  static constexpr unsigned long long i3 = inv_gcd(MOD1 * MOD2, MOD3).second;
  assert(N + M - 1 <= 16777216);

  auto c1 = convolution<MOD1>(a, b), c2 = convolution<MOD2>(a, b), c3 = convolution<MOD3>(a, b);

  std::vector<long long> res(N + M - 1);
  for (int i = 0; i < N + M - 1; ++i) {
    unsigned long long x = 0;
    x += (c1[i] * i1) % MOD1 * M2M3;
    x += (c2[i] * i2) % MOD2 * M3M1;
    x += (c3[i] * i3) % MOD3 * M1M2;
    long long diff = c1[i] - (long long)(x % MOD1);
    if (diff < 0) diff += MOD1;
    static constexpr unsigned long long offset[5] = {0, 0, M1M2M3, 2 * M1M2M3, 3 * M1M2M3};
    x -= offset[diff % 5];
    res[i] = x;
  }

  return res;
}

// 任意 mod 畳み込み（32bit）
// O(NlogN) time
template <class Mint>
std::vector<Mint> convolution_arbitrary(const std::vector<Mint>& a, const std::vector<Mint>& b) {
  int N = a.size(), M = b.size();
  if (!N || !M) return {};

  static constexpr long long MOD1 = 754974721, MOD2 = 167772161, MOD3 = 469762049;
  static constexpr long long i12 = 95869806, i123 = 187290749;
  assert(N + M - 1 <= 16777216);

  std::vector<long long> a1(N), a2(N), a3(N), b1(M), b2(M), b3(M);
  for (int i = 0; i < N; ++i) a1[i] = a2[i] = a3[i] = a[i].val();
  for (int i = 0; i < M; ++i) b1[i] = b2[i] = b3[i] = b[i].val();

  auto c1 = convolution<MOD1>(a1, b1), c2 = convolution<MOD2>(a2, b2), c3 = convolution<MOD3>(a3, b3);

  long long m1m2 = MOD1 * MOD2 % Mint::get_mod();
  std::vector<Mint> res(N + M - 1);
  for (int i = 0; i < N + M - 1; ++i) {
    long long v1 = c1[i];
    long long v2 = safe_mod(c2[i] - v1, MOD2) * i12 % MOD2;
    long long v3 = safe_mod(c3[i] - (v1 + v2 * MOD1) % MOD3, MOD3) * i123 % MOD3;
    res[i] = v1 + v2 * MOD1 + v3 * m1m2;
  }

  return res;
}

template <class Mint>
std::vector<Mint> convolution_arbitrary(std::vector<Mint>&& a, std::vector<Mint>&& b) { return convolution_arbitrary(std::move(a), std::move(b)); }

template <
  unsigned int m,
  class T
>
std::vector<T> convolution_arbitrary(const std::vector<T>& a, const std::vector<T>& b) {
  int N = a.size(), M = b.size();
  if (!N || !M) return {};

  static constexpr long long MOD1 = 754974721, MOD2 = 167772161, MOD3 = 469762049;
  static constexpr long long i12 = 95869806, i123 = 187290749;
  assert(N + M - 1 <= 16777216);

  auto c1 = convolution<MOD1>(a, b), c2 = convolution<MOD2>(a, b), c3 = convolution<MOD3>(a, b);

  long long m1m2 = MOD1 * MOD2 % m;
  std::vector<T> res(N + M - 1);
  for (int i = 0; i < N + M - 1; ++i) {
    long long v1 = c1[i];
    long long v2 = safe_mod((long long)c2[i] - v1, MOD2) * i12 % MOD2;
    long long v3 = safe_mod((long long)c3[i] - (v1 + v2 * MOD1) % MOD3, MOD3) * i123 % MOD3;
    res[i] = (v1 + v2 * MOD1 + v3 * m1m2) % m;
  }

  return res;
}
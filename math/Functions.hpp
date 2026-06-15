#pragma once
#include "../utility/XorShift.hpp"

namespace internal {

unsigned long long binary_gcd_impl(unsigned long long x, unsigned long long y) {
  if (x == y) return x;
  unsigned long long a = y - x, b = x - y;
  unsigned long long s = x < y ? a : b, t = x < y ? x : y;
  return binary_gcd_impl(s >> __builtin_ctzll(b), t);
}

constexpr bool miller_rabin(unsigned long long N, std::initializer_list<unsigned long long> A) {
  unsigned long long Ninv = N;
  __uint128_t T128 = -__uint128_t(N) % N;
  for (int i = 0; i < 5; ++i) Ninv *= 2 - N * Ninv;
  auto red = [&](__uint128_t t) noexcept {
    t = (t + __uint128_t((unsigned long long)t * -Ninv) * N) >> 64;
    return t < N ? t : t - N;
  };
  unsigned long long one = red(T128), m1 = N - one;

  unsigned long long d = N - 1;
  while (~d & 1) d >>= 1;

  for (unsigned long long base : A) {
    if (!(base % N)) continue;

    unsigned long long a = one, x = red(__uint128_t(base % N) * T128), n = d;
    while (n) {
      if (n & 1) a = red(__uint128_t(a) * x);
      x = red(__uint128_t(x) * x);
      n >>= 1;
    }

    for (n = d; n < N - 1 && a != one && a != m1; n <<= 1) a = red(__uint128_t(a) * a);
    if (a != m1 && (~n & 1)) return false;
  }

  return true;
}

}

// safe_mod
//
// x が負の数の場合も安全に剰余を取れる
// O(1) time
constexpr long long safe_mod(long long x, unsigned long long m) {
  x %= m;
  if (x < 0) x += m;
  return x;
}

// pow_mod_constexpr
//
// コンパイル時に冪乗の剰余を取った値を計算
// O(logN) time
constexpr unsigned long long pow_mod_constexpr(long long x,
                                               unsigned long long N,
                                               unsigned long long m) {
  if (m == 1) return 0;
  if (!N) return 1;
  unsigned long long y = safe_mod(x, m), res = 1;
  while (N) {
    if (N & 1) res = res * y % m;
    y = y * y % m;
    N >>= 1;
  }
  return res;
}

// inv_gcd
//
// {gcd(a, b), a^{-1} * gcd(a, b)} を返す
// O(log(a)) time
constexpr std::pair<long long, long long> inv_gcd(long long a, long long b) {
  a = safe_mod(a, b);
  if (!a) return {b, 0};

  long long u = b, v = a, m0 = 0, m1 = 1;
  while (v) {
    long long q = u / v;
    u -= q * v, std::swap(u, v);
    m0 -= q * m1, std::swap(m0, m1);
  }

  if (m0 < 0) m0 += b / u;
  return {u, m0};
}

// binary gcd
//
// 高速な gcd 計算
// O(log(x)) time
unsigned long long binary_gcd(unsigned long long x, unsigned long long y) {
  if (!x) return y;
  if (!y) return x;
  int n = __builtin_ctzll(x), m = __builtin_ctzll(y);
  return internal::binary_gcd_impl(x >> n, y >> m) << (n < m ? n : m);
}

// primitive_root_constexpr
//
// コンパイル時に m の原始根を計算する
// - m は素数
// O(sqrt(m)) time（平均的には非常に高速）
constexpr unsigned int primitive_root_constexpr(unsigned int m) {
  if (m == 2) return 1;
  if (m == 167772161) return 3;
  if (m == 469762049) return 3;
  if (m == 754974721) return 11;
  if (m == 998244353) return 3;

  unsigned int divs[20] = {}, cnt = 1, x = (m - 1) >> 1;
  divs[0] = 2;
  while (!(x & 1)) x >>= 1;
  for (unsigned int i = 3; (long long)(i) * i <= x; i += 2) {
    if (x % i) continue;
    divs[cnt++] = i;
    while (!(x % i)) x /= i;
  }
  if (x > 1) divs[cnt++] = x;

  for (unsigned int g = 2;; ++g) {
    bool f = true;
    for (unsigned int i = 0; i < cnt; ++i) {
      if (pow_mod_constexpr(g, (m - 1) / divs[i], m) == 1) {
        f = false;
        break;
      }
    }
    if (f) return g;
  }
}

// is_prime_constexpr
//
// コンパイル時での素数判定
// O(logN) time
constexpr bool is_prime_constexpr(unsigned long long N) {
  if (N <= 1) return false;
  if (N == 2 || N == 3 || N == 5 || N == 7 || N == 61) return true;
  if (!(N & 1) || !(N % 3) || !(N % 5)) return false;
  if (N < 4759123141ULL) return internal::miller_rabin(N, {2, 7, 61});
  return internal::miller_rabin(N, {2, 325, 9375, 28178, 450775, 9780504, 1795265022});  
}

// factorize
//
// Pollard の ρ 法を用いた高速素因数分解
// O(N^{1/4}logN) time
std::vector<unsigned long long> factorize(unsigned long long N) {
	if (N <= 1) return {};
	if (is_prime_constexpr(N)) return {N};

  unsigned long long N_ = N;
	std::vector<unsigned long long> p;
	for (unsigned long long i : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61,
                               67, 71, 73, 79, 83, 89, 97}) {
		if (N % i) continue;
		while (!(N % i)) p.push_back(i), N /= i;
	}
  if (N == 1) return p;
  if (is_prime_constexpr(N)) {
    p.push_back(N);
    return p;
  }

	unsigned long long r = 1;
	__uint128_t n2 = 1;
	auto update = [&]() {
		r = N;
		for (unsigned int i = 0; i < 5; ++i) r *= 2 - r * N;
		n2 = -(__uint128_t)N % N;
	};
	auto reduction = [&](__uint128_t t) noexcept -> unsigned long long {
		unsigned long long s = ((__uint128_t)N * ((unsigned long long)t * r)) >> 64, t2 = t >> 64;
		return t2 - s + (t2 < s ? N : 0);
	};
	auto mul = [&](unsigned long long a, unsigned long long b) noexcept -> unsigned long long {
		return reduction((__uint128_t)reduction((__uint128_t)a * n2) * b);
	};

	unsigned long long v;
	p.push_back(N);
	for (unsigned int pi = p.size() - 1; pi < p.size(); ++pi) {
		while (p[pi] != 1 && !is_prime_constexpr(p[pi])) {
			N = p[pi], update();
			while (p[pi] == N) {
				v = xor_shift();
				unsigned long long c = reduction(v);
				if (!c) continue;
				auto f = [&](unsigned long long a) noexcept -> unsigned long long {
					return reduction((__uint128_t)a * a + c);
				};
				unsigned long long a = 0, b = f(a), buf = 1, sz = 1, nxt = 10;
				while (true) {
					while (nxt != sz && a != b) {
						buf = mul(buf, a > b ? a - b : b - a);
						++sz;
						a = f(a), b = f(f(b));
					}
					unsigned long long g = binary_gcd(buf, N);
					if (g != 1) {
						while (!(p[pi] % g)) p[pi] /= g;
						p.push_back(g);
						break;
					}
					if (a == b) break;
					nxt = sz * 3 >> 1;
				}
			}
		}
	}

  std::vector<unsigned long long> res;
  for (unsigned long long q : p) {
    if (q == 1) continue;
    while (!(N_ % q)) res.push_back(q), N_ /= q;
  }
	std::sort(res.begin(), res.end());
	return res;
}

// divisors
//
// 高速な約数列挙
// O(N^{1/4}logN + d(N)) time
std::vector<unsigned long long> divisors(unsigned long long N) {
  if (!N) return {};

  std::vector<std::pair<unsigned long long, unsigned int>> v;
  for (unsigned long long p : factorize(N)) {
    if (v.empty() || v.back().first != p) v.emplace_back(p, 1);
    else v.back().second++;
  }

  std::vector<unsigned long long> res;
  auto dfs = [&](unsigned int idx, unsigned long long div, auto self) -> void {
    if (idx == v.size()) {
      res.push_back(div);
      return;
    }
    self(idx + 1, div, self);
    for (unsigned int i = 0; i < v[idx].second; ++i) self(idx + 1, div *= v[idx].first, self);
  };
  dfs(0, 1, dfs);
  std::sort(res.begin(), res.end());
  return res;
}
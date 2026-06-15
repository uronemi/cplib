#pragma once

namespace internal {

struct Barrett {
  private:
  unsigned int m;
  unsigned long long im;

  public:
  explicit Barrett(unsigned int m_) : m(m_), im((unsigned long long)(-1) / m + 1) {}

  unsigned int get_mod() const { return m; }

  unsigned int mul(unsigned int a, unsigned int b) const {
    unsigned long long z = a;
    z *= b;
    unsigned long long x = (unsigned long long)(((__uint128_t)(z) * im) >> 64);
    unsigned long long y = x * m;
    return (unsigned int)(z - y + (z < y ? m : 0));
  }
};

};

// Runtime Modint
//
// 実行時に法が決まる modint
template <unsigned int id>
class RuntimeModint {
  private:
  using Mint = RuntimeModint;
  unsigned int v;
  static inline internal::Barrett bt{998244353};

  static constexpr std::pair<int, int> inv_gcd(int a, int b) {
		if (a == 0) return {b, 0};
		int s = b, t = a, m0 = 0, m1 = 1;
		while (t) {
			const int q = s / t;
			s -= t * q, std::swap(s, t);
			m0 -= m1 * q, std::swap(m0, m1);
		}
		if (m0 < 0) m0 += b / s;
		return {s, m0};
	}

  public:
  RuntimeModint() : v(0) {}

  // v で初期化
  // - T は整数型
  template <typename T>
  RuntimeModint(T v_) {
    assert(std::is_integral_v<T>);
		if (std::is_signed_v<T>) {
			long long x = (long long)(v_ % (long long)(get_mod()));
			if (x < 0) x += get_mod();
			v = (unsigned int)(x);
		}
		else v = (unsigned int)(v_ % get_mod());
  }

  // 法を返す
  // O(1) time
  static unsigned int get_mod() { return bt.get_mod(); }

  // 法を m とする
  // - 1 <= m < 2^31
  // O(1) time
  static void set_mod(unsigned int m) {
    assert(1 <= m && m < (1U << 31));
    bt = internal::Barrett(m);
  }

  // v をそのまま代入する
  // - v < m
  // O(1) time
  static Mint raw(unsigned int v) {
    Mint x;
    x.v = v;
    return x;
  }

  unsigned int val() const { return v; }

  Mint& operator++() { return *this += 1; }

  Mint operator++(int) { Mint res = *this; ++*this; return res; }

  Mint& operator--() { return *this -= 1; }

  Mint operator--(int) { Mint res = *this; --*this; return res; }

  Mint& operator+=(const Mint& rhs) {
    v += rhs.v;
    if (v >= get_mod()) v -= get_mod();
    return *this;
  }

  Mint& operator-=(const Mint& rhs) {
    v += get_mod() - rhs.v;
    if (v >= get_mod())v -= get_mod();
    return *this;
  }

  Mint& operator*=(const Mint& rhs) {
    v = bt.mul(v, rhs.v);
    return *this;
  }

  Mint& operator/=(const Mint& rhs) { return *this *= rhs.inv(); }

  Mint operator+() const { return *this; }

  Mint operator-() const { return Mint{} - *this; }

  // ダブリングによる高速な冪乗計算
  // O(logN) time
  Mint pow(unsigned long long N) const {
    if (!N) return 1;
    Mint x = *this, res = 1;
    while (N) {
      if (N & 1) res *= x;
      x *= x;
      N >>= 1;
    }
    return res;
  }

  // modulo 逆元を返す
  // - gcd(v, m) = 1
  // O(log(m)) time
  Mint inv() const {
    auto eg = inv_gcd(v, get_mod());
    assert(eg.first == 1);
    return eg.second;
  }

  friend Mint operator+(const Mint& lhs, const Mint& rhs) { return Mint(lhs) += rhs; }

  friend Mint operator-(const Mint& lhs, const Mint& rhs) { return Mint(lhs) -= rhs; }

  friend Mint operator*(const Mint& lhs, const Mint& rhs) { return Mint(lhs) *= rhs; }

  friend Mint operator/(const Mint& lhs, const Mint& rhs) { return Mint(lhs) /= rhs; }

  friend bool operator==(const Mint& lhs, const Mint& rhs) { return lhs.v == rhs.v; }

  friend bool operator!=(const Mint& lhs, const Mint& rhs) { return lhs.v != rhs.v; }
};
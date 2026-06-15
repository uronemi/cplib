#pragma once

// Modint
//
// コンパイル時計算を用いて最適化された，自動で剰余を取る整数型
template <unsigned int m>
class Modint {
  private:
	using Mint = Modint;
	unsigned int v = 0;

	static constexpr bool is_prime = []() -> bool {
		if (m == 1) return false;
		if (m == 2 || m == 7 || m == 61) return true;
		if (!(m & 1)) return false;
		unsigned int d = m - 1;
		while (!(d & 1)) d >>= 1;
		for (unsigned int a : {2, 7, 61}) {
			unsigned int t = d, t_ = d;

			Mint y = 1, a_ = a;
			while (t_) {
				if (t_ & 1) y *= a_;
				a_ *= a_;
				t_ >>= 1;
			}

			while (t != m - 1 && y != 1 && y != m - 1) {
				y *= y;
				t <<= 1;
			}
			if (y != m - 1 && !(t & 1)) return false;
		}
		return true;
	}();
	
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
	constexpr Modint() {}

	// 整数型からの変換
	template <typename T>
	constexpr Modint(T v_) {
		static_assert(std::is_integral_v<T>, "T is not integral type.");
		if constexpr (std::is_signed_v<T>) {
			int64_t x = int64_t(v_ % int64_t(m));
			if (x < 0) x += m;
			v = (unsigned int)(x);
		}
		else v = (unsigned int)(v_ % m);
	}

	// 剰余を取らずに構築する
	// - 0 <= v < m
	static constexpr Mint raw(unsigned int v_) { Mint a; a.v = v_; return a; }

	// 法 m を返す
	static constexpr unsigned int get_mod() { return m; }

	// 値を返す
	constexpr unsigned int val() const { return v; }

	constexpr Mint& operator++() { return *this += 1; }

	constexpr Mint operator++(int) { Mint res = *this; ++*this; return res; }

	constexpr Mint& operator--() { return *this -= 1; }

	constexpr Mint operator--(int) { Mint res = *this; --*this; return res; }

	constexpr Mint& operator+=(Mint rhs) {
		if (v >= m - rhs.v) v -= m;
		v += rhs.v;
		return *this;
	}

	constexpr Mint& operator-=(Mint rhs) {
		if (v < rhs.v) v += m;
		v -= rhs.v;
		return *this;
	}

	constexpr Mint& operator*=(Mint rhs) { return *this = *this * rhs; }

	constexpr Mint& operator/=(Mint rhs) { return *this *= rhs.inv(); }

	constexpr Mint operator+() const { return *this; }

	constexpr Mint operator-() const { return Mint{} - *this; }

	// ダブリングによる高速な冪乗計算
	// O(logN) time
	constexpr Mint pow(unsigned long long N) const {
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
	// - gcd(v, m) == 1
	// O(logm) time
	constexpr Mint inv() const {
		if constexpr (is_prime) {
			assert(v);
			return pow(m - 2);
		}
		else {
			auto eg = inv_gcd(v, m);
			assert(eg.first == 1);
			return eg.second;
		}
	}

	friend constexpr Mint operator+(Mint lhs, Mint rhs) { return lhs += rhs; }

	friend constexpr Mint operator-(Mint lhs, Mint rhs) { return lhs -= rhs; }

	friend constexpr Mint operator*(Mint lhs, Mint rhs) { return (unsigned long long)(lhs.v) * rhs.v; }
	
	friend constexpr Mint operator/(Mint lhs, Mint rhs) { return lhs /= rhs; }

	friend constexpr bool operator==(Mint lhs, Mint rhs) { return lhs.v == rhs.v; }

	friend constexpr bool operator!=(Mint lhs, Mint rhs) { return lhs.v != rhs.v; }
};

using Mint998 = Modint<998244353>;
using Mint1007 = Modint<1000000007>;

constexpr Mint998 operator""_M(unsigned long long x) { return x; }
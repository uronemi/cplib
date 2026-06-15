#pragma once
#include "Functions.hpp"

// Mod Binomial
//
// k <= n <= N に対し binom(n, k) mod m を求めるためのクラス
// - m > 1
template <typename Mint>
class ModBinom {
  private:
  std::vector<Mint> f, fiv, iv;
  unsigned int N;

  public:
  // N を指定する
  // O(N) time
  explicit ModBinom(unsigned int N_) : f(N_ + 1), fiv(N_ + 1), iv(N_ + 1), N(N_) {
    unsigned int m = std::min(N + 1, Mint::get_mod());
    f[0] = 1;
    for (unsigned int i = 1; i < m; ++i) f[i] = f[i - 1] * Mint(i);
    fiv[m - 1] = f[m - 1].inv();
    iv[m - 1] = f[m - 2] * fiv[m - 1];
    for (unsigned int i = m - 1; i > 0; --i) {
      fiv[i - 1] = fiv[i] * Mint(i);
      iv[i] = f[i - 1] * fiv[i];
    }
  }

  // n! mod m を返す
  // - n <= N
  // O(1) time
  Mint factorial(unsigned int n) const {
    assert(n <= N);
    return f[n];
  }

  // (n!)^{-1} mod m を返す
  // - n <= N
  // O(1) time
  Mint finv(unsigned int n) const {
    assert(n <= N);
    return fiv[n];
  }

  // n^{-1} mod m を返す
  // - n <= N
  // O(1) time
  Mint inv(unsigned int n) const {
    assert(n <= N);
    return iv[n];
  }

  // binom(n, k) を返す
  // - k <= n <= N
  // O(1) time
  Mint operator()(unsigned int n, unsigned int k) const {
    assert(k <= n && n <= N);
    return f[n] * fiv[k] * fiv[n - k];
  }
};
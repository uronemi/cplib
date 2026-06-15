#pragma once
#include "NTT.hpp"
#include "FormalPowerSeries.hpp"

template <class Mint>
using FPS = FormalPowerSeries<Mint>;

template <class Mint>
FPS<Mint>& FPS<Mint>::operator*=(const FPS<Mint>& rhs) {
  if (this->empty() || rhs.empty()) {
    this->clear();
    return *this;
  }

  auto res = convolution<Mint>(*this, rhs);
  return *this = FPS<Mint>(res.begin(), res.end());
}

template <class Mint>
FPS<Mint>& FPS<Mint>::operator*=(FPS<Mint>&& rhs) {
  if (this->empty() || rhs.empty()) {
    this->clear();
    return *this;
  }

  auto res = convolution<Mint>(*this, std::move(rhs));
  return *this = FPS<Mint>(res.begin(), res.end());
}

// (f(x))^(-1) を返す
// O(NlogN) time
template <class Mint>
FPS<Mint> FPS<Mint>::inv(int deg) const {
  assert((*this)[0] != Mint(0));
  if (deg == -1) deg = (int)this->size();

  FPS<Mint> res(deg);
  res[0] = Mint(1) / (*this)[0];
  for (int d = 1; d < deg; d <<= 1) {
    FPS<Mint> f(2 * d), g(2 * d);
    for (int i = 0; i < std::min(deg, 2 * d); ++i) f[i] = (*this)[i];
    for (int i = 0; i < d; ++i) g[i] = res[i];
    internal::butterfly(f), internal::butterfly(g);  
    for (int i = 0; i < 2 * d; ++i) f[i] *= g[i];
    internal::butterfly_inv(f);
    for (int i = 0; i < d; ++i) f[i] = 0;
    internal::butterfly(f);
    for (int i = 0; i < 2 * d; ++i) f[i] *= g[i];
    internal::butterfly_inv(f);

    Mint iv = Mint(2 * d).inv();
    iv *= -iv;
    for (int i = d; i < std::min(2 * d, deg); ++i) res[i] = f[i] * iv;
  }

  return res.pre(deg);
}

// exp(f(x)) を返す
// O(NlogN) time
template <typename Mint>
FPS<Mint> FPS<Mint>::exp(int deg) const {
  assert(this->empty() || (*this)[0] == Mint(0));
  if (deg == -1) deg = this->size();

  FPS<Mint> res(deg), g = {1}, gt = {1, 1}, drv = this->diff(), inv = {0, 1};
  res[0] = 1;
  for (int i = 1; i < deg; ++i) res[i] = (*this)[i];
  drv.resize(deg);
  inv.reserve(deg + 1);

  auto integral_inplace = [&](FPS<Mint>& f) noexcept {
    const unsigned int n = f.size();
    auto mod = Mint::get_mod();
    while (inv.size() <= n) {
      unsigned int i = inv.size();
      inv.push_back((-inv[mod % i]) * (mod / i));
    }

    f.insert(f.begin(), Mint(0));
    for (unsigned int i = 1; i <= n; ++i) f[i] *= inv[i];
  };

  auto diff_inplace = [](FPS<Mint>& f) noexcept {
    if (f.empty()) return;
    f.erase(f.begin());
    Mint coeff = 1, one = 1;
    for (unsigned int i = 0; i < f.size(); ++i) {
      f[i] *= coeff;
      coeff += one;
    }
  };

  for (int d = 2; d < deg; d <<= 1) {
    FPS<Mint> ft(2 * d);
    for (int i = 0; i < std::min(deg, d); ++i) ft[i] = res[i];
    internal::butterfly(ft);

    FPS<Mint> fd(res.begin(), res.begin() + d);
    internal::butterfly(fd);
    
    {
      FPS<Mint> g_(d);
      for (int i = 0; i < d; ++i) g_[i] = fd[i] * gt[i];
      internal::butterfly_inv(g_);
      for (int i = 0; i < d / 2; ++i) g_[i] = g_[d / 2 + i];
      for (int i = d / 2; i < d; ++i) g_[i] = 0;
      internal::butterfly(g_);
      for (int i = 0; i < d; ++i) g_[i] *= gt[i];
      internal::butterfly_inv(g_);
      
      Mint imd2 = Mint(1) / (Mint(-d) * d);
      for (int i = 0; i < d / 2; ++i) g_[i] *= imd2;
      g.insert(g.end(), g_.begin(), g_.begin() + d / 2);
    }

    FPS<Mint> t(res.begin(), res.begin() + d);
    diff_inplace(t);
    t.push_back(Mint(0));
    {
      FPS<Mint> r(d);
      for (int i = 0; i < d - 1; ++i) r[i] = drv[i];
      internal::butterfly(r);
      for (int i = 0; i < d; ++i) r[i] *= fd[i];
      internal::butterfly_inv(r);
      r /= Mint(-d);
      t += r;
      t.insert(t.begin(), t.back()), t.pop_back();
    }

    if (2 * d < deg) {
      t.resize(2 * d);
      internal::butterfly(t);
      gt = g, gt.resize(2 * d);
      internal::butterfly(gt);
      for (int i = 0; i < 2 * d; ++i) t[i] *= gt[i];
      internal::butterfly_inv(t);
      t.resize(d);
      t /= 2 * d;
    }
    else {
      FPS<Mint> g1(d), t1(d);
      for (int i = 0; i < d / 2; ++i) g1[i] = g[i + d / 2], t1[i] = t[i + d / 2];
      for (int i = d / 2; i < d; ++i) t[i] = 0;
      internal::butterfly(g1), internal::butterfly(t1), internal::butterfly(t);
      for (int i = 0; i < d; ++i) t1[i] = gt[i] * t1[i] + g1[i] * t[i];
      for (int i = 0; i < d; ++i) t[i] *= gt[i];
      internal::butterfly_inv(t), internal::butterfly_inv(t1);
      Mint iv = Mint(1) / Mint(d);
      for (int i = 0; i < d; ++i) t[i] *= iv, t1[i] *= iv;
      for (int i = 0; i < d / 2; ++i) t[i + d / 2] += t1[i];
    }

    FPS<Mint> v(2 * d);
    for (int i = 0; i < std::min(deg - d, d); ++i) v[i] = res[d + i];
    t.insert(t.begin(), d - 1, 0);
    t.push_back(0);
    integral_inplace(t);
    for (int i = 0; i < d; ++i) v[i] -= t[d + i];

    internal::butterfly(v);
    for (int i = 0; i < 2 * d; ++i) v[i] *= ft[i];
    internal::butterfly_inv(v);
    v.resize(d);
    v /= Mint(2 * d);

    for (int i = 0; i < std::min(deg - d, d); ++i) res[d + i] = v[i];
  }

  return res;
}
#pragma once

// Formal Power Series
//
// Mint 上の形式的冪級数を扱う構造体
template <typename Mint>
struct FormalPowerSeries : std::vector<Mint> {
  private:
  using std::vector<Mint>::vector;
  using std::vector<Mint>::operator=;
  using std::vector<Mint>::empty;
  using std::vector<Mint>::reserve;
  using std::vector<Mint>::push_back;
  using std::vector<Mint>::pop_back;
  using std::vector<Mint>::size;
  using std::vector<Mint>::insert;
  using std::vector<Mint>::erase;
  using std::vector<Mint>::begin;
  using std::vector<Mint>::end;
  using std::vector<Mint>::resize;
  using std::vector<Mint>::back;
  using F = FormalPowerSeries;

  public:
  F& operator+=(const F& rhs) {
    if (this->size() < rhs.size()) this->resize(rhs.size());
    for (unsigned int i = 0; i < rhs.size(); ++i) (*this)[i] += rhs[i];
    return *this;
  }

  F& operator+=(const Mint& rhs) {
    if (this->empty()) this->resize(1);
    (*this)[0] += rhs;
    return *this;
  }

  F& operator-=(const F& rhs) {
    if (this->size() < rhs.size()) this->resize(rhs.size());
    for (unsigned int i = 0; i < rhs.size(); ++i) (*this)[i] -= rhs[i];
    return *this;
  }

  F& operator-=(const Mint& rhs) {
    if (this->empty()) this->resize(1);
    (*this)[0] -= rhs;
    return *this;
  }

  F& operator*=(const Mint& rhs) {
    for (unsigned int i = 0; i < this->size(); ++i) (*this)[i] *= rhs;
    return *this;
  }

  F& operator/=(const F& rhs) {
    if (this->size() < rhs.size()) {
      this->clear();
      return *this;
    }

    unsigned int n = this->size() - rhs.size() + 1;
    if (rhs.size() <= 60) {
      F f(*this), g(rhs);
      g.shrink();

      Mint coeff = g.back().inv();
      for (auto& x : g) x *= coeff;

      unsigned int deg = f.size() - g.size() + 1, gs = g.size();
      F q(deg);
      for (unsigned int i = deg; i-- > 0;) {
        q[i] = f[i + gs - 1];
        for (unsigned int j = 0; j < gs; ++j) f[i + j] -= q[i] * g[j];
      }

      *this = q * coeff;
      this->resize(n, Mint(0));
      return *this;
    }
    return *this = ((*this).rev().pre(n) * rhs.rev().inv(n)).pre(n).rev();
  }

  F& operator/=(const Mint& rhs) { return *this *= Mint(1) / Mint(rhs); }

  F& operator%=(const F& rhs) {
    *this -= *this / rhs * rhs;
    shrink();
    return *this;
  }

  F operator+(const F& rhs) const { return F(*this) += rhs; }

  F operator+(const Mint& rhs) const { return F(*this) += rhs; }

  F operator-(const F& rhs) const { return F(*this) -= rhs; }

  F operator-(const Mint& rhs) const { return F(*this) -= rhs; }

  F operator*(const F& rhs) const { return F(*this) *= rhs; }

  F operator*(const Mint& rhs) const { return F(*this) *= rhs; }

  F operator/(const F& rhs) const { return F(*this) /= rhs; }

  F operator/(const Mint& rhs) const { return F(*this) /= rhs; }

  F operator%(const F& rhs) const { return F(*this) %= rhs; }

  F operator-() const {
    F res(this->size());
    for (unsigned int i = 0; i < this->size(); ++i) res[i] = -(*this)[i];
    return res;
  }

  // 最高次が非ゼロまたは空列になるようにする
  // O(N) time
  void shrink() { while (!this->empty() && this->back() == Mint(0)) this->pop_back(); }

  // 係数列を反転する
  // O(N) time
  F rev() const {
    F res(*this);
    std::reverse(res.begin(), res.end());
    return res;
  }

  // 次数ごとに係数をかける
  // O(N) time
  F dot(const F& rhs) const {
    F res(std::min(this->size(), rhs.size()));
    for (unsigned int i = 0; i < res.size(); ++i) res[i] = (*this)[i] * rhs[i];
    return res;
  }

  // 前 N 項を取る
  // O(N) time
  F pre(unsigned int N) const {
    F res(this->begin(), this->begin() + std::min((unsigned int)this->size(), N));
    if (res.size() < N) res.resize(N, Mint(0));
    return res;
  }

  F operator>>(unsigned int N) const {
    if (this->size() <= N) return {};
    F res(*this);
    res.erase(res.begin(), res.begin() + N);
    return res;
  }

  F operator<<(unsigned int N) const {
    F res(*this);
    res.insert(res.begin(), N, Mint(0));
    return res;
  }

  // 微分
  // O(N) time
  F diff() const {
    unsigned int n = this->size();
    F res(std::max(0U, n - 1));
    Mint one(1), coeff(1);
    for (unsigned int i = 1; i < n; ++i) {
      res[i - 1] = (*this)[i] * coeff;
      coeff += one;
    }
    return res;
  }

  // 積分
  // O(N) time
  F integral() const {
    unsigned int n = this->size();
    F res(n + 1);
    res[0] = Mint(0);
    if (n) res[1] = Mint(1);
    auto mod = Mint::get_mod();
    for (unsigned int i = 2; i <= n; ++i) res[i] = (-res[mod % i]) * (mod / i);
    for (unsigned int i = 0; i < n; ++i) res[i + 1] *= (*this)[i];
    return res;
  }

  // f(x) を返す
  // O(N) time
  Mint eval(const Mint& x) {
    Mint res = 0, p = 1;
    for (auto& c : *this) res += c * p, p *= x;
    return res;
  }

  // f(x) = exp(g(x)) を満たす g を返す
  // O(NlogN) time
  F log(int deg = -1) const {
    assert(!this->empty() && (*this)[0] == Mint(1));
    if (deg == -1) deg = (int)this->size();
    return (this->diff() * this->inv(deg)).pre(deg - 1).integral();
  }

  // (f(x))^k を返す
  // O(NlogN) time
  F pow(unsigned long long k, int deg = -1) const {
    const int n = (int)this->size();
    if (deg == -1) deg = n;
    if (k == 0) {
      F res(deg);
      if (deg) res[0] = 1;
      return res;
    }

    for (unsigned int i = 0; i < n; ++i) {
      if ((*this)[i] != Mint(0)) {
        Mint iv = Mint(1) / (*this)[i];
        F res = ((((*this) * iv) >> i).log(deg) * k).exp(deg);
        res *= (*this)[i].pow(k);
        res = (res << (i * k)).pre(deg);
        if ((int)res.size() < deg) res.resize(deg, Mint(0));
        return res;
      }
      if ((__int128_t)(i + 1) * k >= deg) return F(deg, Mint(0));
    }
    
    return F(deg, Mint(0));
  }

  F& operator*=(const F& rhs);
  F& operator*=(F&& rhs);
  F inv(int deg = -1) const;
  F exp(int deg = -1) const;
};
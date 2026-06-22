#pragma once

// Potentialized DSU
//
// 各頂点のポテンシャルを管理する DSU。可換群が乗る
template <
  typename S,
  auto op,
  auto e,
  auto inverse
>
class PotentializedDSU {
  public:
  using value_type = S;
  using size_type = std::size_t;

  private:
  std::vector<int> par;
  std::vector<S> diff_;
  size_type N;

  public:
  PotentializedDSU(size_type n) : par(n, -1), diff_(n, e()), N(n) {}

  size_type leader(size_type x) {
    if (par[x] < 0) return x;
    size_type l = leader(par[x]);
    diff_[x] = op(diff_[x], diff_[par[x]]);
    return par[x] = l;
  }

  // potential(y) = potential(x) + w となるように x と y を merge
  // - x, y < N
  // O(α(N)) time
  bool merge(size_type x, size_type y, S w) {
    w = op(w, potential(x)), w = op(w, inverse(potential(y)));
    x = leader(x), y = leader(y);
    if (x == y) return false;
    if (par[x] > par[y]) std::swap(x, y), w = inverse(w);

    par[x] += par[y];
    par[y] = x;
    diff_[y] = w;
    return true;
  }

  bool same(size_type x, size_type y) { return leader(x) == leader(y); }

  // x のポテンシャルを返す
  // - x < N
  // O(α(N)) time
  value_type potential(size_type x) {
    leader(x);
    return diff_[x];
  }

  // potential(y) - potential(x) を返す
  // - x, y < N
  // O(α(N)) time
  value_type diff(size_type x, size_type y) { return op(potential(y), inverse(potential(x))); }
};
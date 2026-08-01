#pragma once
#include "../utility/Algebra.hpp"

// Disjoint Set Union（DSU）
//
// 集合族を管理するデータ構造。可換モノイドを載せる実装
template <IsMonoid M = internal::VoidMonoid>
class DisjointSetUnion {
  private:
  using value_type = M::value_type;
  unsigned int N;
  std::vector<std::pair<int, value_type>> tree;

  public:
  DisjointSetUnion() {}

  explicit DisjointSetUnion(unsigned int N_) : DisjointSetUnion(std::vector<value_type>(N_, M::e())) {}

  // 初期値 v で DSU を構築
  // O(|v|) time
  explicit DisjointSetUnion(const std::vector<value_type>& v) : N(v.size()), tree(N) {
    for (unsigned int i = 0; i < N; ++i) tree[i] = {-1, v[i]};
  }

  // x が属する集合の代表元を返す
  // - x < N
  // O(α(N)) time（α(N) は逆 Ackermann 関数）
  unsigned int leader(unsigned int x) {
    assert(x < N);
    if (tree[x].first < 0) return x;
    return tree[x].first = leader(tree[x].first);
  }

  // x が属する集合の要素数を返す
  // - x < N
  // O(α(N)) time
  unsigned int size(unsigned int x) { return -tree[leader(x)].first; }

  // x が属する集合の値の総積を返す
  // - x < N
  // O(α(N)) time
  value_type fold(unsigned int x) { return tree[leader(x)].second; }

  // x と y が属する集合を併合
  // - x < N && y < N
  // O(α(N)) time
  bool merge(unsigned int x, unsigned int y) {
    x = leader(x), y = leader(y);
    if (x == y) return false;
    if (tree[x].first > tree[y].first) std::swap(x, y);

    tree[x].first += tree[y].first;
    tree[y].first = x;
    tree[x].second = M::op(tree[x].second, tree[y].second);
    return true;
  }

  // x と y が同じ集合に属するか返す
  // - x < N && y < N
  // O(α(N)) time
  bool same(unsigned int x, unsigned int y) { return leader(x) == leader(y); }

  // 現在の集合族の状態を返す
  // O(Nα(N)) time
  std::vector<std::vector<unsigned int>> groups() {
    std::vector<std::vector<unsigned int>> res(N);

    for (unsigned int i = 0; i < N; ++i) res[leader(i)].emplace_back(i);

    res.erase(std::remove_if(res.begin(),
                             res.end(),
                             [&](const std::vector<unsigned int>& v) { return v.empty(); }),
              res.end());
    return res;
  }
};
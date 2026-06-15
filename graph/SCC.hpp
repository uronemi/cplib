#pragma once

// Strongly Connected Component
//
// 有向グラフを強連結成分分解する
class SCC {
  private:
  std::vector<std::vector<unsigned int>> G, rev;
  std::vector<unsigned int> component_;
  unsigned int N;

  void dfs1(unsigned int pos, unsigned int& i, std::vector<bool>& visited, std::vector<unsigned int>& v) {
    visited[pos] = true;
    for (unsigned int& nxt : G[pos]) {
      if (visited[nxt]) continue;
      dfs1(nxt, i, visited, v);
    }
    v[i++] = pos;
  }

  void dfs2(unsigned int pos, unsigned int c, std::vector<bool>& visited, std::vector<std::vector<unsigned int>>& mem) {
    visited[pos] = true;
    mem[c].emplace_back(pos);
    component_[pos] = c;
    for (unsigned int& nxt : rev[pos]) {
      if (visited[nxt]) continue;
      dfs2(nxt, c, visited, mem);
    }
  }

  public:
  SCC() {}

  // 頂点数 N_ でグラフを作成
  // O(N) time
  explicit SCC(unsigned int N_) : G(N_), rev(N_), component_(N_), N(N_) {}

  // 有向辺 u->v を追加
  // - u < N && v < N
  // O(1) time
  void add_edge(unsigned int u, unsigned int v) {
    assert(u < N && v < N);
    G[u].emplace_back(v);
    rev[v].emplace_back(u);
  }

  // グラフを強連結成分分解する
  // O(N) time
  std::vector<std::vector<unsigned int>> scc() {
    std::vector<std::vector<unsigned int>> mem;
    std::vector<unsigned int> v(N);
    std::vector<bool> visited(N);
    unsigned int cnt = 0;

    for (unsigned int i = 0; i < N; ++i) {
      if (visited[i]) continue;
      dfs1(i, cnt, visited, v);
    }

    for (auto&& b : visited) b = false;

    unsigned int c = 0;
    for (unsigned int i = N; i-- > 0;) {
      if (visited[v[i]]) continue;
      mem.emplace_back();
      dfs2(v[i], c++, visited, mem);
    }

    return mem;
  }

  // x が属する強連結成分の番号を返す
  // O(1) time
  unsigned int component(unsigned int x) const {
    assert(x < N);
    return component_[x];
  }
};
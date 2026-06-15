#pragma once

// Heavy Light Decomposition
//
// 木をいくつかのパスに分解する
class HeavyLightDecomposition {
  private:
  unsigned int N;
  std::vector<std::vector<unsigned int>> G;
  std::vector<unsigned int> idx, rev, head, sz, par, depth;

  void dfs1(unsigned int pos, unsigned int p) {
    if (G[pos].size() && G[pos][0] == p) std::swap(G[pos][0], G[pos].back());
    for (unsigned int& nxt : G[pos]) {
      if (nxt == p) continue;
      dfs1(nxt, pos);
      sz[pos] += sz[nxt];
      if (sz[nxt] > sz[G[pos][0]]) std::swap(nxt, G[pos][0]);
    }
  }

  void dfs2(unsigned int pos, unsigned int p, unsigned int& t) {
    par[pos] = p, idx[pos] = t, rev[t++] = pos;
    for (unsigned int& nxt : G[pos]) {
      if (nxt == p) continue;
      head[nxt] = (nxt == G[pos][0] ? head[pos] : nxt);
      depth[nxt] = depth[pos] + 1;
      dfs2(nxt, pos, t);
    }
  }

  public:
  HeavyLightDecomposition() {}

  // 頂点数 N_ でグラフを構築
  // O(N) time
  explicit HeavyLightDecomposition(unsigned int N_)
    : N(N_), G(N), idx(N), rev(N), head(N), sz(N, 1), par(N), depth(N) {}

  // 無向辺 u<->v を追加
  // O(1) time
  void add_edge(unsigned int u, unsigned int v) {
    assert(u < N && v < N);
    G[u].push_back(v);
    G[v].push_back(u);
  }

  // r を根として HLD を実行する
  // - r < N
  // O(N) time
  void build(unsigned int r = 0) {
    assert(r < N);
    dfs1(r, N);
    head[r] = r;
    unsigned int t = 0;
    dfs2(r, N, t);
  }

  // u に対応する添字を返す
  // - u < N
  // O(1) time
  unsigned int get_idx(unsigned int u) const {
    assert(u < N);
    return idx[u];
  }

  // パス u->v に含まれる端点含む頂点に対するクエリを処理する
  // - u < N && v < N
  // - f(l, r) は閉区間 [l, r] に対する処理
  // O(f(N)logN) time
  void path_vertex(unsigned int u, unsigned int v, const auto& f) {
    assert(u < N && v < N);
    while (true) {
      if (idx[u] > idx[v]) std::swap(u, v);
      f(std::max(idx[head[v]], idx[u]), idx[v]);
      if (head[u] != head[v]) v = par[head[v]];
      else break;
    }
  }

  // パス u->v に含まれる辺に対するクエリを処理する
  // - u < N && v < N
  // - f(l, r) は閉区間 [l, r] に対する処理
  // O(f(N)logN) time
  void path_edge(unsigned int u, unsigned int v, const auto& f) {
    assert(u < N && v < N);
    while (true) {
      if (idx[u] > idx[v]) std::swap(u, v);
      if (head[u] != head[v]) {
        f(idx[head[v]], idx[v]);
        v = par[head[v]];
      }
      else {
        if (u != v) f(idx[u] + 1, idx[v]);
        break;
      }
    }
  }

  // u と v の LCA を返す
  // - u < N && v < N
  // O(logN) time
  unsigned int lca(unsigned int u, unsigned int v) const {
    assert(u < N && v < N);
    while (true) {
      if (idx[u] > idx[v]) std::swap(u, v);
      if (head[u] == head[v]) return u;
      v = par[head[v]];
    }
  }

  // u の k 個上の祖先を求める。存在しないときは N を返す
  // - u < N
  // O(logN) time
  unsigned int level_ancestor(unsigned int u, unsigned int k) const {
    assert(u < N);
    while (true) {
      if (idx[u] - idx[head[u]] >= k) return rev[idx[u] - k];
      k -= idx[u] - idx[head[u]] + 1, u = par[head[u]];
      if (u == N) return N;
    }
  }

  // u, v の距離を返す
  // - u, v < N
  // O(logN) time
  unsigned int dist(unsigned int u, unsigned int v) const {
    assert(u < N && v < N);
    return depth[u] + depth[v] - 2 * depth[lca(u, v)];
  }
};
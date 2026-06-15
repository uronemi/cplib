#pragma once

// Rerooting（全方位木 dp）
//
// 木の各頂点 v に対し，v を根としたときの木 dp の結果を計算
template <
  typename E,                       // 可換モノイドの型
  typename V,                       // DP の値の型
  E (*merge)(E, E),                 // ⊕ : E × E → E を計算する
  E (*e)(),                         // 単位元 e in E を返す
  E (*put_edge)(V, unsigned int),   // 辺番号 i の辺を加える
  V (*put_vertex)(E, unsigned int)  // 頂点番号 v の頂点を加える
>
class Rerooting {
  private:
  struct edge { unsigned int to, idx, ridx; };

  std::vector<std::vector<edge>> G;
  std::vector<std::vector<E>> sub_dp;
  unsigned int N, id;

  std::vector<V> reroot(unsigned int r) {
    std::vector<V> res(N);

    auto dfs = [&](unsigned int pos, unsigned int par, E p_val, const auto& self) -> void {
      unsigned int d = G[pos].size();
      for (unsigned int i = 0; i < d; ++i) {
        if (G[pos][i].to == par) sub_dp[pos][i] = p_val;
      }

      std::vector<E> acc_l(d + 1, e()), acc_r(d + 1, e());
      for (unsigned int i = 0; i < d; ++i) acc_l[i + 1] = merge(acc_l[i], sub_dp[pos][i]);
      for (unsigned int i = d; i > 0; --i) acc_r[i - 1] = merge(sub_dp[pos][i - 1], acc_r[i]);
      res[pos] = put_vertex(acc_l[d], pos);

      for (unsigned int i = 0; i < d; ++i) {
        if (G[pos][i].to == par) continue;
        E exc_dp = put_vertex(merge(acc_l[i], acc_r[i + 1]), pos);
        self(G[pos][i].to, pos, put_edge(exc_dp, G[pos][i].ridx), self);
      }
    };

    dfs(r, N, e(), dfs);
    return res;
  }

  public:
  // 頂点数 N で木を作成
  // O(N) time
  explicit Rerooting(unsigned int N_) : G(N_), sub_dp(N_), N(N_) {}

  // 辺番号 idx の有向辺 u->v，辺番号 ridx の有向辺 v->u を追加
  // - u, v < N && idx, ridx < 2 N - 2
  // O(1) time
  void add_edge(unsigned int u, unsigned int v, unsigned int idx, unsigned int ridx) {
    assert(u < N && v < N && idx < 2 * (N - 1) && ridx < 2 * (N - 1));
    G[u].emplace_back(v, idx, ridx), G[v].emplace_back(u, ridx, idx);
  }

  // r が属する木で全方位木 dp を行う
  std::vector<V> build(unsigned int r = 0) {
    std::vector<V> dp(N);

    auto dfs = [&](unsigned int pos, unsigned int par, const auto& self) -> E {
      E res = e();
      sub_dp[pos].resize(G[pos].size());
      for (unsigned int i = 0; i < G[pos].size(); ++i) {
        if (G[pos][i].to == par) continue;
        V ch_dp = put_vertex(self(G[pos][i].to, pos, self), G[pos][i].to);
        sub_dp[pos][i] = put_edge(ch_dp, G[pos][i].idx);
        res = merge(res, sub_dp[pos][i]);
      }
      return res;
    };

    dfs(r, N, dfs);
    return reroot(r);
  }
};
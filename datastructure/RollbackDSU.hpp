#pragma once

// Rollback DSU
//
// undo・rollback が可能な DSU
class RollbackDSU {
  private:
  std::vector<int> tree;
  std::stack<std::pair<unsigned int, int>> history;
  unsigned int N, snap;

  public:
  RollbackDSU() {}

  // N 個の集合で DSU を構築
  // O(N) time
  explicit RollbackDSU(unsigned int N_) : tree(N_, -1), N(N_), snap(0) {}

  // x が属する集合の代表元を返す
  // - x < N
  // O(α(N)) time（α(N) は逆 Ackermann 関数）
  unsigned int leader(unsigned int x) {
    assert(x < N);
    if (tree[x] < 0) return x;
    return tree[x] = leader(tree[x]);
  }

  // x が属する集合の要素数を返す
  // - x < N
  // O(α(N)) time
  unsigned int size(unsigned int x) {
    assert(x < N);
    return -tree[leader(x)];
  }

  // x と y が属する集合を併合
  // - x < N && y < N
  // O(α(N)) time
  bool merge(unsigned int x, unsigned int y) {
    assert(x < N);

    x = leader(x), y = leader(y);
    history.emplace(x, tree[x]);
    history.emplace(y, tree[y]);
    if (x == y) return false;
    if (tree[x] > tree[y]) std::swap(x, y);

    tree[x] += tree[y];
    tree[y] = x;
    return true;
  }

  // x と y が同じ集合に属するか返す
  // - x < N && y < N
  // O(α(N)) time
  bool same(unsigned int x, unsigned int y) {
    assert(x < N && y < N);
    return leader(x) == leader(y);
  }

  // 直前の merge を取り消す
  // O(1) time
  void undo() {
    tree[history.top().first] = history.top().second;
    history.pop();
    tree[history.top().first] = history.top().second;
    history.pop();
  }

  // 現在の状態で snapshot を撮る
  // O(1) time
  void snapshot() { snap = history.size() >> 1; }
  
  // snapshot を撮ったときの状態に戻す
  // snapshot を撮ってからの merge の回数を m として O(m) time
  void rollback() {
    unsigned int state = snap << 1;
    assert(state <= history.size());
    while (state < history.size()) undo();
  }
};
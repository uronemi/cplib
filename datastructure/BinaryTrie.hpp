#pragma once

// Binary Trie
//
// 非負整数の（多重）集合につよいデータ構造
struct BinaryTrie {
  private:
  using u64 = unsigned long long;
  using u32 = unsigned int;
  struct Node {
    Node *ch[2];
    u64 cnt;

    Node() : ch{nullptr, nullptr}, cnt(0) {}
  };

  Node *root;
  static constexpr const u32 W = 64;

  public:
  explicit BinaryTrie() : root(new Node()) {}

  // 現在の集合の要素数を返す
  // O(1) time
  u64 size() const { return root->cnt; }

  // 集合が空かどうか返す
  // O(1) time
  bool empty() const { return !root->cnt; }

  // x を k 個挿入する
  // O(w) time（w は wordsize）
  void insert(u64 x, u64 k = 1, u64 xor_val = 0) {
    root->cnt += k;
    Node *cur = root;
    for (u32 i = 64; i--;) {
      u32 to = ((x >> i) & 1) ^ ((xor_val >> i) & 1);
      if (!cur->ch[to]) cur->ch[to] = new Node();
      cur = cur->ch[to];
      cur->cnt += k;
    }
  }

  // x を 1 個削除（存在しない場合は何もしない）
  // O(w) time
  void erase(u64 x, u64 xor_val = 0) {
    if (!contains(x, xor_val)) return;
    root->cnt--;
    Node *cur = root;
    for (u32 i = 64; i--;) {
      u32 to = ((x >> i) & 1) ^ ((xor_val >> i) & 1);
      if (!cur->ch[to]) return;
      cur = cur->ch[to];
      cur->cnt--;
    }
  }

  // 集合に含まれる x の個数を返す
  // O(w) time
  u64 count(u64 x, u64 xor_val = 0) const {
    Node *cur = root;
    for (u32 i = 64; i--;) {
      u32 to = ((x >> i) & 1) ^ ((xor_val >> i) & 1);
      if (!cur->ch[to]) return 0;
      cur = cur->ch[to];
    }
    return cur->cnt;
  }

  // 集合に x が含まれるか返す
  // O(w) time
  bool contains(u64 x, u64 xor_val = 0) const {
    Node *cur = root;
    for (u32 i = 64; i--;) {
      u32 to = ((x >> i) & 1) ^ ((xor_val >> i) & 1);
      if (!cur->ch[to]) return false;
      cur = cur->ch[to];
    }
    return cur->cnt > 0;
  }

  // 0-indexed で k 番目に小さい要素を返す
  //
  // - k < |S|
  //
  // O(w) time
  u64 kth_element(u64 k, u64 xor_val = 0) const {
    assert(k < root->cnt);
    Node *cur = root;
    u64 res = 0;
    for (u32 i = 64; i--;) {
      u64 b = (xor_val >> i) & 1, cnt = (cur->ch[b] ? cur->ch[b]->cnt : 0);
      if (k < cnt) {
        cur = cur->ch[b];
        res |= b << i;
      }
      else {
        k -= cnt;
        cur = cur->ch[b ^ 1];
        res |= (b ^ 1) << i;
      }
    }
    return res;
  }

  template <auto f>
  void fold_lt(u64 r, u64 xor_val = 0) const { fold_lt(r, [](u64 x, u64 k) { f(x, k); }, xor_val); }

  // r 未満の要素について f(x, count(x)) を実行する
  //
  // - f は bit ごとに実行可能であること
  //
  // O(w) time
  template <typename F>
  void fold_lt(u64 r, const F& f, u64 xor_val = 0) const {
    Node *cur = root;
    u64 pre = 0;
    for (u32 i = 64; i--;) {
      u64 b = (xor_val >> i) & 1;
      if ((r >> i) & 1) {
        if (cur->ch[b] && cur->ch[b]->cnt) f(pre | (b << i), cur->ch[b]->cnt);
        if (!cur->ch[b ^ 1] || !cur->ch[b ^ 1]->cnt) break;
        cur = cur->ch[b ^ 1];
        pre |= (b ^ 1) << i;
      }
      else {
        if (!cur->ch[b] || !cur->ch[b ^ 1]->cnt) break;
        cur = cur->ch[b];
        pre |= b << i;
      }
    }
  }
};
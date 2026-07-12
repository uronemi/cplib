#pragma once
#include "../utility/Algebra.hpp"

// SplayTree Array（遅延伝播反転可能 Splay Tree）
//
// †最強†のデータ構造。区間反転のために可換性を要求する実装。
template <IsReversibleActedMonoid M>
struct SplayTreeArray {
  public:
  using value_type = M::value_type;
  using operator_type = M::operator_type;
  
  private:
  using idx_type = unsigned int;
  struct Node {
    value_type val, acc;
    operator_type lazy;
    idx_type l, r, sz;
    bool rev, propagated;
  };

  static std::vector<Node> pool;
  idx_type root;

  static inline idx_type new_node(const value_type& x) {
    idx_type idx = pool.size();
    pool.push_back({x, x, M::id(), 0, 0, 1, false, true});
    return idx;
  }

  static inline idx_type pushup(idx_type t) {
    if (!t) return 0;
    pool[t].sz = 1 + pool[pool[t].l].sz + pool[pool[t].r].sz;
    pool[t].acc = M::op(M::op(pool[pool[t].l].acc, pool[t].val), pool[pool[t].r].acc);
    return t;
  }

  static inline void toggle(idx_type t) {
    if (!t) return;
    std::swap(pool[t].l, pool[t].r);
    pool[t].acc = M::rev(pool[t].acc);
    pool[t].rev ^= true;
  }

  static inline void propagate(idx_type t, const operator_type& f) {
    if (!t) return;
    pool[t].val = M::mapping(f, pool[t].val);
    pool[t].acc = M::mapping(f, pool[t].acc);
    pool[t].lazy = pool[t].propagated ? f : M::composition(f, pool[t].lazy);
    pool[t].propagated = false;
  }

  static inline void pushdown(idx_type t) {
    if (!t) return;
    if (!pool[t].propagated) {
      propagate(pool[t].l, pool[t].lazy);
      propagate(pool[t].r, pool[t].lazy);
      pool[t].propagated = true;
    }
    if (pool[t].rev) {
      toggle(pool[t].l);
      toggle(pool[t].r);
      pool[t].rev = false;
    }
  }

  static inline idx_type rotL(idx_type t) {
    idx_type x = pool[t].r;
    pool[t].r = pool[x].l;
    pool[x].l = pushup(t);
    return pushup(x);
  }

  static inline idx_type rotR(idx_type t) {
    idx_type x = pool[t].l;
    pool[t].l = pool[x].r;
    pool[x].r = pushup(t);
    return pushup(x);
  }

  static idx_type splay(idx_type t, idx_type k) {
    if (!t) return 0;
    pushdown(t);
    idx_type lsz = pool[pool[t].l].sz;
    if (k == lsz) return t;
    if (k < lsz) {
      pushdown(pool[t].l);
      idx_type llsz = pool[pool[pool[t].l].l].sz;
      if (k == llsz) {
        pool[t].l = splay(pool[t].l, k);
        return rotR(t);
      }
      else if (k < llsz) {
        pool[pool[t].l].l = splay(pool[pool[t].l].l, k);
        return rotR(rotR(t));
      }
      else {
        pool[pool[t].l].r = splay(pool[pool[t].l].r, k - llsz - 1);
        pool[t].l = rotL(pool[t].l);
        return rotR(t);
      }
    }
    else {
      k -= lsz + 1;
      pushdown(pool[t].r);
      idx_type rlsz = pool[pool[pool[t].r].l].sz;
      if (k == rlsz) {
        pool[t].r = splay(pool[t].r, k);
        return rotL(t);
      }
      else if (k < rlsz) {
        pool[pool[t].r].l = splay(pool[pool[t].r].l, k);
        pool[t].r = rotR(pool[t].r);
        return rotL(t);
      }
      else {
        pool[pool[t].r].r = splay(pool[pool[t].r].r, k - rlsz - 1);
        return rotL(rotL(t));
      }
    }
  }

  static idx_type merge_node(idx_type l, idx_type r) {
    if (!l) return r;
    if (!r) return l;
    l = splay(l, pool[l].sz - 1);
    pool[l].r = r;
    return pushup(l);
  }

  static std::pair<idx_type, idx_type> split_node(idx_type t, idx_type k) {
    if (!k) return {0, t};
    if (k >= pool[t].sz) return {t, 0};
    t = splay(t, k);
    idx_type l = pool[t].l;
    pool[t].l = 0;
    return {l, pushup(t)};
  }

  static inline void init_pool() { if (pool.empty()) pool.push_back({M::e(), M::e(), M::id(), 0, 0, 0, false}); }

  static idx_type build(idx_type l, idx_type r, const std::vector<value_type>& v) {
    if (l >= r) return 0;
    idx_type m = (l + r) >> 1, t = new_node(v[m]);
    pool[t].l = build(l, m, v), pool[t].r = build(m + 1, r, v);
    return pushup(t);
  }

  static idx_type clone(idx_type t) {
    if (!t) return 0;
    idx_type nt = pool.size();
    pool.push_back(pool[t]);
    pool[nt].l = clone(pool[t].l), pool[nt].r = clone(pool[t].r);
    return nt;
  }

  SplayTreeArray(idx_type i) : root(i) {}

  public:
  SplayTreeArray() : root(0) { init_pool(); }

  explicit SplayTreeArray(const std::vector<value_type>& v) : root(0) {
    init_pool();
    if (v.size()) root = build(0, v.size(), v);
  }

  template <class Inputit>
  SplayTreeArray(Inputit first, Inputit last) : root(0) {
    init_pool();
    std::vector<value_type> v(first, last);
    if (v.size()) root = build(0, v.size(), v);
  }

  SplayTreeArray(std::initializer_list<value_type> init) : root(0) {
    init_pool();
    if (init.size()) {
      std::vector<value_type> v(init);
      root = build(0, v.size(), v);
    }
  }

  SplayTreeArray(const SplayTreeArray& other) : root(0) { init_pool(); root = clone(other.root); }

  SplayTreeArray(SplayTreeArray&& other) noexcept : root(other.root) { other.root = 0; }

  ~SplayTreeArray() { root = 0; }

  SplayTreeArray& operator=(const SplayTreeArray& other) {
    if (this != &other) root = clone(other.root);
    return *this;
  }

  SplayTreeArray& other(SplayTreeArray&& other) noexcept {
    if (this != &other) root = other.root, other.root = 0;
    return *this;
  }

  // 現在の列の大きさを返す
  // O(1) time
  idx_type size() const { return pool[root].sz; }

  // 列が空かどうかを返す
  // O(1) time
  bool empty() const { return !pool[root].sz; }

  // 自身を [0, k)，返り値を [k, |T|) として列を分割
  // - k < |T|
  // O(log|T|) time
  SplayTreeArray split(idx_type k) {
    auto [l, r] = split_node(root, k);
    root = l;
    return SplayTreeArray(r);
  }

  // other を自身の先頭に結合し，other を空にする
  // - k < |T|
  // O(log|T|) time
  void merge_front(SplayTreeArray& other) {
    root = merge_node(other.root, root);
    other.root = 0;
  }

  void merge_front(SplayTreeArray&& other) {
    root = merge_node(other.root, root);
    other.root = 0;
  }

  // other を自身の末尾に結合し，other を空にする
  // - k < |T|
  // O(log|T|) time
  void merge_back(SplayTreeArray& other) {
    root = merge_node(root, other.root);
    other.root = 0;
  }

  void merge_back(SplayTreeArray&& other) {
    root = merge_node(root, other.root);
    other.root = 0;
  }

  // k 番目の要素として値を挿入（k>=|T| なら末尾）
  // - k <= |T|
  // O(log|T|) time
  void insert(idx_type k, const value_type& x) {
    auto [l, r] = split_node(root, k);
    root = merge_node(merge_node(l, new_node(x)), r);
  }

  // k 番目の要素を削除
  // O(log|T|) time
  void erase(idx_type k) {
    if (k >= pool[root].sz) return;
    auto [l0, r0] = split_node(root, k + 1);
    auto [l1, l2] = split_node(l0, k);
    root = merge_node(l1, r0);
  }

  // k 番目の要素を変更
  // - k < |T|
  // O(log|T|) time
  void set(idx_type k, const value_type& x) {
    assert(k < pool[root].sz);
    root = splay(root, k);
    pool[root].val = x;
    pushup(root);
  }

  // k 番目の要素に f を作用させる
  // - k < |T|
  // O(log|T|) time
  void apply(idx_type k, const operator_type& f) {
    assert(k < pool[root].sz);
    root = splay(root, k);
    pool[root].val = M::mapping(f, pool[root].val);
    pushup(root);
  }

  // [l, r) に f を作用させる
  // - l <= r
  // O(log|T|) time
  void apply(idx_type l, idx_type r, const operator_type& f) {
    assert(l <= r && r <= pool[root].sz);
    auto [l0, r0] = split_node(root, r);
    auto [l1, l2] = split_node(l0, l);
    propagate(l2, f);
    root = merge_node(merge_node(l1, l2), r0);
  }

  // [l, r) での値の総積を返す
  // - l <= r
  // O(log|T|) time
  value_type fold(idx_type l, idx_type r) {
    assert(l <= r && r <= pool[root].sz);
    auto [l0, r0] = split_node(root, r);
    auto [l1, l2] = split_node(l0, l);
    value_type res = pool[l2].acc;
    root = merge_node(merge_node(l1, l2), r0);
    return res;
  }

  // [l, r) を反転させる
  // - l <= r
  // O(log|T|) time
  void reverse(idx_type l, idx_type r) {
    assert(l <= r && r <= pool[root].sz);
    auto [l0, r0] = split_node(root, r);
    auto [l1, l2] = split_node(l0, l);
    toggle(l2);
    root = merge_node(merge_node(l1, l2), r0);
  }
};

template <IsReversibleActedMonoid M>
std::vector<typename SplayTreeArray<M>::Node> SplayTreeArray<M>::pool;
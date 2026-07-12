#pragma once
#include "../utility/Algebra.hpp"

// Segment Tree
//
// モノイドを載せると一点更新・区間集約ができる
template <IsMonoid M>
class Segtree {
  public:
  using value_type = M::value_type;
  using idx_type = unsigned int;

  private:
  idx_type N, sz;
  std::vector<value_type> dat;

  void pushup(idx_type idx) { dat[idx] = M::op(dat[idx * 2], dat[idx * 2 + 1]); }

  public:
  Segtree() {}

  explicit Segtree(idx_type N_) : Segtree(std::vector<value_type>(N_, M::e())) {}

  explicit Segtree(idx_type N_, const value_type &x) : Segtree(std::vector<value_type>(N_, x)) {}

  // v から Segtree を構築
  // O(|v|) time
  explicit Segtree(const std::vector<value_type> &v) : N(v.size()) {
    sz = std::bit_ceil(N);
    dat.assign(sz * 2, M::e());

    for (idx_type i = 0; i < N; ++i) dat[sz + i] = v[i];
    for (idx_type i = sz - 1; i >= 1; --i) pushup(i);
  }

  // 指定した位置の値を変更
  //
  // - idx < N
  //
  // O(logN) time
  void set(idx_type idx, const value_type &x) {
    assert(idx < N);
    idx += sz;
    dat[idx] = x;
    while (idx > 1) {
      idx >>= 1;
      pushup(idx);
    }
  }

  // 指定した位置に値を加算
  //
  // - S に operator+= が定義されている
  // - idx < N
  //
  // O(logN) time
  void add(idx_type idx, const value_type &x) {
    assert(idx < N);
    idx += sz;
    dat[idx] += x;
    while (idx > 1) {
      idx >>= 1;
      pushup(idx);
    }
  }

  // 区間 [l, r) の値の総積を返す
  //
  // - l <= r <= N
  //
  // O(logN) time
  value_type fold(idx_type l, idx_type r) const {
    assert(l <= r && r <= N);
    value_type resl = M::e(), resr = M::e();

    for (l += sz, r += sz; l < r; l >>= 1, r >>= 1) {
      if (l & 1) resl = M::op(resl, dat[l++]);
      if (r & 1) resr = M::op(dat[--r], resr);
    }

    return M::op(resl, resr);
  }

  // 列全体の値の総積を返す
  // O(1) time
  value_type all_fold() const noexcept { return dat[1]; }

  // 指定した位置の値を返す
  // O(1) time
  value_type get(idx_type idx) const {
    assert(idx < N);
    return dat[idx + sz];
  }

  // 指定した位置の値を返す（境界チェックなし）
  value_type operator[](idx_type idx) const noexcept { return dat[idx + sz]; }

  // f が単調なとき，f(fold(l, r)) が true になる最大の r を返す
  //
  // - l <= N
  // - f(e()) = true
  //
  // O(logN) time
  template<typename F>
  idx_type max_right(idx_type l, const F &f) const {
    assert(l <= N);
    assert(f(M::e()));
    if (l == N) return N;
    l += sz;
    value_type prod = M::e();

    do {
      while (!(l & 1)) l >>= 1;
      if (!f(M::op(prod, dat[l]))) {
        while (l < sz) {
          l <<= 1;
          if (f(M::op(prod, dat[l]))) prod = M::op(prod, dat[l++]);
        }
        return l - sz;
      }
      prod = M::op(prod, dat[l++]);
    } while ((l & -l) != l);
    
    return N;
  }
  
  // f が単調なとき，f(fold(l, r)) が true になる最小の l を返す
  //
  // - r <= N
  // - f(e()) = true
  //
  // O(logN) time
  template <typename F>
  idx_type min_left(idx_type r, const F &f) const {
    assert(r <= N);
    assert(f(M::e()));
    if (r == 0) return 0;
    r += sz;
    value_type prod = M::e();

    do {
      --r;
      while (r > 1 && (r & 1)) r >>= 1;
      if (!f(M::op(dat[r], prod))) {
        while (r < sz) {
          r = r * 2 + 1;
          if (f(M::op(dat[r], prod))) prod = M::op(dat[r--], prod);
        }
        return r + 1 - sz;
      }
      prod = M::op(dat[r], prod);
    } while ((r & -r) != r);

    return 0;
  }
};
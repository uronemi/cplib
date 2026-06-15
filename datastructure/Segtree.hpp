#pragma once
#include "../utility/Template.hpp"

// Segment Tree
//
// モノイドを載せると一点更新・区間集約ができる
template <
  typename S,
  auto op,
  auto e
>
class Segtree {
  private:
  unsigned int N, sz;
  std::vector<S> dat;

  void pushup(unsigned int idx) { dat[idx] = op(dat[idx * 2], dat[idx * 2 + 1]); }

  public:
  Segtree() {}

  explicit Segtree(unsigned int N_) : Segtree(std::vector<S>(N_, e())) {}

  explicit Segtree(unsigned int N_, const S &x) : Segtree(std::vector<S>(N_, x)) {}

  // v から Segtree を構築
  // O(|v|) time
  explicit Segtree(const std::vector<S> &v) : N(v.size()) {
    sz = std::bit_ceil(N);
    dat.assign(sz * 2, e());

    for (unsigned int i = 0; i < N; ++i) dat[sz + i] = v[i];
    for (unsigned int i = sz - 1; i >= 1; --i) pushup(i);
  }

  // 指定した位置の値を変更
  // - idx < N
  // O(logN) time
  void set(unsigned int idx, const S &x) {
    assert(idx < N);
    idx += sz;
    dat[idx] = x;
    while (idx > 1) {
      idx >>= 1;
      pushup(idx);
    }
  }

  // 指定した位置に値を加算
  // - S に operator+= が定義されている
  // - idx < N
  // O(logN) time
  void add(unsigned int idx, const S &x) {
    assert(idx < N);
    idx += sz;
    dat[idx] += x;
    while (idx > 1) {
      idx >>= 1;
      pushup(idx);
    }
  }

  // 区間 [l, r) の値の総積を返す
  // - l <= r <= N
  // O(logN) time
  S fold(unsigned int l, unsigned int r) const {
    assert(l <= r && r <= N);
    S resl = e(), resr = e();

    for (l += sz, r += sz; l < r; l >>= 1, r >>= 1) {
      if (l & 1) resl = op(resl, dat[l++]);
      if (r & 1) resr = op(dat[--r], resr);
    }

    return op(resl, resr);
  }

  // 列全体の値の総積を返す
  // O(1) time
  S all_fold() const noexcept { return dat[1]; }

  // 指定した位置の値を返す
  // O(1) time
  S get(unsigned int idx) const {
    assert(idx < N);
    return dat[idx + sz];
  }

  // 指定した位置の値を返す（境界チェックなし）
  S operator[](unsigned int idx) const noexcept { return dat[idx + sz]; }

  // f が単調なとき，f(fold(l, r)) が true になる最大の r を返す
  // - l <= N
  // - f(e()) = true
  // O(logN) time
  template<typename F>
  unsigned int max_right(unsigned int l, const F &f) const {
    assert(l <= N);
    assert(f(e()));
    if (l == N) return N;
    l += sz;
    S prod = e();

    do {
      while (!(l & 1)) l >>= 1;
      if (!f(op(prod, dat[l]))) {
        while (l < sz) {
          l <<= 1;
          if (f(op(prod, dat[l]))) prod = op(prod, dat[l++]);
        }
        return l - sz;
      }
      prod = op(prod, dat[l++]);
    } while ((l & -l) != l);
    
    return N;
  }
  
  // f が単調なとき，f(fold(l, r)) が true になる最小の l を返す
  // - r <= N
  // - f(e()) = true
  // O(logN) time
  template <typename F>
  unsigned int min_left(unsigned int r, const F &f) const {
    assert(r <= N);
    assert(f(e()));
    if (r == 0) return 0;
    r += sz;
    S prod = e();

    do {
      --r;
      while (r > 1 && (r & 1)) r >>= 1;
      if (!f(op(dat[r], prod))) {
        while (r < sz) {
          r = r * 2 + 1;
          if (f(op(dat[r], prod))) prod = op(dat[r--], prod);
        }
        return r + 1 - sz;
      }
      prod = op(dat[r], prod);
    } while ((r & -r) != r);

    return 0;
  }
};
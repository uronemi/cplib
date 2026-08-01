#pragma once
#include "../utility/Algebra.hpp"
#include "SuccintBitVector.hpp"
#include "Segtree.hpp"
#include "../utility/bit.hpp"

// Wavelet Matrix
//
// 静的な非負整数列にいろいろできるデータ構造。可換モノイドを一緒に持つ実装。
template <typename Key, IsCommutativeMonoid M>
class WaveletMatrix {
  private:
  using key_type = Key;
  using value_type = M::value_type;
  using idx_type = unsigned int;
  std::vector<SuccintBitVector> bits;
  std::vector<Segtree<M>> sgs;
  std::vector<std::vector<idx_type>> rev_idx;
  std::vector<key_type> A;
  std::vector<value_type> V;
  idx_type N, H;

  public:
  WaveletMatrix() = default;

  explicit WaveletMatrix(idx_type n) : N(n), A(n), V(n) {}

  explicit WaveletMatrix(const std::vector<key_type>& a)
    : WaveletMatrix(a, std::vector<value_type>(a.size(), M::e())) {}

  explicit WaveletMatrix(const std::vector<key_type>& a, const std::vector<value_type>& v)
    : N(a.size()), A(a), V(v) {
    build();
  }

  // Wavelet Matrix を構築する
  // O(NH) time
  void build() {
    H = internal::ceil_log2(*std::max_element(std::begin(A), std::end(A))) + 1;
    std::vector<idx_type> L(N), R(N), idx(N);
    std::vector<value_type> X(N);
    std::iota(std::begin(idx), std::end(idx), 0);
    bits.assign(H, SuccintBitVector(N));
    rev_idx.assign(H, std::vector<idx_type>(N));

    for (idx_type h = H; h--;) {
      idx_type l = 0, r = 0;
      for (idx_type i = 0; i < N; ++i) {
        if ((A[idx[i]] >> h) & 1) bits[h].set(i), R[r++] = idx[i];
        else L[l++] = idx[i];
      }
      bits[h].build();
      idx.swap(L);
      for (idx_type i = 0; i < r; ++i) idx[i + l] = R[i];
      for (idx_type i = 0; i < N; ++i) X[i] = V[idx[i]];
      sgs.emplace_back(X);
      for (idx_type i = 0; i < N; ++i) rev_idx[h][idx[i]] = i;
    }
    std::reverse(std::begin(sgs), std::end(sgs));
  }

  // A[i] を key に変更する
  //
  // - i < N && key >= 0
  //
  // O(1) time
  void set_key(idx_type i, const key_type& key) {
    assert(i < N && key >= 0);
    A[i] = key;
  }

  // A[i] を返す
  //
  // - i < N
  //
  // O(1) time
  Key access_key(idx_type i) const {
    assert(i < N);
    return A[i];
  }

  // V[i] を val に変更する
  //
  // - i < N
  // - build は実行済みであること
  //
  // O(HlogN) time
  void set_val(idx_type i, const value_type val) {
    assert(i < N);
    V[i] = val;
    for (idx_type j = 0; j < H; ++j) sgs[j].set(rev_idx[j][i], val);
  }

  // V[i] を返す
  //
  // - i < N
  //
  // O(1) time
  value_type access_val(idx_type i) const {
    assert(i < N);
    return V[i];
  }

  // A[i] = x なる i \in [l, r) の個数を返す
  //
  // - l <= r <= N
  //
  // O(H) time
  unsigned int range_count(idx_type l, idx_type r, const key_type& x) const {
    assert(l <= r && r <= N);
    for (idx_type h = H; h--;) {
      idx_type l0 = bits[h].rank0(l), r0 = bits[h].rank0(r);
      if ((x >> h) & 1) l += bits[h].count0() - l0, r += bits[h].count0() - r0;
      else l = l0, r = r0;
    }
    return r - l;
  }

  // A[i] \in [0, ub) なる i \in [l, r) の個数を返す
  //
  // - l <= r <= N && ub >= 0
  //
  // O(H) time
  unsigned int range_freq(idx_type l, idx_type r, const key_type& ub) const {
    assert(l <= r && r <= N && ub >= 0);
    unsigned int res = 0;
    for (idx_type h = H; h--;) {
      idx_type l0 = bits[h].rank0(l), r0 = bits[h].rank0(r);
      if ((ub >> h) & 1) l += bits[h].count0() - l0, r += bits[h].count0() - r0, res += r0 - l0;
      else l = l0, r = r0;
    }
    return res;
  }

  // A[i] \in [lb, ub) なる i \in [l, r) の個数を返す
  //
  // - l <= r <= N && 0 <= lb <= ub
  //
  // O(H) time
  unsigned int range_freq(idx_type l, idx_type r, const key_type& lb, const key_type& ub) const {
    assert(lb <= ub);
    return range_freq(l, r, ub) - range_freq(l, r, lb);
  }

  // A[l, r) 内の要素のうち k 番目（0-indexed）に小さい要素を返す
  //
  //  - l < r <= N
  //
  // O(H) time
  key_type kth_smallest(idx_type l, idx_type r, unsigned int k) const {
    assert(l < r && r <= N);
    key_type res = 0;
    for (idx_type h = H; h--;) {
      idx_type l0 = bits[h].rank0(l), r0 = bits[h].rank0(r);
      if (r0 - l0 <= k) {
        l += bits[h].count0() - l0;
        r += bits[h].count0() - r0;
        k -= r0 - l0;
        res |= key_type(1) << h;
      }
      else l = l0, r = r0;
    }
    return res;
  }

  // A[l, r) 内の要素のうち k 番目（0-indexed）に小さい要素を返す
  //
  //  - l < r <= N
  //
  // O(H) time
  key_type kth_largest(idx_type l, idx_type r, unsigned int k) const {
    return kth_smallest(l, r, r - l - k - 1);
  }

  // A[l, r) を安定ソートしたとき [0, k] に含まれるような i についての V[i] の fold
  //
  // - l <= r <= N
  //
  // O(HlogN) time
  value_type k_small_fold(idx_type l, idx_type r, unsigned int k) const {
    assert(l <= r && r <= N && k <= r - l);
    if (l == r || !k) return M::e();
    value_type res = M::e();
    for (idx_type h = H; h--;) {
      idx_type l0 = bits[h].rank0(l), r0 = bits[h].rank0(r);
      unsigned int c0 = r0 - l0;
      if (k >= c0) {
        if (c0 > 0) res = M::op(res, sgs[h].fold(l0, r0));
        l += bits[h].count0() - l0;
        r += bits[h].count0() - r0;
        k -= c0;
      }
      else l = l0, r = r0;
      if (!k) break;
    }
    if (k) res = M::op(res, sgs[0].fold(l, l + k));
    return res;
  }

  // A[l, r) を安定ソートしたとき [r - l - k, r - l] に含まれるような i についての V[i] の fold
  //
  // - l <= r <= N
  //
  // O(HlogN) time
  value_type k_large_fold(idx_type l, idx_type r, unsigned int k) const {
    assert(l <= r && r <= N && k <= r - l);
    if (l == r || !k) return M::e();
    value_type res = M::e();
    for (idx_type h = H; h--;) {
      idx_type l0 = bits[h].rank0(l), r0 = bits[h].rank0(r);
      idx_type l1 = l + bits[h].count0() - l0, r1 = r + bits[h].count0() - r0;
      unsigned int c1 = r1 - l1;
      if (k >= c1) {
        if (c1 > 0) res = M::op(res, sgs[h].fold(l1, r1));
        l = l0;
        r = r0;
        k -= c1;
      }
      else l = l1, r = r1;
      if (!k) break;
    }
    if (k) res = M::op(res, sgs[0].fold(r - k, r));
    return res;
  }

  // A[l, r) 内の要素のうち x 未満の最大の要素
  //
  // - l <= r <= N
  //
  // O(H) time
  key_type pred(idx_type l, idx_type r, const key_type& x) const {
    assert(l <= r && r <= N);
    unsigned int cnt = range_freq(l, r, x);
    return cnt ? kth_smallest(l, r, cnt - 1) : key_type(-1);
  }
  
  // A[l, r) 内の要素のうち x 以上の最小の要素
  //
  // - l <= r <= N
  //
  // O(H) time
  key_type succ_or_equal(idx_type l, idx_type r, const key_type& x) const {
    assert(l <= r && r <= N);
    unsigned int cnt = range_freq(l, r, x);
    return cnt == r - l ? key_type(-1) : kth_smallest(l, r, cnt);
  }
};
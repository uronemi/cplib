#pragma once

// Succint Bit Vector
//
// bit vector に対するいくつかのクエリを高速で処理するデータ構造
struct SuccintBitVector {
  private:
  std::vector<unsigned int> B, large;
  std::vector<std::uint16_t> small;
  unsigned int N, cnt1;

  public:
  explicit SuccintBitVector(unsigned int n) : N(n), cnt1(0) {
    B.assign(((n + 31) >> 5) + 1, 0);
    large.assign((n >> 9) + 1, 0);
    small.assign((n >> 5) + 1, 0);
  }

  // i 番目の bit を 1 にする
  //
  // - i < N
  //
  // O(1) time
  void set(unsigned int i) {
    assert(i < N);
    B[i >> 5] |= 1u << (i & 31);
  }

  // i 番目の bit を返す
  //
  // - i < N
  //
  // O(1) time
  unsigned int access(unsigned int i) const {
    assert(i < N);
    return (B[i >> 5] >> (i & 31)) & 1u;
  }

  // 構築
  // O(N) time
  void build() {
    for (unsigned int i = 0; i <= N; ++i) {
      if (!(i & 511)) large[i >> 9] = cnt1;
      if (!(i & 31)) {
        small[i >> 5] = cnt1 - large[i >> 9];
        if (i < N) cnt1 += __builtin_popcount(B[i >> 5]);
      }
    }
  }

  // [0, i) 内の 0 の個数
  //
  // - i <= N
  //
  // O(1) time
  unsigned int rank0(unsigned int i) const { return i - rank1(i); }

  // [0, i) 内の 1 の個数
  //
  // - i <= N
  //
  // O(1) time
  unsigned int rank1(unsigned int i) const {
    assert(i <= N);
    return large[i >> 9] + small[i >> 5] + __builtin_popcount(B[i >> 5] & ((1u << (i & 31)) - 1u));
  }

  // [0, N) 内の 0 の個数
  // O(1) time
  unsigned int count0() const noexcept { return N - cnt1; }

  // [0, N) 内の 1 の個数
  // O(1) time
  unsigned int count1() const noexcept { return cnt1; }
};
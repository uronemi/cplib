#pragma once
#include "../utility/Algebra.hpp"

// Lazy Segment Tree
//
// モノイドを載せると区間更新・区間集約ができる
template <IsActedMonoid M>
class LazySegtree {
	public:
	using value_type = M::value_type;
	using operator_type = M::operator_type;
	using idx_type = unsigned int;

	private:
	std::vector<value_type> dat;
	std::vector<operator_type> lazy;
	idx_type N, sz, log;

	void propagate(idx_type idx, const operator_type &f) {
		dat[idx] = M::mapping(f, dat[idx]);
		if (idx < sz) lazy[idx] = M::composition(f, lazy[idx]);
	}

	void pushdown(idx_type idx) {
		if (lazy[idx] == M::id()) return;
		propagate(idx * 2, lazy[idx]);
		propagate(idx * 2 + 1, lazy[idx]);
		lazy[idx] = M::id();
	}

	void merge(idx_type idx) { dat[idx] = M::op(dat[idx * 2], dat[idx * 2 + 1]); }

	void pushup(idx_type idx) {
		while (idx > 1) {
			idx >>= 1;
			merge(idx);
		}
	}

	public:
	LazySegtree() {}

	explicit LazySegtree(idx_type N_) : LazySegtree(std::vector<value_type>(N_, M::e())) {}

	LazySegtree(idx_type N_, const value_type &x) : LazySegtree(std::vector<value_type>(N_, x)) {}

	// v から Segtree を構築
	// O(|v|) time
	explicit LazySegtree(const std::vector<value_type>& v) : N(v.size()) {
		sz = std::bit_ceil(N);
		log = std::bit_width(sz - 1);
		dat.assign(sz * 2, M::e());
		lazy.assign(sz * 2, M::id());
		for (idx_type i = 0; i < N; ++i) dat[i + sz] = v[i];
		for (idx_type i = sz - 1; i >= 1; --i) merge(i);
	}

	// 指定した位置の値を変更
	//
	// - idx < N
	//
	// O(logN) time
	void set(idx_type idx, const value_type &x) {
		assert(idx < N);
		idx += sz;
		for (idx_type i = log; i >= 1; --i) pushdown(idx >> i);
		dat[idx] = x;
		pushup(idx);
	}

	// 指定した位置に値を加算
	//
	// - idx < N
	// - S に operator+= が定義されている
	//
	// O(logN) time
	void add(idx_type idx, const value_type &x) {
		assert(idx < N);
		idx += sz;
		for (idx_type i = log; i >= 1; --i) pushdown(idx >> i);
		dat[idx] += x;
		pushup(idx);
	}

	// 指定した位置に f を作用させる
	//
	// - idx < N
	//
	// O(logN) time
	void apply(int idx, const operator_type &f) {
		assert(idx < N);
		idx += sz;
		for (idx_type i = log; i >= 1; --i) pushdown(idx >> i);
		dat[idx] = M::mapping(f, dat[idx]);
		pushup(idx);
	}

	// 区間 [l, r) に f を作用させる
	//
	// - l <= r && r <= N
	//
	// O(logN) time
	void apply(idx_type l, idx_type r, const operator_type &f) {
		assert(l <= r && r <= N);
		if (l == r) return;
		l += sz, r += sz;
		for (idx_type i = log; i >= 1; --i) {
			if (((l >> i) << i) != l) pushdown(l >> i);
			if (((r >> i) << i) != r) pushdown((r - 1) >> i);
		}
		for (idx_type l2 = l, r2 = r; l2 < r2; l2 >>= 1, r2 >>= 1) {
			if (l2 & 1) propagate(l2++, f);
			if (r2 & 1) propagate(--r2, f);
		}
		for (idx_type i = 1; i <= log; ++i) {
			if (((l >> i) << i) != l) merge(l >> i);
			if (((r >> i) << i) != r) merge((r - 1) >> i);
		}
	}

	// 指定した位置の値を返す
	//
	// - idx < N
	//
	// O(logN) time
	value_type get(idx_type idx) {
		assert(idx < N);
		idx += sz;
		for (idx_type i = log; i >= 1; --i) pushdown(idx >> i);
		return dat[idx];
	}

	// 指定した位置の値を返す（境界チェックなし）
	value_type operator[](idx_type idx) noexcept {
		idx += sz;
		for (idx_type i = log; i >= 1; --i) pushdown(idx >> i);
		return dat[idx];
	}

	// 区間 [l, r) の値の総積を返す
	//
	// - l <= r && r <= N
	//
	// O(logN) time
	value_type fold(idx_type l, idx_type r) {
		assert(l <= r && r <= N);
		if (l == r) return M::e();
		l += sz, r += sz;
		for (idx_type i = log; i >= 1; --i) {
			if (((l >> i) << i) != l) pushdown(l >> i);
			if (((r >> i) << i) != r) pushdown((r - 1) >> i);
		}
		value_type resl = M::e(), resr = M::e();
		for (; l < r; l >>= 1, r >>= 1) {
			if (l & 1) resl = M::op(resl, dat[l++]);
			if (r & 1) resr = M::op(dat[--r], resr);
		}
		return M::op(resl, resr);
	}

	// 列全体の値の総積を返す
	// O(1) time
	value_type all_fold() const { return dat[1]; }

	template <auto g>
	idx_type max_right(idx_type l) {
		return max_right(l, [](const value_type& x) -> bool { return g(x); });
	}

	// f が単調のとき g(fold(l, r)) = true になる最大の r を返す
	//
	// - l <= N
	// - g(e()) = true
	//
	// O(logN) time
	template <typename G>
	idx_type max_right(idx_type l, const G& g) {
		assert(l <= N);
		assert(g(M::e()));
		if (l == N) return N;
		l += sz;
		for (idx_type i = log; i >= 1; --i) pushdown(l >> i);
		value_type prod = M::e();
		do {
			while (!(l & 1)) l >>= 1;
			if (!g(M::op(prod, dat[l]))) {
				while (l < sz) {
					pushdown(l);
					l <<= 1;
					if (g(M::op(prod, dat[l]))) prod = M::op(prod, dat[l++]);
				}
				return l - sz;
			}
			prod = M::op(prod, dat[l++]);
		} while ((l & -l) != l);
		return N;
	}

	template <auto g>
	idx_type min_left(idx_type r) {
		return min_left(r, [](const value_type& x) -> bool { return g(x); });
	}

	// f が単調のとき g(fold(l, r)) = true になる最小の l を返す
	// - r <= N
	// - g(e()) = true
	// O(logN) time
	template <typename G>
	idx_type min_left(idx_type r, const G& g) {
		assert(r <= N);
		assert(g(M::e()));
		if (r == 0) return 0;
		r += sz;
		for (idx_type i = log; i >= 1; --i) pushdown((r - 1) >> i);
		value_type prod = M::e();
		do {
			--r;
			while (r > 1 && (r & 1)) r >>= 1;
			if (!g(M::op(dat[r], prod))) {
				while (r < sz) {
					pushdown(r);
					r = r * 2 + 1;
					if (g(op(dat[r], prod))) prod = M::op(dat[r--], prod);
				}
				return r + 1 - sz;
			}
			prod = M::op(dat[r], prod);
		} while ((r & -r) != r);
		return 0;
	}
};
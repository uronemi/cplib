#pragma once

// Lazy Segment Tree
//
// モノイドを載せると区間更新・区間集約ができる
template <
	typename S,
	auto op,
	auto e,
	typename F,
	auto mapping,
	auto composition,
	auto id
>
class LazySegtree {
	private:
	std::vector<S> dat;
	std::vector<F> lazy;
	unsigned int N, sz, log;

	void propagate(unsigned int idx, const F &f) {
		dat[idx] = mapping(f, dat[idx]);
		if (idx < sz) lazy[idx] = composition(f, lazy[idx]);
	}

	void pushdown(unsigned int idx) {
		if (lazy[idx] == id()) return;
		propagate(idx * 2, lazy[idx]);
		propagate(idx * 2 + 1, lazy[idx]);
		lazy[idx] = id();
	}

	void merge(unsigned int idx) { dat[idx] = op(dat[idx * 2], dat[idx * 2 + 1]); }

	void pushup(unsigned int idx) {
		while (idx > 1) {
			idx >>= 1;
			merge(idx);
		}
	}

	public:
	LazySegtree() {}

	explicit LazySegtree(unsigned int N_) : LazySegtree(std::vector<S>(N_, e())) {}

	LazySegtree(unsigned int N_, const S &x) : LazySegtree(std::vector<S>(N_, x)) {}

	// v から Segtree を構築
	// O(|v|) time
	explicit LazySegtree(const std::vector<S>& v) : N(v.size()) {
		sz = std::bit_ceil(N);
		log = std::bit_width(sz - 1);
		dat.assign(sz * 2, e());
		lazy.assign(sz * 2, id());
		for (unsigned int i = 0; i < N; ++i) dat[i + sz] = v[i];
		for (unsigned int i = sz - 1; i >= 1; --i) dat[i] = op(dat[i * 2], dat[i * 2 + 1]);
	}

	// 指定した位置の値を変更
	// - idx < N
	// O(logN) time
	void set(unsigned int idx, const S &x) {
		assert(idx < N);
		idx += sz;
		for (unsigned int i = log; i >= 1; --i) pushdown(idx >> i);
		dat[idx] = x;
		pushup(idx);
	}

	// 指定した位置に値を加算
	// - idx < N
	// - S に operator+= が定義されている
	// O(logN) time
	void add(unsigned int idx, const S &x) {
		assert(idx < N);
		idx += sz;
		for (unsigned int i = log; i >= 1; --i) pushdown(idx >> i);
		dat[idx] += x;
		pushup(idx);
	}

	// 指定した位置に f を作用させる
	// - idx < N
	// O(logN) time
	void apply(int idx, const F &f) {
		assert(idx < N);
		idx += sz;
		for (unsigned int i = log; i >= 1; --i) pushdown(idx >> i);
		dat[idx] = mapping(f, dat[idx]);
		pushup(idx);
	}

	// 区間 [l, r) に f を作用させる
	// - l <= r && r <= N
	// O(logN) time
	void apply(unsigned int l, unsigned int r, const F &f) {
		assert(l <= r && r <= N);
		if (l == r) return;
		l += sz, r += sz;
		for (unsigned int i = log; i >= 1; --i) {
			if (((l >> i) << i) != l) pushdown(l >> i);
			if (((r >> i) << i) != r) pushdown((r - 1) >> i);
		}
		for (unsigned int l2 = l, r2 = r; l2 < r2; l2 >>= 1, r2 >>= 1) {
			if (l2 & 1) propagate(l2++, f);
			if (r2 & 1) propagate(--r2, f);
		}
		for (unsigned int i = 1; i <= log; ++i) {
			if (((l >> i) << i) != l) merge(l >> i);
			if (((r >> i) << i) != r) merge((r - 1) >> i);
		}
	}

	// 指定した位置の値を返す
	// - idx < N
	// O(logN) time
	S get(unsigned int idx) {
		assert(idx < N);
		idx += sz;
		for (unsigned int i = log; i >= 1; --i) pushdown(idx >> i);
		return dat[idx];
	}

	// 指定した位置の値を返す（境界チェックなし）
	S operator[](unsigned int idx) noexcept {
		idx += sz;
		for (unsigned int i = log; i >= 1; --i) pushdown(idx >> i);
		return dat[idx];
	}

	// 区間 [l, r) の値の総積を返す
	// - l <= r && r <= N
	// O(logN) time
	S fold(unsigned int l, unsigned int r) {
		assert(l <= r && r <= N);
		if (l == r) return e();
		l += sz, r += sz;
		for (unsigned int i = log; i >= 1; --i) {
			if (((l >> i) << i) != l) pushdown(l >> i);
			if (((r >> i) << i) != r) pushdown((r - 1) >> i);
		}
		S resl = e(), resr = e();
		for (; l < r; l >>= 1, r >>= 1) {
			if (l & 1) resl = op(resl, dat[l++]);
			if (r & 1) resr = op(dat[--r], resr);
		}
		return op(resl, resr);
	}

	// 列全体の値の総積を返す
	// O(1) time
	S all_fold() const { return dat[1]; }

	template <auto g>
	unsigned int max_right(unsigned int l) {
		return max_right(l, [](const S &x) -> bool { return g(x); });
	}

	// f が単調のとき g(fold(l, r)) = true になる最大の r を返す
	// - l <= N
	// - g(e()) = true
	// O(logN) time
	template <typename G>
	unsigned int max_right(unsigned int l, const G &g) {
		assert(l <= N);
		assert(g(e()));
		if (l == N) return N;
		l += sz;
		for (unsigned int i = log; i >= 1; --i) pushdown(l >> i);
		S prod = e();
		do {
			while (!(l & 1)) l >>= 1;
			if (!g(op(prod, dat[l]))) {
				while (l < sz) {
					pushdown(l);
					l <<= 1;
					if (g(op(prod, dat[l]))) prod = op(prod, dat[l++]);
				}
				return l - sz;
			}
			prod = op(prod, dat[l++]);
		} while ((l & -l) != l);
		return N;
	}

	template <auto g>
	unsigned int min_left(unsigned int r) {
		return min_left(r, [](const S &x) -> bool { return g(x); });
	}

	// f が単調のとき g(fold(l, r)) = true になる最小の l を返す
	// - r <= N
	// - g(e()) = true
	// O(logN) time
	template <typename G>
	unsigned int min_left(unsigned int r, const G& g) {
		assert(r <= N);
		assert(g(e()));
		if (r == 0) return 0;
		r += sz;
		for (unsigned int i = log; i >= 1; --i) pushdown((r - 1) >> i);
		S prod = e();
		do {
			--r;
			while (r > 1 && (r & 1)) r >>= 1;
			if (!g(op(dat[r], prod))) {
				while (r < sz) {
					pushdown(r);
					r = r * 2 + 1;
					if (g(op(dat[r], prod))) prod = op(dat[r--], prod);
				}
				return r + 1 - sz;
			}
			prod = op(dat[r], prod);
		} while ((r & -r) != r);
		return 0;
	}
};
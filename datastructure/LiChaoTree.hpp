#pragma once
#include "../utility/Algebra.hpp"

// Li Chao Tree
//
// Convex Hull Trick を実現するデータ構造の 1 つ。一般化して高々 1 回しか交わらない曲線を管理する。
template <
  IsLiChao L,
  L::value_type x_min,
  L::value_type x_max
>
class LiChaoTree {
  public:
  using function_type = L::function_type;
  using value_type = L::value_type;

  private:
  struct Node {
    function_type f;
    unsigned int l, r;

    Node (const function_type& F) : f(F), l(0), r(0) {}
  };

  std::vector<Node> pool;
  unsigned int root = 0;

  unsigned int new_node(const function_type& f) {
    pool.emplace_back(f);
    return pool.size() - 1;
  }

  unsigned int add_line_internal(
    unsigned int t,
    function_type f,
    value_type l,
    value_type r,
    value_type y_l,
    value_type y_r
  ) {
    unsigned int sub = t, par = 0;
    bool from_l = false;

    while (true) {
      if (!t) {
        unsigned int nt = new_node(f);
        if (!par) sub = nt;
        else if (from_l) pool[par].l = nt;
        else pool[par].r = nt;
        return sub;
      }

      value_type ty_l = L::eval(pool[t].f, l), ty_r = L::eval(pool[t].f, r);
      if (ty_l <= y_l && ty_r <= y_r) return sub;
      if (ty_l >= y_l && ty_r >= y_r) {
        pool[t].f = f;
        return sub;
      }

      value_type m = std::midpoint(l, r), ty_m = L::eval(pool[t].f, m), y_m = L::eval(f, m);
      par = t;
      if (ty_m > y_m) {
        std::swap(pool[t].f, f);
        if (y_l >= ty_l) {
          from_l = true;
          t = pool[par].l;
          r = m;
          y_r = ty_m;
          y_l = ty_l;
        }
        else {
          from_l = false;
          value_type ny_l = L::eval(f, m + 1);
          t = pool[par].r;
          l = m + 1;
          y_l = ny_l;
          y_r = ty_r;
        }
      }
      else {
        if (ty_l >= y_l) {
          from_l = true;
          t = pool[par].l;
          r = m;
          y_r = y_m;
        }
        else {
          from_l = false;
          value_type ny_l = L::eval(f, m + 1);
          t = pool[par].r;
          l = m + 1;
          y_l = ny_l;
        }
      }
    }
  }

  unsigned int add_segment_internal(
    unsigned int t,
    const function_type& f,
    const value_type& a,
    const value_type& b,
    value_type l,
    value_type r,
    value_type y_l,
    value_type y_r
  ) {
    if (r < a || b < l) return t;
    if (a <= l && r <= b) return add_line_internal(t, f, l, r, y_l, y_r);

    if (t) {
      value_type ty_l = L::eval(pool[t].f, l), ty_r = L::eval(pool[t].f, r);
      if (ty_l <= y_l && ty_r <= y_r) return t;
    }
    else t = new_node(L::e());

    value_type m = std::midpoint(l, r), y_m = L::eval(f, m);
    pool[t].l = add_segment_internal(pool[t].l, f, a, b, l, m, y_l, y_m);
    pool[t].r = add_segment_internal(pool[t].r, f, a, b, m + 1, r, L::eval(f, m + 1), y_r);
    return t;
  }

  public:
  LiChaoTree() { pool.emplace_back(L::e()); }

  void reserve(std::size_t N) { pool.reserve(N + 1); }

  // [x_min, x_max) 上で定義された曲線 y=f(x) を追加する
  // O(log(x_max-x_min)) time
  void add_line(const function_type& f) {
    root = add_line_internal(root, f, x_min, x_max - 1, L::eval(f, x_min), L::eval(f, x_max - 1));
  }

  // [a, b) で定義された曲線 y=f(x) を追加する
  // O(log(x_max-x_min)) time
  void add_segment(const function_type& f, const value_type& a, const value_type& b) {
    function_type f_ = f;
    root = add_segment_internal(root, f_, a, b - 1, x_min, x_max - 1, L::eval(f, x_min), L::eval(f, x_max - 1));
  }

  // x=p における y の最小値を返す
  //
  // - x_min <= p < x_max
  //
  // O(log(x_max-x_min)) time
  value_type query(const value_type& p) {
    assert(x_min <= p && p < x_max);
    value_type l = x_min, r = x_max - 1;
    unsigned int t = root;
    if (!t) return L::eval(L::e(), p);

    value_type res = L::eval(pool[t].f, p);
    while (l < r) {
      value_type m = std::midpoint(l, r);
      unsigned int nt;
      if (p <= m) nt = pool[t].l, r = m;
      else nt = pool[t].r, l = m + 1;
      if (!nt) break;
      t = nt;
      res = std::min(res, L::eval(pool[t].f, p));
    }
    return res;
  }
};
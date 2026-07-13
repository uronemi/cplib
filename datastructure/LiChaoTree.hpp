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
    Node *l, *r;

    Node (const function_type& F) : f(F), l(nullptr), r(nullptr) {}
  };
  
  Node *root = nullptr;

  Node* add_line_internal(
    Node *t,
    function_type& f,
    const value_type& l,
    const value_type& r,
    const value_type& y_l,
    const value_type& y_r
  ) {
    if (!t) return new Node(f);
    value_type ty_l = L::eval(t->f, l), ty_r = L::eval(t->f, r);
    if (ty_l <= y_l && ty_r <= y_r) return t;
    else if (ty_l >= y_l && ty_r >= y_r) {
      t->f = f;
      return t;
    }

    value_type m = std::midpoint(l, r);
    value_type ty_m = L::eval(t->f, m), y_m = L::eval(f, m);
    if (ty_m > y_m) {
      std::swap(t->f, f);
      if (y_l >= ty_l) t->l = add_line_internal(t->l, f, l, m, ty_l, ty_m);
      else t->r = add_line_internal(t->r, f, m + 1, r, L::eval(f, m + 1), ty_r);
    }
    else {
      if (ty_l >= y_l) t->l = add_line_internal(t->l, f, l, m, y_l, y_m);
      else t->r = add_line_internal(t->r, f, m + 1, r, L::eval(f, m + 1), y_r);
    }

    return t;
  }

  Node* add_segment_internal(
    Node *t,
    function_type f,
    const value_type& a,
    const value_type& b,
    const value_type& l,
    const value_type& r,
    const value_type& y_l,
    const value_type& y_r
  ) {
    if (r < a || b < l) return t;
    if (a <= l && r <= b) return add_line_internal(t, f, l, r, y_l, y_r);
    if (t) {
      value_type ty_l = L::eval(t->f, l), ty_r = L::eval(t->f, r);
      if (ty_l <= y_l && ty_r <= y_r) return t;
    }
    else t = new Node(L::e());

    value_type m = std::midpoint(l, r);
    value_type y_m = L::eval(f, m);
    t->l = add_segment_internal(t->l, f, a, b, l, m, y_l, y_m);
    t->r = add_segment_internal(t->r, f, a, b, m + 1, r, L::eval(f, m + 1), y_r);
    return t;
  }

  value_type query_internal(const Node* t, const value_type& l, const value_type& r, const value_type& p) {
    if (!t) return L::eval(L::e(), p);
    if (l == r) return L::eval(t->f, p);

    value_type m = std::midpoint(l, r);
    if (p <= m) return std::min(L::eval(t->f, p), query_internal(t->l, l, m, p));
    else return std::min(L::eval(t->f, p), query_internal(t->r, m + 1, r, p));
  }

  public:
  LiChaoTree() = default;

  // [x_min, x_max) 上で定義された曲線 y=f(x) を追加する
  // O(log(x_max-x_min)) time
  void add_line(const function_type& f) {
    function_type f_ = f;
    root = add_line_internal(root, f_, x_min, x_max - 1, L::eval(f, x_min), L::eval(f, x_max - 1));
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
    return query_internal(root, x_min, x_max - 1, p);
  }
};
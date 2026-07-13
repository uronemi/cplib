#pragma once

// Line Container
//
// Convex Hull Trick を実現するデータ構造の 1 つ。線分は追加できないが，クエリ先読みが不要。
namespace internal {

struct Line {
  const long long a, b;
  mutable long long p;
  
  bool operator<(const Line& rhs) const { return a < rhs.a; }
  bool operator<(long long x) const { return p < x; }
};

};

template <const bool FIND_MIN = true>
class LineContainer : std::multiset<internal::Line, std::less<>> {
  private:
  static const long long INF = 9223372036854775807LL;

  long long floor_div(long long a, long long b) { return a / b - ((a ^ b) < 0 && a % b); }

  bool intersect(iterator x, iterator y) {
    if (y == end()) {
      x->p = INF;
      return 0;
    }
    if (x->a == y->a) x->p = (x->b > y->b ? INF : -INF);
    else x->p = floor_div(y->b - x->b, x->a - y->a);
    return x->p >= y->p;
  }

  public:
  void add(long long a, long long b) {
    if constexpr (FIND_MIN) a = -a, b = -b;
    auto z = insert({a, b, 0}), y = z++, x = y;
    while (intersect(y, z)) z = erase(z);
    if (x != begin() && intersect(--x, y)) intersect(x, y = erase(y));
    while ((y = x) != begin() && (--x)->p >= y->p) intersect(x, erase(y));
  }

  long long query(long long x) {
    assert(size());
    auto l = *lower_bound(x);
    long long res = l.a * x + l.b;
    if constexpr (FIND_MIN) return -res;
    return res;
  }
};
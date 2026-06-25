#pragma once

void yn(bool f) { println(f ? "yes" : "no"); }

void Yn(bool f) { println(f ? "Yes" : "No"); }

void YN(bool f) { println(f ? "YES" : "NO"); }

template <
  typename T,
  typename... Ts
>
bool chmax(T& x, const Ts... y) {
  T old = x;
  ((x = std::max(x, static_cast<T>(y))), ...);
  return x != old;
}

template <
  typename T,
  typename... Ts
>
bool chmin(T& x, const Ts... y) {
  T old = x;
  ((x = std::min(x, static_cast<T>(y))), ...);
  return x != old;
}

template <
  typename T,
  typename... ints
>
bool argchmax(const std::vector<T>& v, int& x, ints... y) {
  int old = x;
  auto update = [&](int idx) { if (v[x] < v[idx]) x = idx; };
  (update(static_cast<int>(y)), ...);
  return x != old;
}

template <
  typename T,
  typename... ints
>
bool argchmin(const std::vector<T>& v, int& x, ints... y) {
  int old = x;
  auto update = [&](int idx) { if (v[x] > v[idx]) x = idx; };
  (update(static_cast<int>(y)), ...);
  return x != old;
}

template <typename T>
void compress(std::vector<T>& v) {
  std::vector<T> sorted = v;
  std::sort(sorted.begin(), sorted.end());
  sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
  for (int i = 0; i < v.size(); ++i) v[i] = std::lower_bound(sorted.begin(), sorted.end(), v[i]) - sorted.begin();
}

template <typename T>
T pow(T e, unsigned long long N) {
  T res = 1;
  while (N) {
    if (N & 1) res *= e;
    e *= e;
    N >>= 1;
  }
  return res;
}

template <typename T = char>
void print_raw(const std::vector<T>& v, bool endline = true) {
  for (const auto& e : v) print_ns(e);
  if (endline) println();
}

template <typename T = char>
void print_raw2d(const std::vector<std::vector<T>>& v) {
  for (const std::vector<T>& v_r : v) {
    for (const T& e : v_r) print_ns(e);
    println();
  }
}
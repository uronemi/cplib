#pragma once

using i32 = int;
using u32 = unsigned int;
using usize = unsigned int;
using i64 = long long;
using u64 = unsigned long long;
using i128 = __int128_t;
using u128 = __uint128_t;
using f32 = float;
using f64 = double;
using f128 = long double;
using String = std::string;
template <typename T>
using Vec = std::vector<T>;
template <typename T>
using Maxpq = std::priority_queue<T>;
template <typename T>
using Minpq = std::priority_queue<T, std::vector<T>, std::greater<T>>;
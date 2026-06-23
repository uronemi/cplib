#pragma once
#include <bits/stdc++.h>
#include "Fastio.hpp"
#include "Modint.hpp"

template <typename T>
using Maxpq = std::priority_queue<T>;
template <typename T>
using Minpq = std::priority_queue<T, std::vector<T>, std::greater<T>>;
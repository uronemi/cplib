#pragma once

unsigned long long xor_shift() {
  static unsigned long long x = []() {
    unsigned long long seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    seed ^= 9845834732710364265ULL;
    seed ^= seed << 24, seed ^= seed >> 31, seed ^= seed << 35;
    return seed;
  }();
  return x ^= x << 13, x ^= x >> 7, x ^= x << 17;
}
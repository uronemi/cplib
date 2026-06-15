#pragma once

// binomial table
//
// binom(n, k) (0 <= k <= n <= N) を計算する
// O(N^2) time
template <typename T>
std::vector<std::vector<T>> binomial_table(unsigned int N) {
  std::vector<std::vector<T>> dp(N + 1, std::vector<T>(N + 1, T{0}));
  dp[0][0] = 1;
  for (unsigned int i = 0; i < N; ++i) {
    for (unsigned int j = 0; j <= i; ++j) {
      dp[i + 1][j] += dp[i][j];
      dp[i + 1][j + 1] += dp[i][j];
    }
  }
  return dp;
}
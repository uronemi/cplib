#pragma once

// Z-algorithm
//
// Z[i] = (S と S[i:] の LCS 長) となるような配列 Z を返す
// O(|S|) time
std::vector<unsigned int> z_algorithm(const std::string& S) {
  unsigned int N = S.size();
  std::vector<unsigned int> res(N);
  res[0] = N;

  for (unsigned int i = 1, len = 0; i < N;) {
    while (i + len < N && S[len] == S[i + len]) len++;
    res[i] = len;

    if (!len) i++;
    else {
      unsigned int j = 1;
      while (j < len && j + res[j] < len) res[i + j] = res[j], j++;

      i += j, len -= j;
    }
  }

  return res;
}
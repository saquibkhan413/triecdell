#pragma once

#include <string_view>
#include <vector>
#include <algorithm>
#include <cstdint>

namespace spell {

class EditDistance {
public:
    // Level 2: Standard Levenshtein Distance using Dynamic Programming
    // O(N * M) time, O(min(N, M)) space
    static uint32_t levenshtein(std::string_view s1, std::string_view s2) {
        size_t n = s1.size();
        size_t m = s2.size();

        if (n == 0) return static_cast<uint32_t>(m);
        if (m == 0) return static_cast<uint32_t>(n);

        // Keep s2 as the shorter string to minimize memory
        if (n < m) {
            std::swap(s1, s2);
            std::swap(n, m);
        }

        std::vector<uint32_t> prev(m + 1);
        std::vector<uint32_t> curr(m + 1);

        for (uint32_t j = 0; j <= m; ++j) {
            prev[j] = j;
        }

        for (size_t i = 1; i <= n; ++i) {
            curr[0] = static_cast<uint32_t>(i);
            for (size_t j = 1; j <= m; ++j) {
                uint32_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
                curr[j] = std::min({
                    curr[j - 1] + 1,        // Insertion
                    prev[j] + 1,            // Deletion
                    prev[j - 1] + cost      // Substitution
                });
            }
            prev = curr;
        }

        return prev[m];
    }

    // Damerau-Levenshtein: adds adjacent transpositions (e.g. "teh" -> "the" = 1 operation)
    static uint32_t damerau_levenshtein(std::string_view s1, std::string_view s2) {
        size_t n = s1.size();
        size_t m = s2.size();

        if (n == 0) return static_cast<uint32_t>(m);
        if (m == 0) return static_cast<uint32_t>(n);

        std::vector<std::vector<uint32_t>> dp(n + 1, std::vector<uint32_t>(m + 1, 0));

        for (size_t i = 0; i <= n; ++i) dp[i][0] = static_cast<uint32_t>(i);
        for (size_t j = 0; j <= m; ++j) dp[0][j] = static_cast<uint32_t>(j);

        for (size_t i = 1; i <= n; ++i) {
            for (size_t j = 1; j <= m; ++j) {
                uint32_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
                dp[i][j] = std::min({
                    dp[i - 1][j] + 1,       // Deletion
                    dp[i][j - 1] + 1,       // Insertion
                    dp[i - 1][j - 1] + cost // Substitution
                });

                // Transposition check
                if (i > 1 && j > 1 && s1[i - 1] == s2[j - 2] && s1[i - 2] == s2[j - 1]) {
                    dp[i][j] = std::min(dp[i][j], dp[i - 2][j - 2] + 1);
                }
            }
        }

        return dp[n][m];
    }
};

} // namespace spell

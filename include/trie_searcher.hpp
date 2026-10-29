#pragma once

#include "trie.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <cstdint>

namespace spell {

struct CandidateWord {
    std::string word;
    uint32_t edit_distance{0};
    uint32_t frequency{0};

    bool operator<(const CandidateWord& o) const {
        // Primary sort: lower edit distance
        if (edit_distance != o.edit_distance) {
            return edit_distance < o.edit_distance;
        }
        // Secondary sort: higher frequency
        return frequency > o.frequency;
    }
};

class TrieSearcher {
private:
    static void search_recursive(
        const TrieNode* node,
        char ch,
        std::string_view target,
        const std::vector<uint32_t>& previous_row,
        uint32_t max_cost,
        std::vector<CandidateWord>& results
    ) {
        size_t m = target.size();
        std::vector<uint32_t> current_row(m + 1);
        current_row[0] = previous_row[0] + 1;

        for (size_t i = 1; i <= m; ++i) {
            uint32_t insert_cost = current_row[i - 1] + 1;
            uint32_t delete_cost = previous_row[i] + 1;
            uint32_t replace_cost = previous_row[i - 1] + (std::tolower(static_cast<unsigned char>(target[i - 1])) == ch ? 0 : 1);

            current_row[i] = std::min({insert_cost, delete_cost, replace_cost});
        }

        // If this node completes a valid dictionary word and satisfies edit distance threshold
        if (node->is_terminal && current_row[m] <= max_cost) {
            results.push_back({node->word, current_row[m], node->frequency});
        }

        // Pruning condition: if the minimum element in current_row exceeds max_cost,
        // no word down this entire branch can possibly match within max_cost.
        uint32_t min_row_val = *std::min_element(current_row.begin(), current_row.end());
        if (min_row_val <= max_cost) {
            for (size_t c = 0; c < 26; ++c) {
                if (node->children[c]) {
                    char child_char = static_cast<char>('a' + c);
                    search_recursive(node->children[c].get(), child_char, target, current_row, max_cost, results);
                }
            }
        }
    }

public:
    // Level 4: Trie-based approximate matching with Dynamic Programming pruning
    // Does NOT scan the entire dictionary; prunes branches in O(Trie pruned path)
    static std::vector<CandidateWord> find_approximate_matches(
        const Trie& trie,
        std::string_view target,
        uint32_t max_cost = 2
    ) {
        std::vector<CandidateWord> results;
        if (target.empty() || !trie.root()) return results;

        size_t m = target.size();
        std::vector<uint32_t> initial_row(m + 1);
        for (size_t i = 0; i <= m; ++i) {
            initial_row[i] = static_cast<uint32_t>(i);
        }

        const TrieNode* root = trie.root();
        for (size_t c = 0; c < 26; ++c) {
            if (root->children[c]) {
                char child_char = static_cast<char>('a' + c);
                search_recursive(root->children[c].get(), child_char, target, initial_row, max_cost, results);
            }
        }

        return results;
    }
};

} // namespace spell

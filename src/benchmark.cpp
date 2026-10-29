#include "spell_checker.hpp"
#include "edit_distance.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <numeric>

using namespace spell;

void benchmark_trie_lookup(const SpellChecker& checker) {
    std::cout << "\n=======================================================\n";
    std::cout << "  1. Level 1: Trie Exact Search Performance\n";
    std::cout << "=======================================================\n";

    std::vector<std::string> test_words = {
        "apple", "computer", "algorithm", "database", "hello", "world",
        "zebra", "nonexistentword", "misspelling", "programming"
    };

    auto t0 = std::chrono::high_resolution_clock::now();
    size_t iterations = 100000;
    size_t found_count = 0;

    for (size_t i = 0; i < iterations; ++i) {
        const auto& w = test_words[i % test_words.size()];
        if (checker.is_word_correct(w)) found_count++;
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    double total_us = std::chrono::duration<double, std::micro>(t1 - t0).count();
    double ns_per_lookup = (total_us * 1000.0) / iterations;

    std::cout << "  - Iterations:       " << iterations << " (" << found_count << " hits)\n";
    std::cout << "  - Total Time:       " << total_us / 1000.0 << " ms\n";
    std::cout << "  - Latency/lookup:   " << std::fixed << std::setprecision(1) << ns_per_lookup << " ns/op\n";
    std::cout << "  - Throughput:       " << (iterations / (total_us / 1000000.0)) << " lookups/second\n";
}

void benchmark_naive_vs_trie_pruning(SpellChecker& checker) {
    std::cout << "\n=======================================================\n";
    std::cout << "  2. Level 2 (Full Scan DP) vs Level 4 (Trie-Guided DP Pruning)\n";
    std::cout << "=======================================================\n";

    std::string typo = "aple";
    uint32_t max_dist = 2;

    // A. Naive Full Dictionary Scan:
    // Computes Levenshtein edit distance for EVERY single word in the dictionary
    auto all_words = checker.trie().autocomplete("", 100000);

    auto t0 = std::chrono::high_resolution_clock::now();
    std::vector<CandidateWord> naive_candidates;
    for (int iter = 0; iter < 100; ++iter) {
        naive_candidates.clear();
        for (const auto& [w, freq] : all_words) {
            uint32_t d = EditDistance::levenshtein(typo, w);
            if (d <= max_dist) {
                naive_candidates.push_back({w, d, freq});
            }
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double naive_ms = std::chrono::duration<double, std::milli>(t1 - t0).count() / 100.0;

    // B. Level 4: Trie-Guided DP Approximate Matching
    // Prunes invalid Trie branches, never visiting subtrees exceeding max_dist
    t0 = std::chrono::high_resolution_clock::now();
    std::vector<CandidateWord> trie_candidates;
    for (int iter = 0; iter < 100; ++iter) {
        trie_candidates = TrieSearcher::find_approximate_matches(checker.trie(), typo, max_dist);
    }
    t1 = std::chrono::high_resolution_clock::now();
    double trie_ms = std::chrono::duration<double, std::milli>(t1 - t0).count() / 100.0;

    std::cout << "Target typo: \"" << typo << "\" (Max edit distance: " << max_dist << ")\n";
    std::cout << "Dictionary words inspected: " << all_words.size() << "\n\n";
    std::cout << "  - Naive Full-Scan DP Latency:   " << std::fixed << std::setprecision(3) << naive_ms << " ms\n";
    std::cout << "  - Trie-Guided DP Pruned Latency:" << trie_ms << " ms (" 
              << trie_ms * 1000.0 << " us)\n";
    std::cout << "  - Level 4 Pruning Speedup:      " << std::setprecision(1) << (naive_ms / trie_ms) << "x faster!\n";
    std::cout << "  - Candidates Found:             " << trie_candidates.size() << "\n";
}

void benchmark_lru_cache(SpellChecker& checker) {
    std::cout << "\n=======================================================\n";
    std::cout << "  3. Level 4: LRU Cache Acceleration\n";
    std::cout << "=======================================================\n";

    std::string typo = "aple";
    // First query: Cache Miss (computes Trie-guided DP)
    auto r1 = checker.check_word(typo);

    // Subsequent queries: Cache Hit (instant O(1) from hash table + list)
    auto t0 = std::chrono::high_resolution_clock::now();
    size_t iterations = 100000;
    for (size_t i = 0; i < iterations; ++i) {
        auto r2 = checker.check_word(typo);
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    double total_us = std::chrono::duration<double, std::micro>(t1 - t0).count();
    double ns_per_hit = (total_us * 1000.0) / iterations;

    std::cout << "  - First Lookup (Cache Miss): " << r1.lookup_time_us << " us\n";
    std::cout << "  - Cached Lookups:            " << iterations << " queries\n";
    std::cout << "  - Cached Latency:            " << std::fixed << std::setprecision(1) << ns_per_hit << " ns/query\n";
    std::cout << "  - Cache Hit Rate:            " << checker.cache().hit_rate() << "%\n";
}

int main() {
    SpellChecker checker;
    std::string dict_path = "data/dictionary.txt";

    std::cout << "Loading dictionary from '" << dict_path << "'...\n";
    if (!checker.load_dictionary(dict_path)) {
        std::cout << "Failed to load dictionary. Exiting.\n";
        return 1;
    }
    std::cout << "Loaded " << checker.dictionary_size() << " words.\n";

    benchmark_trie_lookup(checker);
    benchmark_naive_vs_trie_pruning(checker);
    benchmark_lru_cache(checker);

    return 0;
}

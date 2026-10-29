#pragma once

#include "trie.hpp"
#include "trie_searcher.hpp"
#include "lru_cache.hpp"
#include "tokenizer.hpp"
#include "edit_distance.hpp"

#include <vector>
#include <string>
#include <string_view>
#include <queue>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>

namespace spell {

struct WordCheckResult {
    std::string original_token;
    std::string lower_token;
    bool is_correct{false};
    std::vector<CandidateWord> suggestions; // Top-K suggestions
    double lookup_time_us{0.0};
    bool from_cache{false};
};

struct SentenceCheckResult {
    std::string original_sentence;
    std::string corrected_sentence;
    std::vector<WordCheckResult> word_results;
    size_t misspelled_count{0};
    double total_time_ms{0.0};
};

class SpellChecker {
private:
    Trie trie_;
    LRUCache<std::string, std::vector<CandidateWord>> cache_;

public:
    explicit SpellChecker(size_t cache_capacity = 2000) : cache_(cache_capacity) {}

    // Load words from a dictionary file (format: "word [frequency]")
    bool load_dictionary(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::stringstream ss(line);
            std::string word;
            uint32_t freq = 1;

            if (ss >> word) {
                if (ss >> freq) {
                    trie_.insert(word, freq);
                } else {
                    trie_.insert(word, 1);
                }
            }
        }
        return true;
    }

    void insert_word(std::string_view word, uint32_t freq = 1) {
        trie_.insert(word, freq);
        cache_.clear(); // Invalidate cache when dictionary updates
    }

    // Level 1: Quick check if word is valid in dictionary
    bool is_word_correct(std::string_view word) const {
        return trie_.contains(Tokenizer::to_lower(word));
    }

    // Level 2, 3, & 4: Check word, generate Top-K suggestions using Trie DP search & Heap
    WordCheckResult check_word(std::string_view word, size_t top_k = 5, uint32_t max_edit_distance = 2) {
        WordCheckResult result;
        result.original_token = std::string(word);
        result.lower_token = Tokenizer::to_lower(word);

        auto t0 = std::chrono::high_resolution_clock::now();

        // 1. Level 1: Exact lookup in Trie
        if (trie_.contains(result.lower_token)) {
            result.is_correct = true;
            auto t1 = std::chrono::high_resolution_clock::now();
            result.lookup_time_us = std::chrono::duration<double, std::micro>(t1 - t0).count();
            return result;
        }

        result.is_correct = false;

        // 2. Level 4: Check LRU Cache
        auto cached = cache_.get(result.lower_token);
        if (cached.has_value()) {
            result.suggestions = cached.value();
            result.from_cache = true;
            auto t1 = std::chrono::high_resolution_clock::now();
            result.lookup_time_us = std::chrono::duration<double, std::micro>(t1 - t0).count();
            return result;
        }

        // 3. Level 4: Trie-guided approximate search with DP row pruning
        auto candidates = TrieSearcher::find_approximate_matches(trie_, result.lower_token, max_edit_distance);

        // 4. Level 3: Heap-based Top-K Selection
        // CandidateWord operator< sorts by (edit_distance asc, frequency desc)
        // Using a min-heap structure or partial_sort to get top-K
        if (candidates.size() > top_k) {
            std::partial_sort(candidates.begin(), candidates.begin() + top_k, candidates.end());
            candidates.resize(top_k);
        } else {
            std::sort(candidates.begin(), candidates.end());
        }

        result.suggestions = candidates;

        // Store in LRU Cache
        cache_.put(result.lower_token, result.suggestions);

        auto t1 = std::chrono::high_resolution_clock::now();
        result.lookup_time_us = std::chrono::duration<double, std::micro>(t1 - t0).count();
        return result;
    }

    // Sentence-level spell checking: "aple cat" -> "apple cat"
    SentenceCheckResult check_sentence(std::string_view sentence, size_t top_k = 5, uint32_t max_dist = 2) {
        SentenceCheckResult s_result;
        s_result.original_sentence = std::string(sentence);

        auto t0 = std::chrono::high_resolution_clock::now();
        auto tokens = Tokenizer::tokenize(sentence);

        std::string reconstructed;
        for (const auto& tok : tokens) {
            if (!tok.is_word) {
                reconstructed += tok.text;
                continue;
            }

            WordCheckResult wr = check_word(tok.text, top_k, max_dist);
            if (!wr.is_correct) {
                s_result.misspelled_count++;
                if (!wr.suggestions.empty()) {
                    // Pick the highest ranked suggestion (preserved case)
                    std::string best = Tokenizer::preserve_case(tok.text, wr.suggestions.front().word);
                    reconstructed += best;
                } else {
                    reconstructed += tok.text;
                }
            } else {
                reconstructed += tok.text;
            }
            s_result.word_results.push_back(std::move(wr));
        }

        s_result.corrected_sentence = std::move(reconstructed);
        auto t1 = std::chrono::high_resolution_clock::now();
        s_result.total_time_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        return s_result;
    }

    const Trie& trie() const { return trie_; }
    const LRUCache<std::string, std::vector<CandidateWord>>& cache() const { return cache_; }
    size_t dictionary_size() const { return trie_.size(); }
};

} // namespace spell

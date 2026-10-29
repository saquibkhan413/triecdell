#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <array>
#include <cctype>
#include <cstdint>
#include <algorithm>

namespace spell {

class TrieNode {
public:
    std::array<std::unique_ptr<TrieNode>, 26> children{};
    bool is_terminal{false};
    uint32_t frequency{0};
    std::string word;

    TrieNode() = default;

    inline bool has_child(char c) const {
        int idx = std::tolower(static_cast<unsigned char>(c)) - 'a';
        return (idx >= 0 && idx < 26 && children[idx] != nullptr);
    }

    inline TrieNode* get_child(char c) const {
        int idx = std::tolower(static_cast<unsigned char>(c)) - 'a';
        if (idx >= 0 && idx < 26) {
            return children[idx].get();
        }
        return nullptr;
    }

    inline TrieNode* get_or_create_child(char c) {
        int idx = std::tolower(static_cast<unsigned char>(c)) - 'a';
        if (idx < 0 || idx >= 26) return nullptr;
        if (!children[idx]) {
            children[idx] = std::make_unique<TrieNode>();
        }
        return children[idx].get();
    }
};

class Trie {
private:
    std::unique_ptr<TrieNode> root_;
    size_t word_count_{0};

    void collect_words(const TrieNode* node, std::vector<std::pair<std::string, uint32_t>>& results, size_t max_results) const {
        if (!node || results.size() >= max_results) return;

        if (node->is_terminal) {
            results.push_back({node->word, node->frequency});
        }

        for (const auto& child : node->children) {
            if (child) {
                collect_words(child.get(), results, max_results);
                if (results.size() >= max_results) break;
            }
        }
    }

public:
    Trie() : root_(std::make_unique<TrieNode>()) {}

    // Level 1: Insert word into dictionary with frequency
    void insert(std::string_view word, uint32_t frequency = 1) {
        if (word.empty()) return;

        TrieNode* current = root_.get();
        std::string cleaned;
        cleaned.reserve(word.size());

        for (char ch : word) {
            unsigned char uc = static_cast<unsigned char>(ch);
            if (!std::isalpha(uc)) continue;
            char lower_ch = static_cast<char>(std::tolower(uc));
            cleaned.push_back(lower_ch);

            current = current->get_or_create_child(lower_ch);
            if (!current) return;
        }

        if (cleaned.empty()) return;

        if (!current->is_terminal) {
            current->is_terminal = true;
            current->word = std::move(cleaned);
            word_count_++;
        }
        current->frequency += frequency;
    }

    // Level 1: Search word in Trie (Exact lookup)
    const TrieNode* search_node(std::string_view word) const {
        const TrieNode* current = root_.get();
        for (char ch : word) {
            unsigned char uc = static_cast<unsigned char>(ch);
            if (!std::isalpha(uc)) return nullptr;
            current = current->get_child(ch);
            if (!current) return nullptr;
        }
        return (current && current->is_terminal) ? current : nullptr;
    }

    bool contains(std::string_view word) const {
        return search_node(word) != nullptr;
    }

    uint32_t get_frequency(std::string_view word) const {
        const TrieNode* node = search_node(word);
        return node ? node->frequency : 0;
    }

    bool starts_with(std::string_view prefix) const {
        const TrieNode* current = root_.get();
        for (char ch : prefix) {
            unsigned char uc = static_cast<unsigned char>(ch);
            if (!std::isalpha(uc)) return false;
            current = current->get_child(ch);
            if (!current) return false;
        }
        return current != nullptr;
    }

    std::vector<std::pair<std::string, uint32_t>> autocomplete(std::string_view prefix, size_t max_results = 10) const {
        std::vector<std::pair<std::string, uint32_t>> results;
        const TrieNode* current = root_.get();

        for (char ch : prefix) {
            unsigned char uc = static_cast<unsigned char>(ch);
            if (!std::isalpha(uc)) return results;
            current = current->get_child(ch);
            if (!current) return results;
        }

        collect_words(current, results, max_results);
        return results;
    }

    const TrieNode* root() const { return root_.get(); }
    size_t size() const { return word_count_; }

    void clear() {
        root_ = std::make_unique<TrieNode>();
        word_count_ = 0;
    }
};

} // namespace spell

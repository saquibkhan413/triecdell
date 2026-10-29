#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <cctype>

namespace spell {

struct Token {
    std::string text;
    bool is_word{false};
    size_t start_offset{0};
};

class Tokenizer {
public:
    static std::vector<Token> tokenize(std::string_view text) {
        std::vector<Token> tokens;
        size_t n = text.size();
        size_t i = 0;

        while (i < n) {
            size_t start = i;
            if (std::isalpha(static_cast<unsigned char>(text[i]))) {
                while (i < n && std::isalpha(static_cast<unsigned char>(text[i]))) {
                    ++i;
                }
                tokens.push_back({std::string(text.substr(start, i - start)), true, start});
            } else {
                while (i < n && !std::isalpha(static_cast<unsigned char>(text[i]))) {
                    ++i;
                }
                tokens.push_back({std::string(text.substr(start, i - start)), false, start});
            }
        }

        return tokens;
    }

    static std::string to_lower(std::string_view s) {
        std::string res;
        res.reserve(s.size());
        for (char c : s) {
            res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
        return res;
    }

    static std::string preserve_case(std::string_view original, std::string_view replacement) {
        if (original.empty()) return std::string(replacement);
        std::string res = std::string(replacement);

        bool all_caps = true;
        for (char c : original) {
            if (std::islower(static_cast<unsigned char>(c))) {
                all_caps = false;
                break;
            }
        }

        if (all_caps && original.size() > 1) {
            for (char& c : res) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            return res;
        }

        if (std::isupper(static_cast<unsigned char>(original.front()))) {
            res.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(res.front())));
        }
        return res;
    }
};

} // namespace spell

#include "spell_checker.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>

using namespace spell;

namespace colors {
    const char* RESET   = "\033[0m";
    const char* BOLD    = "\033[1m";
    const char* RED     = "\033[31m";
    const char* GREEN   = "\033[32m";
    const char* YELLOW  = "\033[33m";
    const char* BLUE    = "\033[34m";
    const char* MAGENTA = "\033[35m";
    const char* CYAN    = "\033[36m";
    const char* WHITE   = "\033[37m";
}

void print_banner() {
    std::cout << colors::CYAN << colors::BOLD;
    std::cout << R"(
  ==============================================================
   _____ ____  ___ _____   ____ ___  _____ _     _     
  |_   _|  _ \|_ _| ____| / ___|  _ \| ____| |   | |    
    | | | |_) || ||  _|  | |   | | | |  _| | |   | |    
    | | |  _ < | || |___ | |___| |_| | |___| |___| |___ 
    |_| |_| \_\___|_____| \____|____/|_____|_____|_____|
       TRIE-BASED SPELL CHECKER & AUTOCORRECT (C++20)
  ==============================================================
)" << colors::RESET;
    std::cout << colors::WHITE << "  Level 1: Trie Dictionary & Search\n"
              << "  Level 2: Spelling Mistake Detection & Edit Distance\n"
              << "  Level 3: Frequency Ranking & Top-K Heap Selection\n"
              << "  Level 4: Trie-Guided DP Approximate Pruning & LRU Cache\n\n"
              << colors::RESET;
}

void print_help() {
    std::cout << colors::YELLOW << colors::BOLD << "Commands:\n" << colors::RESET;
    std::cout << "  " << colors::GREEN << "<sentence>" << colors::RESET << "           : Type any word or sentence (e.g. 'aple cat')\n";
    std::cout << "  " << colors::GREEN << "word <token>" << colors::RESET << "         : Detailed approximate search for a single word\n";
    std::cout << "  " << colors::GREEN << "suggest <prefix>" << colors::RESET << "     : Autocomplete prefix search using Trie\n";
    std::cout << "  " << colors::GREEN << "add <word> [freq]" << colors::RESET << "   : Insert a new word into the Trie dictionary\n";
    std::cout << "  " << colors::GREEN << "stats" << colors::RESET << "                : View Trie size and LRU cache statistics\n";
    std::cout << "  " << colors::GREEN << "demo" << colors::RESET << "                 : Run automated demo matching the project diagrams\n";
    std::cout << "  " << colors::GREEN << "help" << colors::RESET << "                 : Show this help menu\n";
    std::cout << "  " << colors::GREEN << "exit / quit" << colors::RESET << "         : Quit application\n\n";
}

void run_sentence_check(SpellChecker& checker, std::string_view sentence) {
    auto res = checker.check_sentence(sentence, 5, 2);

    std::cout << "\n" << colors::BOLD << "Input Sentence: " << colors::CYAN << "\"" << sentence << "\"" << colors::RESET << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (const auto& w : res.word_results) {
        if (w.is_correct) {
            std::cout << "  " << colors::GREEN << "[FOUND]" << colors::RESET 
                      << "  \"" << w.original_token << "\" is spelled correctly.\n";
        } else {
            std::cout << "  " << colors::RED << "[NOT FOUND]" << colors::RESET 
                      << " \"" << w.original_token << "\" -> Suggestions: ";
            if (w.suggestions.empty()) {
                std::cout << colors::YELLOW << "(no suggestions within distance 2)" << colors::RESET;
            } else {
                for (size_t i = 0; i < w.suggestions.size(); ++i) {
                    if (i > 0) std::cout << " | ";
                    std::cout << colors::BOLD << colors::GREEN << w.suggestions[i].word << colors::RESET 
                              << " (d=" << w.suggestions[i].edit_distance 
                              << ", freq=" << w.suggestions[i].frequency << ")";
                }
            }
            std::cout << " [" << std::fixed << std::setprecision(1) << w.lookup_time_us << " us"
                      << (w.from_cache ? ", CACHED" : "") << "]\n";
        }
    }

    std::cout << std::string(60, '-') << "\n";
    if (res.misspelled_count > 0) {
        std::cout << colors::BOLD << "Autocorrected: " << colors::GREEN 
                  << "\"" << res.corrected_sentence << "\"" << colors::RESET << "\n";
    } else {
        std::cout << colors::GREEN << "All words are spelled correctly!\n" << colors::RESET;
    }
    std::cout << "Total processing time: " << std::fixed << std::setprecision(2) << res.total_time_ms << " ms\n\n";
}

void run_word_check(SpellChecker& checker, std::string_view word) {
    auto res = checker.check_word(word, 5, 2);

    std::cout << "\nChecking word: \"" << word << "\"\n";
    std::cout << "Status: " << (res.is_correct ? (colors::GREEN + std::string("CORRECT")) : (colors::RED + std::string("MISSPELLED"))) 
              << colors::RESET << "\n";
    std::cout << "Time:   " << std::fixed << std::setprecision(1) << res.lookup_time_us << " microseconds"
              << (res.from_cache ? " [LRU Cache Hit]" : " [Trie-guided DP search]") << "\n";

    if (!res.is_correct) {
        std::cout << "\nTop-" << res.suggestions.size() << " Ranked Suggestions (Edit Distance asc, Frequency desc):\n";
        std::cout << std::left << std::setw(6) << "Rank" 
                  << std::setw(16) << "Suggestion" 
                  << std::setw(16) << "Edit Distance" 
                  << std::setw(16) << "Frequency\n";
        std::cout << std::string(54, '-') << "\n";

        for (size_t i = 0; i < res.suggestions.size(); ++i) {
            std::cout << std::left << std::setw(6) << (i + 1)
                      << std::setw(16) << res.suggestions[i].word
                      << std::setw(16) << res.suggestions[i].edit_distance
                      << std::setw(16) << res.suggestions[i].frequency << "\n";
        }
    }
    std::cout << "\n";
}

void run_demo(SpellChecker& checker) {
    std::cout << colors::MAGENTA << colors::BOLD << "\n=======================================================\n";
    std::cout << "  RUNNING AUTOMATED SPELL CHECKER DEMO\n";
    std::cout << "=======================================================\n" << colors::RESET;

    std::cout << colors::BOLD << "\n[DEMO 1: Single Word Approximate Search (matching Image 1)]\n" << colors::RESET;
    std::cout << "User enters: \"aple\"\n";
    std::cout << "  ↓\nSpell Checker -> Trie -> Edit Distance\n";
    run_word_check(checker, "aple");

    std::cout << colors::BOLD << "\n[DEMO 2: Sentence Tokenization & Autocorrect (matching Image 2)]\n" << colors::RESET;
    std::cout << "User enters: \"aple cat\"\n";
    std::cout << "  ↓\nTokenizer -> \"aple\" (Not found) | \"cat\" (Found)\n";
    run_sentence_check(checker, "aple cat");

    std::cout << colors::BOLD << "\n[DEMO 3: Sentence with Multiple Errors]\n" << colors::RESET;
    run_sentence_check(checker, "Helo wrld, this is a computr algoritm test.");

    std::cout << colors::BOLD << "\n[DEMO 4: LRU Cache Verification]\n" << colors::RESET;
    std::cout << "Querying \"aple\" again to verify Level 4 LRU Caching:\n";
    run_word_check(checker, "aple");
}

int main(int argc, char* argv[]) {
    print_banner();

    SpellChecker checker;
    std::string dict_path = "data/dictionary.txt";
    if (argc > 1) {
        dict_path = argv[1];
    }

    std::cout << colors::GREEN << "Loading dictionary from '" << dict_path << "'...\n" << colors::RESET;
    auto t0 = std::chrono::high_resolution_clock::now();
    bool loaded = checker.load_dictionary(dict_path);
    auto t1 = std::chrono::high_resolution_clock::now();

    if (!loaded) {
        std::cout << colors::RED << "Warning: Could not open dictionary file at '" << dict_path << "'.\n"
                  << "Using built-in sample vocabulary...\n" << colors::RESET;
        checker.insert_word("apple", 85000000);
        checker.insert_word("ample", 12000000);
        checker.insert_word("ape", 4500000);
        checker.insert_word("cat", 75000000);
        checker.insert_word("hello", 48000000);
        checker.insert_word("world", 82000000);
    } else {
        double load_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        std::cout << colors::GREEN << "Loaded " << checker.dictionary_size() 
                  << " words into Trie in " << std::fixed << std::setprecision(2) << load_ms << " ms!\n\n" << colors::RESET;
    }

    print_help();

    std::string line;
    while (true) {
        std::cout << colors::CYAN << colors::BOLD << "spell-check> " << colors::RESET;
        if (!std::getline(std::cin, line)) break;

        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front()))) line.erase(line.begin());
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back()))) line.pop_back();

        if (line.empty()) continue;
        if (line == "exit" || line == "quit") break;

        if (line == "help") {
            print_help();
            continue;
        }

        if (line == "demo") {
            run_demo(checker);
            continue;
        }

        if (line == "stats") {
            std::cout << colors::YELLOW << "Engine Statistics:\n" << colors::RESET;
            std::cout << "  - Words in Trie Dictionary: " << checker.dictionary_size() << "\n";
            std::cout << "  - LRU Cache Entries:        " << checker.cache().size() << " / " << checker.cache().capacity() << "\n";
            std::cout << "  - LRU Cache Hits:           " << checker.cache().hits() << "\n";
            std::cout << "  - LRU Cache Misses:         " << checker.cache().misses() << "\n";
            std::cout << "  - LRU Cache Hit Rate:       " << std::fixed << std::setprecision(1) << checker.cache().hit_rate() << "%\n\n";
            continue;
        }

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "word") {
            std::string w;
            ss >> w;
            if (w.empty()) {
                std::cout << colors::RED << "Usage: word <misspelled_word>\n" << colors::RESET;
                continue;
            }
            run_word_check(checker, w);
        } else if (cmd == "suggest") {
            std::string prefix;
            ss >> prefix;
            if (prefix.empty()) {
                std::cout << colors::RED << "Usage: suggest <prefix>\n" << colors::RESET;
                continue;
            }
            auto suggestions = checker.trie().autocomplete(prefix, 10);
            std::cout << colors::CYAN << "Trie suggestions for prefix '" << prefix << "':\n" << colors::RESET;
            if (suggestions.empty()) {
                std::cout << "  (no words found with this prefix)\n\n";
            } else {
                for (const auto& [w, freq] : suggestions) {
                    std::cout << "  - " << colors::BOLD << w << colors::RESET << " (freq=" << freq << ")\n";
                }
                std::cout << "\n";
            }
        } else if (cmd == "add") {
            std::string w;
            uint32_t freq = 1;
            ss >> w;
            if (ss >> freq) {}
            if (w.empty()) {
                std::cout << colors::RED << "Usage: add <word> [frequency]\n" << colors::RESET;
                continue;
            }
            checker.insert_word(w, freq);
            std::cout << colors::GREEN << "Added '" << w << "' (freq=" << freq << ") to dictionary. Total words: " 
                      << checker.dictionary_size() << "\n\n" << colors::RESET;
        } else {
            // Treat the input line as a sentence to spell-check
            run_sentence_check(checker, line);
        }
    }

    std::cout << "Exiting Spell Checker. Goodbye!\n";
    return 0;
}

CXX = g++
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -Iinclude -pthread
BIN_DIR = bin

all: $(BIN_DIR)/spell_check $(BIN_DIR)/benchmark

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/spell_check: src/main.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR)/benchmark: src/benchmark.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean

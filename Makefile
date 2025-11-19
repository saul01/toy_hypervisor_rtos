CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread -Isrc -Isrc/hypervisor -Isrc/rtos

# Find all .cpp files recursively
SRC = $(shell find src -name '*.cpp')

# Convert "src/abc/xyz.cpp" → "build/abc/xyz.o"
OBJ = $(patsubst src/%.cpp, build/%.o, $(SRC))

BIN = bin/toy_hv_rt

all: $(BIN)

$(BIN): $(OBJ) | bin
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

# Rule for building object files
# Ensures that nested directories under "build/" exist
build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

bin:
	mkdir -p bin

clean:
	rm -rf build bin

.PHONY: all clean

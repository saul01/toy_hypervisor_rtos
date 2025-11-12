CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst src/%.cpp, build/%.o, $(SRC))
BIN = bin/toy_hv_rt

all: $(BIN)

$(BIN): $(OBJ) | bin
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

bin:
	mkdir -p bin

build:
	mkdir -p build

clean:
	rm -rf build bin

.PHONY: all clean

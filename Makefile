CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude

SRCS = src/main.cpp src/reference.cpp src/generate.cpp

.PHONY: all test clean

all: build/compact_ref

build/compact_ref: $(SRCS) include/compact.hpp include/generate.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

test: build/compact_ref
	./build/compact_ref

clean:
	rm -rf build

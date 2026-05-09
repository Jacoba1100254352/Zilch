.PHONY: all build run test

all: build run

build:
	cmake -S . -B build
	cmake --build build

run:
	./build/Zilch

test: build
	ctest --test-dir build --output-on-failure

.PHONY: all build run

all: build run

build:
	cmake -S . -B build
	cmake --build build

run:
	./build/Zilch

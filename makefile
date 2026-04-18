.PHONY: all

all:
	c++ -std=c++23 -Wall -Wextra -Wpedantic *.cpp -o zilch
	./zilch

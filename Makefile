run: main
	./main;

main: main.cc
	c++ -g -std=c++11 $< -o $@;

all: vgslz4

vgslz4: vgslz4.cpp ../../src/lz4.c ../../src/lz4.h
	gcc -I../../src ../../src/lz4.c -c -o lz4.o
	g++ -std=c++17 -I../../src vgslz4.cpp lz4.o -o vgslz4

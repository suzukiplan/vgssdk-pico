all: vgsmml

vgsmml: src/vgsmml.c src/vgsftv.cpp src/cli.c src/vgsmml.h src/vgsdec.h src/vgsdec_internal.h ../../src/lz4.c
	g++ -std=c++17 -c src/vgsftv.cpp
	gcc -c src/vgsmml.c
	gcc -I../../src -c src/cli.c
	gcc -I../../src -c ../../src/lz4.c -c -o lz4.o
	g++ -o vgsmml vgsftv.o vgsmml.o cli.o lz4.o

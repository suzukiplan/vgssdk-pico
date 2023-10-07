all:
	make copy
	pio run -t upload

./src:
	mkdir ./src

build: ./src
	make copy
	pio run

copy:
	rm -rf ./src
	mkdir ./src
	cp -p ../platformio.ini ./
	cp -p ../../src/lz4.c ./src
	cp -p ../../src/lz4.h ./src
	cp -p ../../src/vgsdecv.hpp ./src
	cp -p ../../src/vgssdk_pico.cpp ./src
	cp -p ../../src/vgssdk.h ./src
	cp -p ../../src/vgstone.c ./src
	cp -p ../../src/FT6336U.hpp ./src
	cp -p ../../example/touch/touch.cpp ./src

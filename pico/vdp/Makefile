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
	cd ../.. && make roms
	cp -p ../../src/lz4.c ./src
	cp -p ../../src/lz4.h ./src
	cp -p ../../src/vgsdecv.hpp ./src
	cp -p ../../src/vgssdk_pico.cpp ./src
	cp -p ../../src/vgssdk.h ./src
	cp -p ../../src/vgstone.c ./src
	cp -p ../../src/FT6336U.hpp ./src
	cp -p ../../example/assets/small_font.c ./src
	cp -p ../../example/assets/vram_ptn.c ./src
	cp -p ../../example/assets/bgm.c ./src
	cp -p ../../example/assets/eff1.c ./src
	cp -p ../../example/assets/roms.hpp ./src
	cp -p ../../example/vdp/vdp_test.cpp ./src

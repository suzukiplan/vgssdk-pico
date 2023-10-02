all: ./src ./include
	make copy
	pio run -t upload

build: ./src ./include
	make copy
	pio run

copy:
	rm -f ./src/*
	rm -f ./include/*
	cp -p ../src/lz4.c ./src
	cp -p ../src/lz4.h ./include
	cp -p ../src/vgsdecv.hpp ./include
	cp -p ../src/vgssdk_pico.cpp ./src
	cp -p ../src/vgssdk.h ./include
	cp -p ../src/vgstone.c ./src
	cp -p ../src/FT6336U.hpp ./include
	cp -p ../example/app.cpp ./src
	cp -p ../example/bgm_test_data.c ./src
	cp -p ../example/image_test_data.c ./src

./src:
	mkdir ./src

./include:
	mkdir ./include

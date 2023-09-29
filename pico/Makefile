all: ./src ./include
	cp -p ../src/lz4.c ./src
	cp -p ../src/lz4.h ./include
	cp -p ../src/vgsdecv.hpp ./include
	cp -p ../src/vgssdk_pico.cpp ./src
	cp -p ../src/vgssdk.h ./include
	cp -p ../src/vgstone.c ./src
	cp -p ../example/app.cpp ./src
	cp -p ../example/bgm_test_data.c ./src
	cp -p ../example/image_test_data.c ./src
	pio run -t upload

./src:
	mkdir ./src

./include:
	mkdir ./include

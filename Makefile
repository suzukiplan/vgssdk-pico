SDLOPTS = -I/usr/include/SDL2 -I/usr/local/include/SDL2 -I/opt/X11/include -D_THREAD_SAFE
CFLAGS = -O2 ${SDLOPTS}
CPPFLAGS = -std=c++11 -O2 ${SDLOPTS}
OBJECTS = app.o bgm_test_data.o image_test_data.o vgssdk_sdl2.o vgstone.o lz4.o

all: $(OBJECTS)
	g++ -o app $(OBJECTS) -L/usr/local/lib -lSDL2

format: 
	make execute-format FILENAME=./example/app.cpp
	make execute-format FILENAME=./src/vgssdk_sdl2.cpp
	make execute-format FILENAME=./src/vgssdk_pico.cpp
	make execute-format FILENAME=./src/vgssdk.h
	make execute-format FILENAME=./src/vgsdecv.hpp
        
execute-format:
	clang-format -style=file < ${FILENAME} > ${FILENAME}.bak
	cat ${FILENAME}.bak > ${FILENAME}
	rm ${FILENAME}.bak

app.o: example/app.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/app.cpp

bgm_test_data.o: example/bgm_test_data.c
	gcc $(CLAGS) -c example/bgm_test_data.c

image_test_data.o: example/image_test_data.c
	gcc $(CLAGS) -c example/image_test_data.c

vgssdk_sdl2.o: src/vgssdk_sdl2.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -c src/vgssdk_sdl2.cpp

vgstone.o: src/vgstone.c
	gcc $(CLAGS) -c src/vgstone.c

lz4.o: src/lz4.c
	gcc $(CLAGS) -c src/lz4.c

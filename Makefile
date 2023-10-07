SDLOPTS = -I/usr/include/SDL2 -I/usr/local/include/SDL2 -I/opt/X11/include -D_THREAD_SAFE
CFLAGS = -O2 ${SDLOPTS}
CPPFLAGS = -std=c++11 -O2 ${SDLOPTS}
OBJECTS_SDK = vgssdk_sdl2.o vgstone.o lz4.o
OBJECTS_RGB = rgb.o ${OBJECTS_SDK}
OBJECTS_TOUCH = touch.o ${OBJECTS_SDK}
OBJECTS_IMAGE = image.o image_test_data.o ${OBJECTS_SDK}
OBJECTS_SOUND = sound.o bgm_test_data.o ${OBJECTS_SDK}
OBJECTS_SANDSTORM = sandstorm.o ${OBJECTS_SDK}

all:
	make bin
	make bin/rgb
	make bin/touch
	make bin/image
	make bin/sound
	make bin/sandstorm

bin/rgb: $(OBJECTS_RGB)
	g++ -o bin/rgb $(OBJECTS_RGB) -L/usr/local/lib -lSDL2

bin/touch: $(OBJECTS_TOUCH)
	g++ -o bin/touch $(OBJECTS_TOUCH) -L/usr/local/lib -lSDL2

bin/image: ${OBJECTS_IMAGE}
	g++ -o bin/image $(OBJECTS_IMAGE) -L/usr/local/lib -lSDL2

bin/sound: ${OBJECTS_SOUND}
	g++ -o bin/sound $(OBJECTS_SOUND) -L/usr/local/lib -lSDL2

bin/sandstorm: ${OBJECTS_SANDSTORM}
	g++ -o bin/sandstorm $(OBJECTS_SANDSTORM) -L/usr/local/lib -lSDL2

bin:
	mkdir bin

format: 
	make execute-format FILENAME=./example/rgb/rgb.cpp
	make execute-format FILENAME=./example/touch/touch.cpp
	make execute-format FILENAME=./example/image/image.cpp
	make execute-format FILENAME=./example/sound/sound.cpp
	make execute-format FILENAME=./example/sandstorm/sandstorm.cpp
	make execute-format FILENAME=./src/vgssdk_sdl2.cpp
	make execute-format FILENAME=./src/vgssdk_pico.cpp
	make execute-format FILENAME=./src/vgssdk.h
	make execute-format FILENAME=./src/vgsdecv.hpp
        
execute-format:
	clang-format -style=file < ${FILENAME} > ${FILENAME}.bak
	cat ${FILENAME}.bak > ${FILENAME}
	rm ${FILENAME}.bak

rgb.o: example/rgb/rgb.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/rgb/rgb.cpp

touch.o: example/touch/touch.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/touch/touch.cpp

sandstorm.o: example/sandstorm/sandstorm.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/sandstorm/sandstorm.cpp

image.o: example/image/image.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/image/image.cpp

image_test_data.o: example/image/image_test_data.c
	gcc $(CFLAGS) -I./src -c example/image/image_test_data.c

sound.o: example/sound/sound.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/sound/sound.cpp

bgm_test_data.o: example/sound/bgm_test_data.c
	gcc $(CLAGS) -c example/sound/bgm_test_data.c

vgssdk_sdl2.o: src/vgssdk_sdl2.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -c src/vgssdk_sdl2.cpp

vgstone.o: src/vgstone.c
	gcc $(CLAGS) -c src/vgstone.c

lz4.o: src/lz4.c
	gcc $(CLAGS) -c src/lz4.c

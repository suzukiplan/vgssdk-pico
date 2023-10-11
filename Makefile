SDLOPTS = -I/usr/include/SDL2 -I/usr/local/include/SDL2 -I/opt/X11/include -D_THREAD_SAFE
CFLAGS = -O2 ${SDLOPTS}
CPPFLAGS = -std=c++11 -O2 ${SDLOPTS}
OBJECTS_SDK = vgssdk_sdl2.o vgstone.o lz4.o
OBJECTS_RGB = rgb.o ${OBJECTS_SDK}
OBJECTS_TOUCH = touch.o ${OBJECTS_SDK}
OBJECTS_IMAGE = image.o image_test_data.o ${OBJECTS_SDK}
OBJECTS_SOUND = sound.o bgm.o small_font.o eff1.o eff2.o eff3.o ${OBJECTS_SDK}
OBJECTS_SANDSTORM = sandstorm.o ${OBJECTS_SDK}
OBJECTS_VDP = vdp_test.o small_font.o vram_ptn.o ${OBJECTS_SDK}

all:
	make roms
	make bin
	make bin/rgb
	make bin/touch
	make bin/image
	make bin/sound
	make bin/sandstorm
	make bin/vdp

roms:
	cd tools && make
	./tools/bmp2img/bmp2img -t 4x8 ./example/sound/small_font.bmp > ./example/sound/small_font.c
	./tools/bin2var/bin2var -s ./example/sound/eff1.wav > ./example/sound/eff1.c
	./tools/bin2var/bin2var -s ./example/sound/eff2.wav > ./example/sound/eff2.c
	./tools/bin2var/bin2var -s ./example/sound/eff3.wav > ./example/sound/eff3.c
	./tools/vgsmml/vgsmml ./example/sound/bgm.mml ./example/sound/bgm.bgm
	./tools/vgsftv/vgsftv ./example/sound/bgm.bgm ./example/sound/bgm.ftv
	./tools/vgslz4/vgslz4 ./example/sound/bgm.ftv ./example/sound/bgm.lz4
	./tools/bin2var/bin2var -b ./example/sound/bgm.lz4 > ./example/sound/bgm.c
	./tools/varext/varext ./example/sound/small_font.c ./example/sound/eff1.c ./example/sound/eff2.c ./example/sound/eff3.c ./example/sound/bgm.c > ./example/sound/roms.hpp
	./tools/bmp2img/bmp2img -t 8x8 ./example/vdp/vram_ptn.bmp > ./example/vdp/vram_ptn.c
	./tools/varext/varext ./example/sound/small_font.c ./example/vdp/vram_ptn.c > ./example/vdp/roms.hpp

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

bin/vdp: $(OBJECTS_VDP)
	g++ -o bin/vdp $(OBJECTS_VDP) -L/usr/local/lib -lSDL2

bin:
	mkdir bin

format: 
	make execute-format FILENAME=./example/rgb/rgb.cpp
	make execute-format FILENAME=./example/touch/touch.cpp
	make execute-format FILENAME=./example/image/image.cpp
	make execute-format FILENAME=./example/sound/sound.cpp
	make execute-format FILENAME=./example/sandstorm/sandstorm.cpp
	make execute-format FILENAME=./example/vdp/vdp_test.cpp
	make execute-format FILENAME=./src/FT6336U.hpp
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

vdp_test.o: example/vdp/vdp_test.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/vdp/vdp_test.cpp

vram_ptn.o: example/vdp/vram_ptn.c
	gcc $(CLAGS) -c example/vdp/vram_ptn.c

sandstorm.o: example/sandstorm/sandstorm.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/sandstorm/sandstorm.cpp

image.o: example/image/image.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/image/image.cpp

image_test_data.o: example/image/image_test_data.c
	gcc $(CFLAGS) -I./src -c example/image/image_test_data.c

sound.o: example/sound/sound.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/sound/sound.cpp

bgm.o: example/sound/bgm.c
	gcc $(CLAGS) -c example/sound/bgm.c

small_font.o: example/sound/small_font.c
	gcc $(CLAGS) -c example/sound/small_font.c

eff1.o: example/sound/eff1.c
	gcc $(CLAGS) -c example/sound/eff1.c

eff2.o: example/sound/eff2.c
	gcc $(CLAGS) -c example/sound/eff2.c

eff3.o: example/sound/eff3.c
	gcc $(CLAGS) -c example/sound/eff3.c

vgssdk_sdl2.o: src/vgssdk_sdl2.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -c src/vgssdk_sdl2.cpp

vgstone.o: src/vgstone.c
	gcc $(CLAGS) -c src/vgstone.c

lz4.o: src/lz4.c
	gcc $(CLAGS) -c src/lz4.c

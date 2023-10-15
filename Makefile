SDLOPTS = -I/usr/include/SDL2 -I/usr/local/include/SDL2 -I/opt/X11/include -D_THREAD_SAFE
CFLAGS = -O2 -I./example/assets ${SDLOPTS}
CPPFLAGS = -std=c++11 -O2 -I./example/assets ${SDLOPTS}
OBJECTS_SDK = vgssdk_sdl2.o vgstone.o lz4.o
OBJECTS_RGB = rgb.o ${OBJECTS_SDK}
OBJECTS_TOUCH = touch.o ${OBJECTS_SDK}
OBJECTS_IMAGE = image.o ${OBJECTS_SDK}
OBJECTS_SOUND = sound.o bgm.o small_font.o eff1.o eff2.o eff3.o ${OBJECTS_SDK}
OBJECTS_SANDSTORM = sandstorm.o ${OBJECTS_SDK}
OBJECTS_VDP = vdp_test.o small_font.o vram_ptn.o bgm.o eff1.o ${OBJECTS_SDK}
ASSET_SOURCE = example/assets/small_font.c\
	example/assets/eff1.c\
	example/assets/eff2.c\
	example/assets/eff3.c\
	example/assets/bgm.c\
	example/assets/vram_ptn.c

all:
	make roms
	make bin
	make bin/rgb
	make bin/touch
	make bin/image
	make bin/sound
	make bin/sandstorm
	make bin/vdp

clean:
	rm -f $(ASSET_SOURCE)
	rm -f *.o
	rm -rf bin

roms:
	cd tools && make
	make example/assets/roms.hpp

example/assets/roms.hpp: $(ASSET_SOURCE)
	./tools/varext/varext $(ASSET_SOURCE) > ./example/assets/roms.hpp

example/assets/small_font.c: example/assets/small_font.bmp
	./tools/bmp2img/bmp2img -t 4x8 ./example/assets/small_font.bmp > ./example/assets/small_font.c

example/assets/eff1.c: example/assets/eff1.wav
	./tools/bin2var/bin2var -s ./example/assets/eff1.wav > ./example/assets/eff1.c

example/assets/eff2.c: example/assets/eff2.wav
	./tools/bin2var/bin2var -s ./example/assets/eff2.wav > ./example/assets/eff2.c

example/assets/eff3.c: example/assets/eff3.wav
	./tools/bin2var/bin2var -s ./example/assets/eff3.wav > ./example/assets/eff3.c

example/assets/bgm.c: example/assets/bgm.mml
	./tools/vgsmml/vgsmml ./example/assets/bgm.mml ./example/assets/bgm.bin
	./tools/bin2var/bin2var -b ./example/assets/bgm.bin > ./example/assets/bgm.c

example/assets/vram_ptn.c: example/assets/vram_ptn.bmp
	./tools/bmp2img/bmp2img -t 8x8 ./example/assets/vram_ptn.bmp > ./example/assets/vram_ptn.c

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

sandstorm.o: example/sandstorm/sandstorm.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/sandstorm/sandstorm.cpp

image.o: example/image/image.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/image/image.cpp

sound.o: example/sound/sound.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -I./src -c example/sound/sound.cpp

vgssdk_sdl2.o: src/vgssdk_sdl2.cpp src/vgssdk.h
	g++ $(CPPFLAGS) -c src/vgssdk_sdl2.cpp

vgstone.o: src/vgstone.c
	gcc $(CLAGS) -c src/vgstone.c

lz4.o: src/lz4.c
	gcc $(CLAGS) -c src/lz4.c

vram_ptn.o: example/assets/vram_ptn.c
	gcc $(CLAGS) -c example/assets/vram_ptn.c

bgm.o: example/assets/bgm.c
	gcc $(CLAGS) -c example/assets/bgm.c

small_font.o: example/assets/small_font.c
	gcc $(CLAGS) -c example/assets/small_font.c

eff1.o: example/assets/eff1.c
	gcc $(CLAGS) -c example/assets/eff1.c

eff2.o: example/assets/eff2.c
	gcc $(CLAGS) -c example/assets/eff2.c

eff3.o: example/assets/eff3.c
	gcc $(CLAGS) -c example/assets/eff3.c

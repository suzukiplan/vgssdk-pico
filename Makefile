all:
	make -f Makefile.`uname`

format: 
	make execute-format FILENAME=./src/app.cpp
	make execute-format FILENAME=./src/vgssdk_sdl2.cpp
	make execute-format FILENAME=./src/vgssdk.h
        
execute-format:
	clang-format -style=file < ${FILENAME} > ${FILENAME}.bak
	cat ${FILENAME}.bak > ${FILENAME}
	rm ${FILENAME}.bak

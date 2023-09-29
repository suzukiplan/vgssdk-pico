all:
	make -f Makefile.`uname`

format: 
	make execute-format FILENAME=./example/app.cpp
	make execute-format FILENAME=./src/vgssdk_sdl2.cpp
	make execute-format FILENAME=./src/vgssdk.h
	make execute-format FILENAME=./src/vgsdecv.hpp
        
execute-format:
	clang-format -style=file < ${FILENAME} > ${FILENAME}.bak
	cat ${FILENAME}.bak > ${FILENAME}
	rm ${FILENAME}.bak

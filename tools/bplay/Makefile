all:
	make -f Makefile.`uname`

test:
	make all
	cd ../vgsmml && make
	cd ../vgsftv && make
	cd ../vgslz4 && make
	../vgsmml/vgsmml ../../mml/HOURAI-01.mml test.bg1
	../vgsftv/vgsftv test.bg1 test.bg2
	../vgslz4/vgslz4 test.bg2 test.bgm
	./bplay test.bgm

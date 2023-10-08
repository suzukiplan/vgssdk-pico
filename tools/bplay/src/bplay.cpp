#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sound.h"
#include "vgsdecv.hpp"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        puts("usage: bplay /path/to/file.bgm");
        return 1;
    }
    FILE* fp = fopen(argv[1], "rb");
    if (!fp) {
        puts("file open error");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    void* buf = malloc(size);
    if (size != fread(buf, 1, size, fp)) {
        puts("read failed");
        fclose(fp);
        return -1;
    } else {
        printf("loaded %lu bytes\n", size);
    }
    fclose(fp);

    VGSDecoder vgs;
    vgs.load(buf, size);

    auto snd = sound_create();

    unsigned char sbuf[8192];

    while (vgs.execute(sbuf, sizeof(sbuf))) {
        if (0 < vgs.getLoopCount()) {
            vgs.fadeout();
        }
        sound_enqueue(snd, sbuf, sizeof(sbuf));
        while (sizeof(sbuf) * 4 < sound_buffer_left(snd)) {
            usleep(1000);
        }
    }

    sound_destroy(snd);
    return 0;
}
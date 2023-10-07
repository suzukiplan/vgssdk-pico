#include "vgssdk.h"
#include <stdlib.h>

extern VGS vgs;

extern "C" void vgs_setup()
{
}

extern "C" void vgs_loop()
{
    vgs.gfx.startWrite();
    for (int n = 0; n < 8; n++) {
        for (int i = 0; i < 32; i++) {
            unsigned short c = i;
            c <<= 5;
            c |= i;
            c <<= 6;
            c |= i;
            c |= 0x0020;
            vgs.gfx.pixel(rand() % 240, rand() % 320, c);
        }
    }
    vgs.gfx.endWrite();
}

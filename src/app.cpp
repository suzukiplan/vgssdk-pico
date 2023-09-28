#include "vgssdk.h"
#include <stdlib.h>
#include <time.h>

extern VGS vgs;

extern "C" void vgs_setup()
{
    srand(time(nullptr));
    vgs.gfx.clear(0x001F);
    vgs.gfx.box(8, 8, 240 - 16, 32, 0x07C0);
    vgs.gfx.boxf(8, 48, 240 - 16, 32, 0xF800);
    return;
}

extern "C" void vgs_loop()
{
    vgs.gfx.pixel(rand() % vgs.getDisplayWidth(), rand() % vgs.getDisplayHeight(), rand() & 0xFFFF);
    vgs.gfx.line(rand() % vgs.getDisplayWidth(), rand() % vgs.getDisplayHeight(), rand() % vgs.getDisplayWidth(), rand() % vgs.getDisplayHeight(), rand() & 0xFFFF);
    return;
}
#include "vgssdk.h"
#include <stdlib.h>
#include <time.h>

extern VGS vgs;

extern "C" void vgs_setup()
{
    srand(time(nullptr));
    return;
}

extern "C" void vgs_loop()
{
    vgs.gfx.pixel(rand() % vgs.getDisplayWidth(), rand() % vgs.getDisplayHeight(), rand() & 0xFFFF);
    vgs.gfx.line(rand() % vgs.getDisplayWidth(), rand() % vgs.getDisplayHeight(), rand() % vgs.getDisplayWidth(), rand() % vgs.getDisplayHeight(), rand() & 0xFFFF);
    return;
}
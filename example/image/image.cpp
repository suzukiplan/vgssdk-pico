#include "vgssdk.h"
#include <stdlib.h>

extern VGS vgs;

static const unsigned short imageTestData[256] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x0000,
    0x0000, 0x7C5E, 0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x1033, 0x0000,
    0x0000, 0x7C5E, 0x7C5E, 0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x1033, 0x1033, 0x0000,
    0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x1033, 0x1033, 0x1033, 0x0000,
    0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x1033, 0x1033, 0x1033, 0x1033, 0x0000,
    0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x7C5E, 0x7C5E, 0x0000, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x0000,
    0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x0000, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x0000,
    0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x0000, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x0000,
    0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x1033, 0x1033, 0x0000, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x0000,
    0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x1033, 0x1033, 0x1033, 0x1033, 0x0000, 0x1033, 0x1033, 0x1033, 0x1033, 0x0000,
    0x0000, 0x7C5E, 0x7C5E, 0x7C5E, 0x0000, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x0000, 0x1033, 0x1033, 0x1033, 0x0000,
    0x0000, 0x7C5E, 0x7C5E, 0x0000, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x0000, 0x1033, 0x1033, 0x0000,
    0x0000, 0x7C5E, 0x0000, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x0000, 0x1033, 0x0000,
    0x0000, 0x0000, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x1033, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

extern "C" void vgs_setup()
{
    vgs.gfx.startWrite();
    vgs.gfx.clear(0);
    vgs.gfx.endWrite();
    vgs.setFrameRate(30);
}

extern "C" void vgs_loop()
{
    vgs.gfx.startWrite();
    vgs.gfx.image(rand() % (vgs.gfx.getWidth() - 16), rand() % (vgs.gfx.getHeight() - 16), 16, 16, imageTestData);
    vgs.gfx.endWrite();
}

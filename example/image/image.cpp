#include "vgssdk.h"
#include <stdlib.h>

extern VGS vgs;
extern const unsigned short imageTestData[256];

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
    vgs.gfx.image(rand() % 224, rand() % 304, 16, 16, imageTestData);
    vgs.gfx.endWrite();
}

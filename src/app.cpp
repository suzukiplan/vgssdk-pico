#include <stdlib.h>
#include "vgssdk.h"

extern VGS vgs;

extern "C" void vgs_setup()
{
    return;

}

extern "C" void vgs_loop()
{
    vgs.gfx.pixel(rand() % vgs.getDisplayWidth(), rand() % vgs.getDisplayHeight(), rand() & 0xFFFF);
    return;
}
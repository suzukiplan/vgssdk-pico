#include "vgssdk.h"

extern VGS vgs;

extern "C" void vgs_setup()
{
    vgs.gfx.startWrite();
    for (int i = 0; i < 32; i++) {
        int y = 0;
        vgs.gfx.boxf(8 + i * 6, 8 + 8 * y++, 5, 5, i << 11);                           // Red
        vgs.gfx.boxf(8 + i * 6, 8 + 8 * y++, 5, 5, i << 6 | 0x0020);                   // Green
        vgs.gfx.boxf(8 + i * 6, 8 + 8 * y++, 5, 5, i);                                 // Blue
        vgs.gfx.boxf(8 + i * 6, 8 + 8 * y++, 5, 5, (i << 11) + i);                     // Red + Blue
        vgs.gfx.boxf(8 + i * 6, 8 + 8 * y++, 5, 5, (i << 6 | 0x0020) + i);             // Green + Blue
        vgs.gfx.boxf(8 + i * 6, 8 + 8 * y++, 5, 5, (i << 11) + (i << 6 | 0x0020));     // Red + Green
        vgs.gfx.boxf(8 + i * 6, 8 + 8 * y++, 5, 5, (i << 11) + (i << 6 | 0x0020) + i); // Red + Green + Blue
    }
    vgs.gfx.endWrite();
    vgs.setFrameRate(30);
}

extern "C" void vgs_loop()
{
}

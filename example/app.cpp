#include "vgssdk.h"
#include <stdlib.h>
#include <time.h>

extern VGS vgs;
extern const unsigned char bgmTestData[2229];
extern const unsigned short imageTestData[256];

extern "C" void vgs_setup()
{
    srand(time(nullptr));
    vgs.gfx.clear(0x001F);
    vgs.gfx.box(8, 8, 240 - 16, 32, 0x07E0);
    vgs.gfx.boxf(8, 48, 240 - 16, 32, 0xF800);
    vgs.bgm.load(bgmTestData, sizeof(bgmTestData));
    return;
}

extern "C" void vgs_loop()
{
    static int tx;
    static int ty;
    vgs.gfx.pixel(rand() % vgs.getDisplayWidth(), rand() % vgs.getDisplayHeight(), rand() & 0xFFFF);
    vgs.gfx.line(rand() % vgs.getDisplayWidth(), rand() % vgs.getDisplayHeight(), rand() % vgs.getDisplayWidth(), rand() % vgs.getDisplayHeight(), rand() & 0xFFFF);
    vgs.gfx.image(rand() % vgs.getDisplayWidth() - 8, rand() % vgs.getDisplayHeight() - 8, 16, 16, (unsigned short*)imageTestData);
    if (vgs.io.touch.on) {
        if (tx != vgs.io.touch.x || ty != vgs.io.touch.y) {
            tx = vgs.io.touch.x;
            ty = vgs.io.touch.y;
            printf("touch %d, %d\n", tx, ty);
        }
    } else {
        tx = -1;
        ty = -1;
    }
    return;
}
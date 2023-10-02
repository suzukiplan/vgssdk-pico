#include "vgssdk.h"
#include <stdlib.h>
#include <time.h>

extern VGS vgs;
extern const unsigned char bgmTestData[2229];
extern const unsigned short imageTestData[256];
VGS::GFX vgfx(32, 32);

extern "C" void vgs_setup()
{
    srand(time(nullptr));
    vgs.gfx.startWrite();
    vgs.gfx.clear(0x001F);
    vgs.gfx.box(8, 8, 240 - 16, 32, 0x07E0);
    vgs.gfx.boxf(8, 48, 240 - 16, 32, 0xF800);
    vgfx.image(0, 0, 16, 16, (unsigned short*)imageTestData);
    vgfx.image(16, 0, 16, 16, (unsigned short*)imageTestData);
    vgfx.image(0, 16, 16, 16, (unsigned short*)imageTestData);
    vgfx.image(16, 16, 16, 16, (unsigned short*)imageTestData);
    vgfx.boxf(4, 4, 4, 4, 0xF800);
    vgfx.boxf(24, 4, 4, 4, 0x001F);
    vgfx.boxf(4, 24, 4, 4, 0xF81F);
    vgfx.boxf(24, 24, 4, 4, 0x07E0);
    vgfx.boxf(6, 6, 20, 20, 0xFFFF);
    vgfx.line(6, 6, 26, 26, 0x0000);
    vgfx.line(6, 26, 26, 6, 0x0000);
    vgs.bgm.load(bgmTestData, sizeof(bgmTestData));
    vgs.gfx.endWrite();
    vgs.setFrameRate(30);
}

extern "C" void vgs_loop()
{
    static int tx;
    static int ty;
    vgs.gfx.startWrite();
    vgs.gfx.pixel(rand() % vgs.gfx.getWidth(), rand() % vgs.gfx.getHeight(), rand() & 0xFFFF);
    vgs.gfx.line(rand() % vgs.gfx.getWidth(), rand() % vgs.gfx.getHeight(), rand() % vgs.gfx.getWidth(), rand() % vgs.gfx.getHeight(), rand() & 0xFFFF);
    vgs.gfx.image(rand() % vgs.gfx.getWidth() - 8, rand() % vgs.gfx.getHeight() - 8, 16, 16, (unsigned short*)imageTestData);
    vgfx.push(rand() % vgs.gfx.getWidth() - 16, rand() % vgs.gfx.getHeight() - 16);
    vgs.gfx.endWrite();
    if (vgs.io.touch.on) {
        if (tx != vgs.io.touch.x || ty != vgs.io.touch.y) {
            tx = vgs.io.touch.x;
            ty = vgs.io.touch.y;
            printf("touch %d, %d\n", tx, ty);
        }
        vgs.bgm.pause();
    } else {
        tx = -1;
        ty = -1;
        vgs.bgm.resume();
    }
}

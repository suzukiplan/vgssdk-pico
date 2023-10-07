#include "vgssdk.h"

extern VGS vgs;

extern "C" void vgs_setup()
{
    vgs.gfx.startWrite();
    vgs.gfx.clear(0);
    vgs.gfx.endWrite();
    vgs.setFrameRate(30);
}

extern "C" void vgs_loop()
{
    static bool prevTouch = false;
    static int prevX;
    static int prevY;
    if (!vgs.io.touch.on) {
        prevTouch = false;
        return;
    }
    vgs.gfx.startWrite();
    if (!prevTouch) {
        vgs.gfx.pixel(vgs.io.touch.x, vgs.io.touch.y, 0xFFFF);
    } else {
        vgs.gfx.line(prevX, prevY, vgs.io.touch.x, vgs.io.touch.y, 0xFFFF);
    }
    vgs.gfx.endWrite();
    prevTouch = true;
    prevX = vgs.io.touch.x;
    prevY = vgs.io.touch.y;
}

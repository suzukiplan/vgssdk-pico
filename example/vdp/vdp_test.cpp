#include "vgssdk.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

extern "C" {
extern const unsigned short rom_small_font[1216]; // small_font.c
};
extern VGS vgs;

static void printSmallFont(int x, int y, const char* format, ...)
{
    char buf[64];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    for (int i = 0; buf[i] && i < 64; i++, x += 4) {
        if ('0' <= buf[i] && buf[i] <= '9') {
            vgs.gfx.image(x, y, 4, 8, &rom_small_font[(buf[i] - '0') * 32]);
        } else if ('A' <= buf[i] && buf[i] <= 'Z') {
            vgs.gfx.image(x, y, 4, 8, &rom_small_font[320 + (buf[i] - 'A') * 32]);
        } else if ('.' == buf[i]) {
            vgs.gfx.image(x, y, 4, 8, &rom_small_font[320 + 832]);
        } else if (':' == buf[i]) {
            vgs.gfx.image(x, y, 4, 8, &rom_small_font[320 + 832 + 32]);
        } else {
            vgs.gfx.boxf(x, y, 4, 8, 0b0001000101001000);
        }
    }
}

extern "C" void vgs_setup()
{
    vgs.vdp.begin(192, 240);
    vgs.gfx.startWrite();
    vgs.gfx.clear(0);

    // render grid
    for (int y = 0; y < vgs.gfx.getHeight(); y += 8) {
        for (int x = 0; x < vgs.gfx.getWidth(); x += 8) {
            vgs.gfx.lineH(x, y, 8, 0x001F);
            vgs.gfx.lineV(x, y, 8, 0x001F);
            vgs.gfx.lineH(x, y + 7, 8, 0x0007);
            vgs.gfx.lineV(x + 7, y, 8, 0x0007);
        }
    }

    vgs.gfx.endWrite();
}

extern "C" void vgs_loop()
{
    static int fps = 0;
    static time_t prevTime = time(nullptr);

    // VRAM -> VDP display
    vgs.vdp.execute();

    vgs.gfx.startWrite();

    // VDP display -> LCD
    vgs.gfx.image((vgs.gfx.getWidth() - vgs.vdp.display->width) / 2,
                  (vgs.gfx.getHeight() - vgs.vdp.display->height) / 2,
                  vgs.vdp.display->width,
                  vgs.vdp.display->height,
                  vgs.vdp.display->buf);

    // render frame rate
    fps++;
    auto now = time(nullptr);
    if (now != prevTime) {
        char buf[64];
        snprintf(buf, sizeof(buf), " %dFPS ", fps);
        printSmallFont(vgs.gfx.getWidth() - 4 * strlen(buf) - 4, vgs.gfx.getHeight() - 12, buf);
        fps = 0;
        prevTime = now;
    }

    vgs.gfx.endWrite();
}

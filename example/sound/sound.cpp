#include "roms.hpp"
#include "vgssdk.h"
#include <stdarg.h>
#include <stdio.h>

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
            vgs.gfx.image(x, y, 4, 8, &rom_small_font[(buf[i] - '0') * 32], 0b0001000101001000);
        } else if ('A' <= buf[i] && buf[i] <= 'Z') {
            vgs.gfx.image(x, y, 4, 8, &rom_small_font[320 + (buf[i] - 'A') * 32], 0b0001000101001000);
        } else if ('.' == buf[i]) {
            vgs.gfx.image(x, y, 4, 8, &rom_small_font[320 + 832], 0b0001000101001000);
        } else if (':' == buf[i]) {
            vgs.gfx.image(x, y, 4, 8, &rom_small_font[320 + 832 + 32], 0b0001000101001000);
        }
    }
}

class Position
{
  public:
    int x;
    int y;
    int w;
    int h;

    Position()
    {
        this->set(0, 0, 0, 0);
    }

    Position(int x, int y, int w, int h)
    {
        this->set(x, y, w, h);
    }

    void set(int x, int y, int w, int h)
    {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }

    inline bool hitCheck(int tx, int ty)
    {
        return x <= tx && tx < x + w && y <= ty && ty < y + h;
    }
};

static Position pos[3];

extern "C" void vgs_setup()
{
    vgs.gfx.startWrite();
    vgs.gfx.clear(0);
    auto w = (vgs.gfx.getWidth() - 32) / 3;
    for (int i = 0; i < 3; i++) {
        pos[i].set(8 + i * (w + 8), 8, w, 32);
        vgs.gfx.boxf(pos[i].x, pos[i].y, pos[i].w, pos[i].h, 0x0007);
        vgs.gfx.box(pos[i].x, pos[i].y, pos[i].w, pos[i].h, 0xFFFF);
        printSmallFont(pos[i].x + (pos[i].w - 36) / 2, pos[i].y + (pos[i].h - 8) / 2, "PLAY EFF%d", i + 1);
    }
    vgs.gfx.endWrite();
    vgs.bgm.load(rom_bgm, sizeof(rom_bgm));
    vgs.setFrameRate(30);
}

extern "C" void vgs_loop()
{
    static bool prevOn = false;
    static const short* effData[3] = {
        rom_eff1,
        rom_eff2,
        rom_eff3,
    };
    static const size_t effSize[3] = {
        sizeof(rom_eff1),
        sizeof(rom_eff2),
        sizeof(rom_eff3),
    };

    if (!prevOn && vgs.io.touch.on) {
        for (int i = 0; i < 3; i++) {
            if (pos[i].hitCheck(vgs.io.touch.x, vgs.io.touch.y)) {
                vgs.eff.play(effData[i], effSize[i]);
                break;
            }
        }
    }

    prevOn = vgs.io.touch.on;
}

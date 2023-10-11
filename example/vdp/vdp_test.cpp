#include "roms.hpp"
#include "vgssdk.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
    // 領域サイズ 192x240 ピクセルの VDP を作成
    vgs.vdp.create(192, 240);

    // VDP のパターンエリアに画像を読み込む
    memcpy(vgs.vdp.vram->ptn, rom_vram_ptn, sizeof(rom_vram_ptn));

    // ネームテーブルにパターン番号を設定
    for (int i = 0; i < 4096; i++) {
        vgs.vdp.setBg(i, i % 7);
    }

    // スプライト（VDP）で HELLO,WORLD! を描画
    const char* str = "HELLO,WORLD!";
    int x = (vgs.vdp.getWidth() - strlen(str) * 8) / 2;
    int y = (vgs.vdp.getHeight() - 8) / 2;
    unsigned char n = 0;
    for (; str[n]; n++, x += 8) {
        vgs.vdp.setOam(n, x, y, str[n]);
    }

    // 残りのスプライトは画面上を動くボールにする
    for (; n; n++) {
        vgs.vdp.setOam(n, rand() % (vgs.vdp.getWidth() - 8), rand() % (vgs.vdp.getHeight() - 8), 16 + rand() % 8);
    }

    // VDP 外部の領域に適当なグリッド線を描画しておく
    vgs.gfx.startWrite();
    vgs.gfx.clear(0);
    for (int y = 0; y < vgs.gfx.getHeight(); y += 8) {
        for (int x = 0; x < vgs.gfx.getWidth(); x += 8) {
            vgs.gfx.lineH(x, y, 8, 0x001F);
            vgs.gfx.lineV(x, y, 8, 0x001F);
            vgs.gfx.lineH(x, y + 7, 8, 0x0007);
            vgs.gfx.lineV(x + 7, y, 8, 0x0007);
        }
    }
    vgs.gfx.endWrite();

    // BGMを再生
    vgs.bgm.load(rom_bgm, sizeof(rom_bgm));

    // macOS or Linux（シミュレータ）は 20fps で動かす
    // ※実機（RP2040）は全力でブン回す
    vgs.setFrameRate(20);
}

extern "C" void vgs_loop()
{
    static int fps = 0;
    static time_t prevTime = time(nullptr);
    static bool prevTouch = false;
    static int sx = 0;
    static int sy = 1;

    // 画面をタップしたらスクロールの方向をランダムで変更
    if (vgs.io.touch.on && !prevTouch) {
        sx = rand() % 6 - 3;
        sy = rand() % 6 - 3;
        vgs.eff.play(rom_eff1, sizeof(rom_eff1));
    }
    vgs.vdp.addScroll(sx, sy);
    prevTouch = vgs.io.touch.on;

    // ボールを下に落とす
    for (unsigned char i = 12; i; i++) {
        auto oam = vgs.vdp.getOam(i);
        oam->ptn++;
        oam->ptn &= 0x0F;
        oam->ptn |= 0x10;
        oam->y += 3;
        if (vgs.vdp.getHeight() < oam->y) {
            oam->x = rand() % (vgs.vdp.getWidth() - 8);
            oam->y = -8;
        }
    }

    // start rendering
    vgs.gfx.startWrite();

    // VDP の VRAM の内容を LCD の指定座標に描画
    vgs.vdp.render((vgs.gfx.getWidth() - vgs.vdp.getWidth()) / 2,
                   (vgs.gfx.getHeight() - vgs.vdp.getHeight()) / 2);

    // 1秒間に何フレーム描画できたか表示
    fps++;
    auto now = time(nullptr);
    if (now != prevTime) {
        char buf[64];
        snprintf(buf, sizeof(buf), " %dFPS ", fps);
        printSmallFont(vgs.gfx.getWidth() - 4 * strlen(buf) - 4, vgs.gfx.getHeight() - 12, buf);
        fps = 0;
        prevTime = now;
    }

    // end rendering
    vgs.gfx.endWrite();
}

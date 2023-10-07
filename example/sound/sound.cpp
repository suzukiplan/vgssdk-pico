#include "vgssdk.h"

extern VGS vgs;
extern const unsigned char bgmTestData[2229];

extern "C" void vgs_setup()
{
    vgs.bgm.load(bgmTestData, sizeof(bgmTestData));
    vgs.setFrameRate(30);
}

extern "C" void vgs_loop()
{
    // todo: add pause resume button
    // todo: add play eff
}
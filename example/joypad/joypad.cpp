#include "roms.hpp"
#include "vgssdk.h"

extern VGS vgs;

extern "C" void vgs_setup()
{
    vgs.io.setJoypadEnabled(true);
    vgs.gfx.startWrite();
    vgs.gfx.clear(0);
    vgs.gfx.setViewport(30, 40, 180, 80);
    vgs.gfx.image(0, 0, 180, 80, rom_joypad);
    vgs.gfx.endWrite();
    vgs.setFrameRate(30);
}

extern "C" void vgs_loop()
{
    vgs.gfx.startWrite();
    vgs.gfx.setViewport(30, 40, 180, 80);
    vgs.gfx.image(24, 32, 12, 12, vgs.io.joypad.up ? rom_joypad_cursor_on : rom_joypad_cursor_off);
    vgs.gfx.image(24, 56, 12, 12, vgs.io.joypad.down ? rom_joypad_cursor_on : rom_joypad_cursor_off);
    vgs.gfx.image(12, 44, 12, 12, vgs.io.joypad.left ? rom_joypad_cursor_on : rom_joypad_cursor_off);
    vgs.gfx.image(36, 44, 12, 12, vgs.io.joypad.right ? rom_joypad_cursor_on : rom_joypad_cursor_off);
    vgs.gfx.image(147, 47, 24, 24, vgs.io.joypad.a ? rom_joypad_button_on : rom_joypad_button_off);
    vgs.gfx.image(121, 47, 24, 24, vgs.io.joypad.b ? rom_joypad_button_on : rom_joypad_button_off);
    vgs.gfx.image(58, 61, 28, 7, vgs.io.joypad.select ? rom_joypad_ctrl_on : rom_joypad_ctrl_off);
    vgs.gfx.image(86, 61, 28, 7, vgs.io.joypad.start ? rom_joypad_ctrl_on : rom_joypad_ctrl_off);
    vgs.gfx.endWrite();
}

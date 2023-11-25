/**
 * VGS SDK Pico for SDL2 (macOS and Linux)
 * License under MIT: https://github.com/suzukiplan/vgssdk-pico/blob/master/LICENSE.txt
 * (C)2023, SUZUKI PLAN
 */
#include "SDL.h"
#include "vgsdecv.hpp"
#include "vgssdk.h"
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef VGSVDP_DISPLAY_LIMIT
#define VGSVDP_DISPLAY_LIMIT 90
#endif

#ifndef VGSGFX_ROTATION
#define VGSGFX_ROTATION 2 // default = reverse portrait
#endif

#if VGSGFX_ROTATION == 0
#define VGS_DISPLAY_WIDTH 240
#define VGS_DISPLAY_HEIGHT 320
#elif VGSGFX_ROTATION == 1
#define VGS_DISPLAY_WIDTH 320
#define VGS_DISPLAY_HEIGHT 240
#elif VGSGFX_ROTATION == 2
#define VGS_DISPLAY_WIDTH 240
#define VGS_DISPLAY_HEIGHT 320
#define REVERSE_SCREEN
#elif VGSGFX_ROTATION == 3
#define VGS_DISPLAY_WIDTH 320
#define VGS_DISPLAY_HEIGHT 240
#define REVERSE_SCREEN
#endif

#define abs_(x) (x >= 0 ? (x) : -(x))
#define sgn_(x) (x >= 0 ? (1) : (-1))

VGS vgs;
static SDL_Window* window;
static SDL_Surface* windowSurface;
static unsigned short vdp_display_buf[VGSVDP_DISPLAY_LIMIT * 512];
static VGS::VDP::RAM vdp_vram;
static SDL_AudioDeviceID bgmAudioDeviceId;
static bool bgmLoaded;

static void log(const char* format, ...)
{
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    puts(buf);
}

static inline unsigned int bit5to8(unsigned char bit5)
{
    unsigned char bit8 = bit5 | ((bit5 & 0b11100000) >> 5);
    unsigned int result = bit8;
    result <<= 8;
    result |= bit8;
    result <<= 8;
    result |= bit8;
    result <<= 8;
    result |= bit8;
    return result;
}

static inline unsigned int bit6to8(unsigned char bit6)
{
    unsigned char bit8 = bit6 | ((bit6 & 0b11000000) >> 6);
    unsigned int result = bit8;
    result <<= 8;
    result |= bit8;
    result <<= 8;
    result |= bit8;
    result <<= 8;
    result |= bit8;
    return result;
}

static inline unsigned int color16to32(unsigned short rgb565)
{
    unsigned int color32 = bit5to8((rgb565 & 0b1111100000000000) >> 8) & windowSurface->format->Rmask;
    color32 |= bit6to8((rgb565 & 0b0000011111100000) >> 3) & windowSurface->format->Gmask;
    color32 |= bit5to8((rgb565 & 0b0000000000011111) << 3) & windowSurface->format->Bmask;
    color32 |= windowSurface->format->Amask;
    return color32;
}

static inline unsigned short flip(unsigned short value)
{
    unsigned short result = (value & 0x00FF) << 8;
    result |= (value & 0xFF00) >> 8;
    return result;
}

VGS::GFX::GFX()
{
    log("create GFX for Physical Display (%d, %d)", VGS_DISPLAY_WIDTH, VGS_DISPLAY_HEIGHT);
    memset(&this->vDisplay, 0, sizeof(this->vDisplay));
    this->counter = 0;
    this->clearViewport();
}

VGS::GFX::GFX(int width, int height)
{
    log("create GFX for Virtual Display (%d, %d) = %dKB", width, height, width * height * 2 / 1024 + (width * height * 2 % 1024 ? 1 : 0));
    this->counter = 0;
    this->vDisplay.width = width;
    this->vDisplay.height = height;
    this->vDisplay.buffer = malloc(width * height * 2);
    memset(this->vDisplay.buffer, 0, width * height * 2);
    this->clearViewport();
}

VGS::GFX::~GFX()
{
    if (this->vDisplay.buffer) {
        free(this->vDisplay.buffer);
    }
}

void VGS::GFX::startWrite()
{
    this->counter++;
}

void VGS::GFX::endWrite()
{
    if (0 < this->counter) {
        this->counter--;
        if (0 == this->counter && !this->isVirtual()) {
            SDL_UpdateWindowSurface(window);
        }
    }
}

void VGS::GFX::startWriteSimulatorOnly()
{
    this->startWrite();
}

void VGS::GFX::endWriteSimulatorOnly()
{
    this->endWrite();
}

int VGS::GFX::getWidth()
{
    if (this->isVirtual()) {
        return this->vDisplay.width;
    } else {
        return VGS_DISPLAY_WIDTH;
    }
}

int VGS::GFX::getHeight()
{
    if (this->isVirtual()) {
        return this->vDisplay.height;
    } else {
        return VGS_DISPLAY_HEIGHT;
    }
}

void VGS::GFX::setViewport(int x, int y, int width, int height)
{
    this->viewport.x = x;
    this->viewport.y = y;
    this->viewport.width = width;
    this->viewport.height = height;
}

void VGS::GFX::clearViewport()
{
    this->viewport.x = 0;
    this->viewport.y = 0;
    if (this->isVirtual()) {
        this->viewport.width = this->vDisplay.width;
        this->viewport.height = this->vDisplay.height;
    } else {
        this->viewport.width = VGS_DISPLAY_WIDTH;
        this->viewport.height = VGS_DISPLAY_HEIGHT;
    }
}

void VGS::GFX::clear(unsigned short color)
{
    if (this->isVirtual()) {
        int ptr = 0;
        auto buffer = this->getVirtualBuffer();
        for (int y = 0; y < this->vDisplay.height; y++) {
            for (int x = 0; x < this->vDisplay.width; x++) {
                buffer[ptr++] = color;
            }
        }
    } else {
        auto color32 = color16to32(color);
        auto display = (unsigned int*)windowSurface->pixels;
        for (int y = 0; y < windowSurface->h; y++) {
            for (int x = 0; x < windowSurface->w; x++) {
                display[x] = color32;
            }
            display += windowSurface->pitch / windowSurface->format->BytesPerPixel;
        }
    }
}

void VGS::GFX::pixel(int x, int y, unsigned short color)
{
    if (this->viewport.width <= x || this->viewport.height <= y) {
        return;
    }
    x += this->viewport.x;
    y += this->viewport.y;
    if (x < 0 || y < 0) {
        return;
    }
    if (this->isVirtual()) {
        if (this->vDisplay.width <= x || this->vDisplay.height <= y) {
            return;
        }
        this->getVirtualBuffer()[y * this->vDisplay.width + x] = color;
    } else {
        if (VGS_DISPLAY_WIDTH <= x || VGS_DISPLAY_HEIGHT <= y) {
            return;
        }
        auto display = (unsigned int*)windowSurface->pixels;
        display += y * windowSurface->pitch / windowSurface->format->BytesPerPixel;
        display += x;
        *display = color16to32(color);
    }
}

void VGS::GFX::lineV(int x, int y, int height, unsigned short color)
{
    for (int i = 0; i < height; i++) {
        this->pixel(x, y++, color);
    }
}

void VGS::GFX::lineH(int x, int y, int width, unsigned short color)
{
    for (int i = 0; i < width; i++) {
        this->pixel(x++, y, color);
    }
}

void VGS::GFX::line(int x1, int y1, int x2, int y2, unsigned short color)
{
    if (x1 == x2) {
        this->lineV(x1, y1 < y2 ? y1 : y2, abs(y1 - y2) + 1, color);
    } else if (y1 == y2) {
        this->lineH(x1 < x2 ? x1 : x2, y1, abs(x1 - x2) + 1, color);
    }
    int ia, ib, ie;
    int w;
    int idx = x2 - x1;
    int idy = y2 - y1;
    w = 1;
    ia = abs_(idx);
    ib = abs_(idy);
    if (ia >= ib) {
        ie = -abs_(idy);
        while (w) {
            this->pixel(x1, y1, color);
            if (x1 == x2) {
                break;
            }
            x1 += sgn_(idx);
            ie += 2 * ib;
            if (ie >= 0) {
                y1 += sgn_(idy);
                ie -= 2 * ia;
            }
        }
    } else {
        ie = -abs_(idx);
        while (w) {
            this->pixel(x1, y1, color);
            if (y1 == y2) {
                break;
            }
            y1 += sgn_(idy);
            ie += 2 * ia;
            if (ie >= 0) {
                x1 += sgn_(idx);
                ie -= 2 * ib;
            }
        }
    }
}

void VGS::GFX::box(int x, int y, int width, int height, unsigned short color)
{
    if (0 < width && 0 < height) {
        this->lineH(x, y, width, color);
        this->lineH(x, y + height - 1, width, color);
        this->lineV(x, y, height, color);
        this->lineV(x + width - 1, y, height, color);
    }
}

void VGS::GFX::boxf(int x, int y, int width, int height, unsigned short color)
{
    if (0 < width && 0 < height) {
        for (int i = 0; i < height; i++) {
            this->lineH(x, y++, width, color);
        }
    }
}

void VGS::GFX::image(int x, int y, int width, int height, const unsigned short* buffer)
{
    int ptr = 0;
    for (int yy = 0; yy < height; yy++) {
        for (int xx = 0; xx < width; xx++) {
            this->pixel(x + xx, y + yy, flip(buffer[ptr++]));
        }
    }
}

void VGS::GFX::image(int x, int y, int width, int height, const unsigned short* buffer, unsigned short transparent)
{
    int ptr = 0;
    for (int yy = 0; yy < height; yy++) {
        for (int xx = 0; xx < width; xx++) {
            auto color = buffer[ptr++];
            if (color != transparent) {
                this->pixel(x + xx, y + yy, flip(color));
            }
        }
    }
}

void VGS::GFX::push(int x, int y)
{
    if (!this->isVirtual()) return;
    int ptr = 0;
    for (int yy = 0; yy < this->getHeight(); yy++) {
        for (int xx = 0; xx < this->getWidth(); xx++) {
            vgs.gfx.pixel(x + xx, y + yy, this->getVirtualBuffer()[ptr++]);
        }
    }
}

bool VGS::VDP::create(int width, int height)
{
    if (!this->resize(width, height)) {
        return false;
    }
    memset(this->display.buf, 0, width * height * 2);
    memset(this->vram, 0, sizeof(VDP::RAM));
    return true;
}

bool VGS::VDP::resize(int width, int height)
{
    if (sizeof(vdp_display_buf) < width * height * 2) {
        return false;
    }
    this->display.width = width;
    this->display.height = height;
    this->display.buf = vdp_display_buf;
    this->vram = &vdp_vram;
    return true;
}

void VGS::VDP::render(int x, int y)
{
    this->bgToDisplay();
    this->spriteToDisplay();
    vgs.gfx.image(x, y, this->display.width, this->display.height, this->display.buf);
}

static inline void playSoundEffect(VGS::SoundEffect* eff, short* buffer, int count)
{
    if (!eff->context.ptr || eff->context.masterVolume < 1) {
        return;
    }
    for (int i = 0; i < count; i++) {
        int wav = buffer[i];
        int snd = eff->context.ptr[eff->context.cursor];
        snd *= eff->context.masterVolume;
        snd /= 100;
        wav += snd;
        if (32767 < wav) {
            wav = 32767;
        } else if (wav < -32768) {
            wav = -32768;
        }
        buffer[i] = (short)wav;
        eff->context.cursor++;
        if (eff->context.count <= eff->context.cursor) {
            eff->context.ptr = nullptr;
            return;
        }
    }
}

static void bgmCallback(void* userdata, Uint8* stream, int len)
{
    auto eff = &((VGS*)userdata)->eff;
    if (bgmLoaded) {
        auto bgm = &((VGS*)userdata)->bgm;
        auto decoder = (VGSDecoder*)bgm->getContext();
        decoder->execute(stream, len);
        playSoundEffect(eff, (short*)stream, len / 2);
    } else {
        memset(stream, 0, len);
        playSoundEffect(eff, (short*)stream, len / 2);
    }
}

VGS::BGM::BGM()
{
    this->context = new VGSDecoder();
    bgmLoaded = false;
}

VGS::BGM::~BGM()
{
    delete (VGSDecoder*)this->context;
}

void VGS::BGM::pause()
{
    if (bgmAudioDeviceId && !this->paused) {
        SDL_PauseAudioDevice(bgmAudioDeviceId, 1);
        this->paused = true;
    }
}

void VGS::BGM::resume()
{
    if (bgmAudioDeviceId && this->paused) {
        SDL_PauseAudioDevice(bgmAudioDeviceId, 0);
        this->paused = false;
    }
}

void VGS::BGM::load(const void* buffer, size_t size)
{
    if (bgmAudioDeviceId) {
        this->pause();
        SDL_LockAudioDevice(bgmAudioDeviceId);
        bgmLoaded = ((VGSDecoder*)this->context)->load(buffer, size);
        SDL_UnlockAudioDevice(bgmAudioDeviceId);
        this->resume();
    }
}

int VGS::BGM::getMasterVolume()
{
    return ((VGSDecoder*)this->context)->getMasterVolume();
}

void VGS::BGM::setMasterVolume(int masterVolume)
{
    ((VGSDecoder*)this->context)->setMasterVolume(masterVolume);
}

void VGS::BGM::fadeout()
{
    ((VGSDecoder*)this->context)->fadeout();
}

void VGS::BGM::seekTo(int time, void (*callback)(int percent))
{
    if (bgmAudioDeviceId) {
        this->pause();
        ((VGSDecoder*)this->context)->seekTo(time, callback);
        this->resume();
    }
}

bool VGS::BGM::isPlayEnd()
{
    return ((VGSDecoder*)this->context)->isPlayEnd();
}

int VGS::BGM::getLoopCount()
{
    return ((VGSDecoder*)this->context)->getLoopCount();
}

unsigned char VGS::BGM::getTone(int cn)
{
    if (0 <= cn && cn < 6) {
        return ((VGSDecoder*)this->context)->getTone(cn & 0xFF);
    } else {
        return 0xFF;
    }
}

unsigned char VGS::BGM::getKey(int cn)
{
    if (0 <= cn && cn < 6) {
        return ((VGSDecoder*)this->context)->getKey(cn & 0xFF);
    } else {
        return 0xFF;
    }
}

unsigned int VGS::BGM::getLengthTime()
{
    return ((VGSDecoder*)this->context)->getLengthTime();
}

unsigned int VGS::BGM::getLoopTime()
{
    return ((VGSDecoder*)this->context)->getLoopTime();
}

unsigned int VGS::BGM::getDurationTime()
{
    return ((VGSDecoder*)this->context)->getDurationTime();
}

int VGS::BGM::getIndex()
{
    return ((VGSDecoder*)this->context)->getIndex();
}

VGS::SoundEffect::SoundEffect()
{
    memset(&this->context, 0, sizeof(this->context));
    this->context.masterVolume = 100;
}

void VGS::SoundEffect::play(const short* buffer, size_t size)
{
    SDL_LockAudioDevice(bgmAudioDeviceId);
    this->context.ptr = buffer;
    this->context.count = size / 2;
    this->context.cursor = 0;
    SDL_UnlockAudioDevice(bgmAudioDeviceId);
}

VGS::VGS()
{
    this->halt = false;
    this->frameRate = 0;
}

VGS::~VGS()
{
}

void VGS::delay(int ms)
{
    usleep(ms * 1000);
}

void VGS::led(bool on)
{
    log("LED: %s", on ? "on" : "off");
}

void VGS::setFrameRate(int frameRate)
{
    if (frameRate < 1) {
        this->frameRate = 0;
    } else if (frameRate < 15) {
        this->frameRate = 10;
    } else if (frameRate < 25) {
        this->frameRate = 20;
    } else if (frameRate < 35) {
        this->frameRate = 30;
    } else if (frameRate < 45) {
        this->frameRate = 40;
    } else if (frameRate < 55) {
        this->frameRate = 50;
    } else {
        this->frameRate = 60;
    }
}

int main()
{
    log("Booting VGS for SDL2.");
    SDL_version sdlVersion;
    SDL_GetVersion(&sdlVersion);
    log("SDL version: %d.%d.%d", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);

    log("init SDL");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        log("SDL_Init failed: %s", SDL_GetError());
        exit(-1);
    }

    log("init AudioDriver");
    SDL_AudioSpec desired;
    SDL_AudioSpec obtained;
    desired.freq = 22050;
    desired.format = AUDIO_S16LSB;
    desired.channels = 1;
    desired.samples = 2048;
    desired.callback = bgmCallback;
    desired.userdata = &vgs;
    bgmAudioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (0 == bgmAudioDeviceId) {
        log(" ... SDL_OpenAudioDevice failed: %s", SDL_GetError());
        exit(-1);
    }
    log("- obtained.freq = %d", obtained.freq);
    log("- obtained.format = %X", obtained.format);
    log("- obtained.channels = %d", obtained.channels);
    log("- obtained.samples = %d", obtained.samples);
    SDL_PauseAudioDevice(bgmAudioDeviceId, 0);

    log("create SDL window");
    window = SDL_CreateWindow(
        "VGS for SDL2",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        VGS_DISPLAY_WIDTH,
        VGS_DISPLAY_HEIGHT,
        SDL_WINDOW_OPENGL);

    log("Get SDL window surface");
    windowSurface = SDL_GetWindowSurface(window);
    if (!windowSurface) {
        log("SDL_GetWindowSurface failed: %s", SDL_GetError());
        exit(-1);
    }

    log("PixelFormat: %d bits (%d bytes)", (int)windowSurface->format->BitsPerPixel, (int)windowSurface->format->BytesPerPixel);
    log("Rmask: %08X", (int)windowSurface->format->Rmask);
    log("Gmask: %08X", (int)windowSurface->format->Gmask);
    log("Bmask: %08X", (int)windowSurface->format->Bmask);
    log("Amask: %08X", (int)windowSurface->format->Amask);

    if (4 != windowSurface->format->BytesPerPixel) {
        log("unsupported pixel format (support only 4 bytes / pixel)");
        exit(-1);
    }

    log("VDP info");
    log("- display limit: %ld bytes (%ldKB)", sizeof(vdp_display_buf), sizeof(vdp_display_buf) / 1024);
    log("- VRAM size: %ld bytes (%ldKB)", sizeof(vdp_vram), sizeof(vdp_vram) / 1024 + (sizeof(vdp_vram) % 1024 ? 1 : 0));

    vgs_setup();
    SDL_UpdateWindowSurface(window);

    log("Continue to execute vgs_loop while no stop signal is detected...");
    SDL_Event event;
    unsigned int loopCount = 0;
    const int waitFps0[3] = {0, 0, 0};
    const int waitFps10[3] = {100, 100, 100};
    const int waitFps20[3] = {50, 50, 50};
    const int waitFps30[3] = {34, 33, 33};
    const int waitFps40[3] = {25, 25, 25};
    const int waitFps50[3] = {20, 20, 20};
    const int waitFps60[3] = {17, 17, 16};
    const int* waitTime[7] = {waitFps0, waitFps10, waitFps20, waitFps30, waitFps40, waitFps50, waitFps60};

    while (!vgs.halt) {
        loopCount++;
        auto start = std::chrono::system_clock::now();
        if (0 < vgs.getFrameRate()) {
            vgs.gfx.startWrite();
        }
        vgs_loop();
        bool quit = false;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_q: quit = true; break;
                    case SDLK_UP: vgs.io.joypad.up = true && vgs.io.isJoypadEnabled(); break;
                    case SDLK_DOWN: vgs.io.joypad.down = true && vgs.io.isJoypadEnabled(); break;
                    case SDLK_LEFT: vgs.io.joypad.left = true && vgs.io.isJoypadEnabled(); break;
                    case SDLK_RIGHT: vgs.io.joypad.right = true && vgs.io.isJoypadEnabled(); break;
                    case SDLK_z: vgs.io.joypad.b = true && vgs.io.isJoypadEnabled(); break;
                    case SDLK_x: vgs.io.joypad.a = true && vgs.io.isJoypadEnabled(); break;
                    case SDLK_SPACE: vgs.io.joypad.start = true && vgs.io.isJoypadEnabled(); break;
                    case SDLK_ESCAPE: vgs.io.joypad.select = true && vgs.io.isJoypadEnabled(); break;
                }
                if (quit) {
                    break;
                }
            } else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP: vgs.io.joypad.up = false; break;
                    case SDLK_DOWN: vgs.io.joypad.down = false; break;
                    case SDLK_LEFT: vgs.io.joypad.left = false; break;
                    case SDLK_RIGHT: vgs.io.joypad.right = false; break;
                    case SDLK_z: vgs.io.joypad.b = false; break;
                    case SDLK_x: vgs.io.joypad.a = false; break;
                    case SDLK_SPACE: vgs.io.joypad.start = false; break;
                    case SDLK_ESCAPE: vgs.io.joypad.select = false; break;
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                vgs.io.touch.on = true;
                SDL_GetMouseState(&vgs.io.touch.x, &vgs.io.touch.y);
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                vgs.io.touch.on = false;
            } else {
                SDL_GetMouseState(&vgs.io.touch.x, &vgs.io.touch.y);
            }
        }
        if (quit) {
            break;
        }
        if (0 < vgs.getFrameRate()) {
            vgs.gfx.endWrite();
            std::chrono::duration<double> diff = std::chrono::system_clock::now() - start;
            int ms = (int)(diff.count() * 1000);
            int wait = waitTime[vgs.getFrameRate() / 10][loopCount % 3];
            if (ms < wait) {
                usleep((wait - ms) * 1000);
            }
        }
    }

    if (vgs.halt) {
        log("halted by app");
    } else {
        log("halted by system");
    }

    vgs.bgm.pause();
    SDL_CloseAudioDevice(bgmAudioDeviceId);
    bgmAudioDeviceId = 0;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

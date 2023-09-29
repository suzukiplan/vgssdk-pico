#include "SDL.h"
#include "vgsdecv.hpp"
#include "vgssdk.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define abs_(x) (x >= 0 ? (x) : -(x))
#define sgn_(x) (x >= 0 ? (1) : (-1))

VGS vgs(240, 320);
static SDL_Surface* windowSurface;

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
    memset(&this->display, 0, sizeof(this->display));
    this->clearViewport();
}

VGS::GFX::GFX(int width, int height)
{
    memset(&this->viewport, 0, sizeof(this->viewport));
    this->display.width = width;
    this->display.height = height;
    this->display.buffer = (unsigned short*)malloc(width * height * 2);
}

VGS::GFX::~GFX()
{
    if (this->display.buffer) {
        free(this->display.buffer);
    }
}

void VGS::GFX::setViewport(int x, int y, int width, int height)
{
    this->viewport.enabled = true;
    this->viewport.x = x;
    this->viewport.y = y;
    this->viewport.width = width;
    this->viewport.height = height;
}

void VGS::GFX::clear(unsigned short color)
{
    auto color32 = color16to32(color);
    auto display = (unsigned int*)windowSurface->pixels;
    for (int y = 0; y < windowSurface->h; y++) {
        for (int x = 0; x < windowSurface->w; x++) {
            display[x] = color32;
        }
        display += windowSurface->pitch / windowSurface->format->BytesPerPixel;
    }
}

void VGS::GFX::clearViewport()
{
    memset(&this->viewport, 0, sizeof(this->viewport));
}

void VGS::GFX::pixel(int x, int y, unsigned short color)
{
    if (this->viewport.enabled) {
        if (x < 0 || y < 0 || this->viewport.width <= x || this->viewport.height <= y) {
            return;
        }
        x += this->viewport.x;
        y += this->viewport.y;
    }
    if (x < 0 || y < 0 || vgs.getDisplayWidth() <= x || vgs.getDisplayHeight() <= y) {
        return;
    }
    auto display = (unsigned int*)windowSurface->pixels;
    display += y * windowSurface->pitch / windowSurface->format->BytesPerPixel;
    display += x;
    *display = color16to32(color);
}

void VGS::GFX::lineV(int x1, int y1, int y2, unsigned short color)
{
    if (y2 < y1) {
        auto w = y2;
        y2 = y1;
        y1 = w;
    }
    for (; y1 < y2; y1++) {
        this->pixel(x1, y1, color);
    }
}

void VGS::GFX::lineH(int x1, int y1, int x2, unsigned short color)
{
    if (x2 < x1) {
        auto w = x2;
        x2 = x1;
        x1 = w;
    }
    for (; x1 < x2; x1++) {
        this->pixel(x1, y1, color);
    }
}

void VGS::GFX::line(int x1, int y1, int x2, int y2, unsigned short color)
{
    if (x1 == x2) {
        this->lineV(x1, y1, y2, color);
    } else if (y1 == y2) {
        this->lineH(x1, y1, x2, color);
    }
    int ia, ib, ie;
    int w;
    int idx = x2 - x1;
    int idy = y2 - y1;
    if (!idx || !idy) {
        if (x2 < x1) {
            w = x1;
            x1 = x2;
            x2 = w;
        }
        if (y2 < y1) {
            w = y1;
            y1 = y2;
            y2 = w;
        }
        if (0 == idy) {
            for (; x1 <= x2; x1++) {
                this->pixel(x1, y1, color);
            }
        } else {
            for (; y1 <= y2; y1++) {
                this->pixel(x1, y1, color);
            }
        }
        return;
    }
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
        int x2 = x + width - 1;
        int y2 = y + height - 1;
        this->lineH(x, y, x2, color);
        this->lineH(x, y2, x2, color);
        this->lineV(x, y, y2, color);
        this->lineV(x2, y, y2, color);
    }
}

void VGS::GFX::boxf(int x, int y, int width, int height, unsigned short color)
{
    if (0 < width && 0 < height) {
        int x2 = x + width - 1;
        int y2 = y + height;
        for (; y < y2; y++) {
            this->lineH(x, y, x2, color);
        }
    }
}

void VGS::GFX::image(int x, int y, int width, int height, unsigned short* buffer)
{
    int ptr = 0;
    for (int yy = 0; yy < height; yy++) {
        for (int xx = 0; xx < width; xx++) {
            this->pixel(x + xx, y + yy, flip(buffer[ptr++]));
        }
    }
}

static SDL_AudioDeviceID bgmAudioDeviceId;
static bool bgmLoaded;

static void bgmCallback(void* userdata, Uint8* stream, int len)
{
    if (bgmLoaded) {
        auto bgm = (VGS::BGM*)userdata;
        auto decoder = (VGSDecoder*)bgm->getContext();
        decoder->execute(stream, len);
    } else {
        memset(stream, 0, len);
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

VGS::VGS(int displayWidth, int displayHeight)
{
    this->halt = false;
    this->displayWidth = displayWidth;
    this->displayHeight = displayHeight;
}

VGS::~VGS()
{
    SDL_Quit();
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
    desired.userdata = &vgs.bgm;
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
    SDL_Window* window = SDL_CreateWindow(
        "VGS for SDL2",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        vgs.getDisplayWidth(),
        vgs.getDisplayHeight(),
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

    vgs_setup();
    SDL_UpdateWindowSurface(window);

    log("Continue to execute vgs_loop while no stop signal is detected...");
    SDL_Event event;
    while (!vgs.halt) {
        vgs_loop();
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyWindow(window);
                SDL_Quit();
                break;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                vgs.io.touch.on = true;
                SDL_GetMouseState(&vgs.io.touch.x, &vgs.io.touch.y);
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                vgs.io.touch.on = false;
            } else {
                SDL_GetMouseState(&vgs.io.touch.x, &vgs.io.touch.y);
            }
        }
        SDL_UpdateWindowSurface(window);
    }

    if (vgs.halt) {
        log("halted by app");
    } else {
        log("halted by system");
    }

    vgs.bgm.pause();
    SDL_CloseAudioDevice(bgmAudioDeviceId);
    bgmAudioDeviceId = 0;
    return 0;
}

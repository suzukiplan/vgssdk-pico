#include "SDL.h"
#include "vgssdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define abs_(x) (x >= 0 ? (x) : -(x))
#define sgn_(x) (x >= 0 ? (1) : (-1))

VGS vgs(240, 320);
static SDL_Surface* windowSurface;

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

static inline unsigned char bit6to8(unsigned char bit6)
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
    color32 |= bit5to8((rgb565 & 0b0000011111100000) >> 2) & windowSurface->format->Gmask;
    color32 |= bit5to8((rgb565 & 0b0000000000011111) << 3) & windowSurface->format->Bmask;
    color32 |= windowSurface->format->Amask;
    return color32;
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
    if (vgs.getDisplayHeight() <= y2) {
        y2 = vgs.getDisplayHeight() - 1;
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
    if (vgs.getDisplayWidth() <= x2) {
        x2 = vgs.getDisplayWidth() - 1;
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

VGS::BGM::BGM()
{
}

VGS::BGM::~BGM()
{
}

VGS::VGS(int displayWidth, int displayHeight)
{
    this->halt = false;
    this->displayWidth = displayWidth;
    this->displayHeight = displayHeight;
}

VGS::~VGS()
{
}

static void log(const char* format, ...)
{
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    puts(buf);
}

int main()
{
    log("Booting VGS for SDL2.");
    SDL_version sdlVersion;
    SDL_GetVersion(&sdlVersion);
    log("SDL version: %d.%d.%d", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);

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
            }
        }
        SDL_UpdateWindowSurface(window);
    }

    if (vgs.halt) {
        log("halted by app");
    } else {
        log("halted by system");
    }
    return 0;
}

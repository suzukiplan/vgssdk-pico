#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vgssdk.h"
#include "SDL.h"

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
    SDL_Window *window = SDL_CreateWindow(
        "VGS for SDL2",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        vgs.getDisplayWidth(),
        vgs.getDisplayHeight(),
        SDL_WINDOW_OPENGL
    );

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

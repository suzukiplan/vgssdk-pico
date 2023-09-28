#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vgssdk.h"
#include "SDL.h"

VGS vgs;
static SDL_Surface* windowSurface;

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

void VGS::GFX::clearViewport()
{
    memset(&this->viewport, 0, sizeof(this->viewport));
}

VGS::BGM::BGM()
{
}

VGS::BGM::~BGM()
{
}

VGS::VGS()
{
    this->halt = false;
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
        240,
        320,
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

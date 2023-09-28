#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vgssdk.h"
#include "SDL.h"

VGS vgs;

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

    SDL_Window *window = SDL_CreateWindow(
        "VGS for SDL2",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        240,
        320,
        SDL_WINDOW_OPENGL
    );

    log("execute vgs_setup()");
    vgs_setup();

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
    }

    if (vgs.halt) {
        log("halted by app");
    } else {
        log("halted by system");
    }
    return 0;
}

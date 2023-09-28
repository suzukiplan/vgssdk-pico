#ifndef INCLUDE_VGSSDK_H
#define INCLUDE_VGSSDK_H
#include <stdio.h>

class VGS
{
  public:
    class GFX
    {
      private:
        struct VirtualDisplay {
            unsigned short* buffer;
            int width;
            int height;
        } display;

        struct Viewport {
            bool enabled;
            int x;
            int y;
            int width;
            int height;
        } viewport;

      public:
        GFX();
        GFX(int width, int height);
        ~GFX();
        void clear(unsigned short color = 0);
        void setViewport(int x, int y, int width, int height);
        void clearViewport();
        void pixel(int x, int y, unsigned short color);
        void lineV(int x1, int y1, int y2, unsigned short color);
        void lineH(int x1, int y1, int x2, unsigned short color);
        void line(int x1, int y1, int x2, int y2, unsigned short color);
        void box(int x, int y, int width, int height, unsigned short color);
        void boxf(int x, int y, int width, int height, unsigned short color);
        void image(int x, int y, int width, int height, unsigned short* buffer);
    };

    class BGM
    {
      private:
        void* context;

      public:
        BGM();
        ~BGM();
        void load(const void* buffer, size_t size);
        void pause();
        void resume();
        void stop();
        void fadeout();
        bool isPlaying();
        int length();
        int currentPosition();
        void seekTo(int position);
        int loopPosition();
        int tone(int ch);
        int key(int ch);
        int loopCount();
        bool isLoopSong();
    };

    class IO
    {
        unsigned char gamepad();
        bool touch(int* x, int* y);
    };

    bool halt;

    VGS(int displayWidth, int displayHeight);
    ~VGS();
    VGS::GFX gfx;
    VGS::BGM bgm;
    VGS::IO io;
    int getDisplayWidth() { return this->displayWidth; }
    int getDisplayHeight() { return this->displayHeight; }

  private:
    int displayWidth;
    int displayHeight;
};

// Implement on the app side (only once at startup)
extern "C" void vgs_setup();

// Implement on the app side (repeat after init)
extern "C" void vgs_loop();

#endif

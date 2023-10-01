/**
 * VGS SDK Pico
 * License under MIT: https://github.com/suzukiplan/vgssdk-pico/blob/master/LICENSE.txt
 * (C)2023, SUZUKI PLAN
 */
#ifndef INCLUDE_VGSSDK_H
#define INCLUDE_VGSSDK_H
#include <stdio.h>
#include <string.h>

class VGS
{
  public:
    class GFX
    {
      private:
        unsigned int counter;

        struct VirtualDisplay {
            void* buffer;
            int width;
            int height;
        } vDisplay;

        struct Viewport {
            int x;
            int y;
            int width;
            int height;
        } viewport;

      public:
        GFX();
        GFX(int width, int height);
        ~GFX();
        void startWrite();
        void endWrite();
        inline unsigned short* getVirtualBuffer() { return (unsigned short*)vDisplay.buffer; }
        inline bool isVirtual() { return nullptr != this->getVirtualBuffer(); }
        int getWidth();
        int getHeight();
        void clear(unsigned short color = 0);
        void setViewport(int x, int y, int width, int height);
        void clearViewport();
        void pixel(int x, int y, unsigned short color);
        void lineV(int x, int y, int height, unsigned short color);
        void lineH(int x, int y, int width, unsigned short color);
        void line(int x1, int y1, int x2, int y2, unsigned short color);
        void box(int x, int y, int width, int height, unsigned short color);
        void boxf(int x, int y, int width, int height, unsigned short color);
        void image(int x, int y, int width, int height, const unsigned short* buffer);
        void push(int x, int y);
    };

    class BGM
    {
      private:
        void* context;
        bool paused;

      public:
        BGM();
        ~BGM();
        inline void* getContext() { return this->context; }
        void pause();
        void resume();
        inline bool isPaused() { return this->paused; }
        void load(const void* buffer, size_t size);
        int getMasterVolume();
        void setMasterVolume(int masterVolume);
        void fadeout();
        void seekTo(int time);
        bool isPlayEnd();
        int getLoopCount();
        unsigned char getTone(int cn);
        unsigned char getKey(int cn);
        unsigned int getLengthTime();
        unsigned int getLoopTime();
        unsigned int getDurationTime();
    };

    class IO
    {
      public:
        // TODO: Joypad for tohovgs-pico is not implemented yet.
        struct Joypad {
            bool up;     // TODO: Assuming GPIO1 assignment
            bool down;   // TODO: Assuming GPIO2 assignment
            bool left;   // TODO: Assuming GPIO3 assignment
            bool right;  // TODO: Assuming GPIO4 assignment
            bool start;  // TODO: Assuming GPIO5 assignment
            bool select; // TODO: Assuming GPIO6 assignment
            bool a;      // TODO: Assuming GPIO7 assignment
            bool b;      // TODO: Assuming GPIO8 assignment
        } joypad;

        struct Touch {
            bool on;
            int x;
            int y;
        } touch;

        IO()
        {
            memset(&joypad, 0, sizeof(joypad));
            memset(&touch, 0, sizeof(touch));
        }
    };

    bool halt;

    VGS();
    ~VGS();
    VGS::GFX gfx;
    VGS::BGM bgm;
    VGS::IO io;
    void delay(int ms);
    void led(bool on);
    inline void set60FpsMode(bool on) { this->is60Fps = on; }
    inline bool is60FpsMode() { return this->is60Fps; }

  private:
    bool is60Fps;
};

// Implement on the app side (only once at startup)
extern "C" void vgs_setup();

// Implement on the app side (repeat after init)
extern "C" void vgs_loop();

#endif

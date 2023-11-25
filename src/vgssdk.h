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
  private:
    int frameRate;

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
        void startWriteSimulatorOnly();
        void endWriteSimulatorOnly();
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
        void image(int x, int y, int width, int height, const unsigned short* buffer, unsigned short transparent);
        void push(int x, int y);
    };

    class VDP
    {
      private:
        struct Display {
            unsigned short* buf; // width x height x 2 bytes
            int width;
            int height;
        } display;

      public:
        typedef struct OAM_ {
            int x;
            int y;
            unsigned char ptn;
            unsigned char user[3];
        } OAM;

        typedef struct RAM_ {
            unsigned char bg[64][64];  // BG name table: 64x64 (512x512px)
            int scrollX;               // BG scroll (X)
            int scrollY;               // BG scroll (Y)
            OAM oam[256];              // object attribute memory (sprites)
#ifdef VGSVDP_PTNTBL_ROM               //
            const unsigned short* ptn; // charcter pattern (ROM)
#else                                  //
            unsigned short ptn[16384]; // character pattern (8x8px x 2 x 256 bytes = 32KB = 128x128px)
#endif                                 //
        } RAM;

        RAM* vram;

        bool create(int width, int height);
        bool resize(int width, int height);
        void render(int x, int y);
        inline int getWidth() { return this->display.width; }
        inline int getHeight() { return this->display.height; }
        inline OAM* getOam(int index) { return &this->vram->oam[index & 0xFF]; }
        inline void setScrollX(int x) { this->vram->scrollX = x; }
        inline void setScrollY(int y) { this->vram->scrollY = y; }
        inline void addScrollX(int ax) { this->vram->scrollX += ax; }
        inline void addScrollY(int ay) { this->vram->scrollY += ay; }
        inline void setBg(int x, int y, unsigned char ptn) { this->vram->bg[x & 0x3F][y & 0x3F] = ptn; }

        inline bool loadPattern(const unsigned short* rom, size_t size)
        {
#ifdef VGSVDP_PTNTBL_ROM
            if (size != 0x8000) {
                return false;
            } else {
                this->vram->ptn = rom;
                return true;
            }
#else
            if (0x8000 < size) {
                return false;
            } else {
                memcpy(this->vram->ptn, rom, size);
                return true;
            }
#endif
        }

        inline void setScroll(int x, int y)
        {
            this->setScrollX(x);
            this->setScrollY(y);
        }

        inline void addScroll(int ax, int ay)
        {
            this->addScrollX(ax);
            this->addScrollY(ay);
        }

        inline void setBg(int index, unsigned char ptn)
        {
            index &= 0x0FFF;
            this->setBg(index & 0x3F, index / 64, ptn);
        }

        inline void setOam(int index,
                           int x = 0,
                           int y = 0,
                           unsigned char ptn = 0,
                           unsigned char user0 = 0,
                           unsigned char user1 = 0,
                           unsigned char user2 = 0)
        {
            index &= 0xFF;
            this->vram->oam[index].ptn = ptn;
            this->vram->oam[index].x = x;
            this->vram->oam[index].y = y;
            this->vram->oam[index].user[0] = user0;
            this->vram->oam[index].user[1] = user1;
            this->vram->oam[index].user[2] = user2;
        }

      private:
        inline void renderBg(int x, int y, unsigned char ptn)
        {
            if (y < -7 || this->display.height <= y) {
                return; // outside of display (top/bottom)
            }
            int ptnidx = 64 * ptn;
            if (x < 0) {
                if (x < -7) {
                    return; // outside of display (left)
                }
                // need clip left
                for (int i = 0; i < 8; i++, y++) {
                    if (y < 0) {
                        continue; // clip top
                    } else if (this->display.height <= y) {
                        return; // outside of display (bottom)
                    }
                    memcpy(&this->display.buf[y * this->display.width], &this->vram->ptn[ptnidx + i * 8 - x], 16 + x * 2);
                }
            } else if (this->display.width - 8 < x) {
                if (this->display.width <= x) {
                    return; // outside of display (right)
                }
                // need clip right
                auto dx = x - (this->display.width - 8);
                for (int i = 0; i < 8; i++, y++) {
                    if (y < 0) {
                        continue; // clip top
                    } else if (this->display.height <= y) {
                        return; // outside of display (bottom)
                    }
                    memcpy(&this->display.buf[y * this->display.width + x], &this->vram->ptn[ptnidx + i * 8], (8 - dx) * 2);
                }
            } else {
                // no clip left/right
                for (int i = 0; i < 8; i++, y++) {
                    if (y < 0) {
                        continue; // clip top
                    } else if (this->display.height <= y) {
                        return; // outside of display (bottom)
                    }
                    memcpy(&this->display.buf[y * this->display.width + x], &this->vram->ptn[ptnidx + i * 8], 16);
                }
            }
        }

        inline void bgToDisplay()
        {
            int sx = this->vram->scrollX % 8;
            int sy = this->vram->scrollY % 8;
            int py = 64 - this->vram->scrollY / 8 - 1;
            const int h = this->display.height / 8 + (this->display.height % 8 ? 1 : 0);
            const int w = this->display.width / 8 + (this->display.width % 8 ? 1 : 0);
            for (int ny = -1; ny <= h; ny++, py++) {
                py &= 0x3F;
                int px = 64 - this->vram->scrollX / 8 - 1;
                for (int nx = -1; nx <= w; nx++, px++) {
                    this->renderBg(nx * 8 + sx, ny * 8 + sy, this->vram->bg[py][px & 0x3F]);
                }
            }
        }

        inline void spriteToDisplay()
        {
            for (int i = 0; i < 256; i++) {
                auto oam = &this->vram->oam[i];
                if (oam->ptn) {
                    int ptnidx = 64 * oam->ptn;
                    if (-8 < oam->x && oam->x < this->display.width && -8 < oam->y && oam->y < this->display.height) {
                        for (int y = 0; y < 8; y++) {
                            auto ptn = &this->vram->ptn[ptnidx];
                            if (oam->y + y < 0) {
                                continue;
                            } else if (this->display.height <= oam->y + y) {
                                break;
                            }
                            for (int x = 0; x < 8; x++) {
                                auto col = ptn[y * 8 + x];
                                if (col) {
                                    if (oam->x + x < 0) {
                                        continue;
                                    } else if (this->display.width <= oam->x + x) {
                                        break;
                                    }
                                    this->display.buf[(oam->y + y) * this->display.width + oam->x + x] = col;
                                }
                            }
                        }
                    }
                }
            }
        }
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
        void seekTo(int time, void (*callback)(int percent));
        bool isPlayEnd();
        int getLoopCount();
        unsigned char getTone(int cn);
        unsigned char getKey(int cn);
        unsigned int getLengthTime();
        unsigned int getLoopTime();
        unsigned int getDurationTime();
        int getIndex();
    };

    class SoundEffect
    {
      public:
        struct Context {
            const short* ptr;
            int count;
            int cursor;
            int masterVolume;
        } context;

        SoundEffect();
        void play(const short* buffer, size_t size);
        inline int getMasterVolume() { return this->context.masterVolume; }
        inline void setMasterVolume(int masterVolume) { this->context.masterVolume = masterVolume; }
    };

    class IO
    {
      private:
        bool enabledJoypad;

      public:
        struct Joypad {
            bool up;     // GPIO7
            bool down;   // GPIO6
            bool left;   // GPIO8
            bool right;  // GPIO9
            bool start;  // GPIO10
            bool select; // GPIO11
            bool a;      // GPIO2
            bool b;      // GPIO3
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
            enabledJoypad = false;
        }

        void setJoypadEnabled(bool enabled) { enabledJoypad = enabled; }
        bool isJoypadEnabled() { return enabledJoypad; }
    };

    bool halt;

    VGS();
    ~VGS();
    VGS::GFX gfx;
    VGS::BGM bgm;
    VGS::VDP vdp;
    VGS::SoundEffect eff;
    VGS::IO io;
    void delay(int ms);
    void led(bool on);
    void setFrameRate(int frameRate);
    inline int getFrameRate() { return this->frameRate; }
};

// Implement on the app side (only once at startup)
extern "C" void vgs_setup();

// Implement on the app side (repeat after init)
extern "C" void vgs_loop();

#endif

#include "vgsdecv.hpp"
#include "vgssdk.h"
#include <I2S.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <chrono>
#include <math.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VGS_DISPLAY_WIDTH 240
#define VGS_DISPLAY_HEIGHT 320
#define UDA1334A_PIN_DIN 13
#define UDA1334A_PIN_BCLK 14
#define UDA1334A_PIN_WSEL 15
#define VGS_BUFFER_SIZE 4096
#define REVERSE_SCREEN

#define abs_(x) (x >= 0 ? (x) : -(x))
#define sgn_(x) (x >= 0 ? (1) : (-1))

VGS vgs;
static TFT_eSPI tft(VGS_DISPLAY_WIDTH, VGS_DISPLAY_HEIGHT);
static I2S i2s(OUTPUT);

VGS::GFX::GFX()
{
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.init();

    // ディスプレイの向きを初期化
#ifdef REVERSE_SCREEN
    tft.setRotation(2);
    uint16_t touch[] = {
        300,
        3600,
        300,
        3600,
        0b010};
    tft.setTouch(touch);
#else
    tft.setRotation(0);
    uint16_t touch[] = {
        300,
        3600,
        300,
        3600,
        0b100};
    tft.setTouch(touch);
#endif

    tft.fillScreen(0);
}

VGS::GFX::GFX(int width, int height)
{
    this->counter = 0;
    this->vDisplay.width = width;
    this->vDisplay.height = height;
    this->vDisplay.buffer = (unsigned short*)new TFT_eSprite(tft);
}

VGS::GFX::~GFX()
{
    if (this->vDisplay.buffer) {
        delete (TFT_eSprite*)this->vDisplay.buffer;
    }
}

void VGS::GFX::startWrite()
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->startWrite();
    } else {
        tft.startWrite();
    }
}

void VGS::GFX::endWrite()
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->endWrite();
    } else {
        tft.endWrite();
    }
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
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->setViewport(x, y, width, height);
    } else {
        tft.setViewport(x, y, width, height);
    }
}

void VGS::GFX::clearViewport()
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->setViewport(0, 0, this->vDisplay.width, this->vDisplay.height);
    } else {
        tft.setViewport(0, 0, VGS_DISPLAY_WIDTH, VGS_DISPLAY_HEIGHT);
    }
}

void VGS::GFX::clear(unsigned short color)
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->fillScreen(color);
    } else {
        tft.fillScreen(color);
    }
}

void VGS::GFX::pixel(int x, int y, unsigned short color)
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->drawPixel(x, y, color);
    } else {
        tft.drawPixel(x, y, color);
    }
}

void VGS::GFX::lineV(int x1, int y1, int y2, unsigned short color)
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->drawFastVLine(x1, y1, y2, color);
    } else {
        tft.drawFastVLine(x1, y1, y2, color);
    }
}

void VGS::GFX::lineH(int x1, int y1, int x2, unsigned short color)
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->drawFastHLine(x1, y1, x2, color);
    } else {
        tft.drawFastHLine(x1, y1, x2, color);
    }
}

void VGS::GFX::line(int x1, int y1, int x2, int y2, unsigned short color)
{
    if (x1 == x2) {
        this->lineV(x1, y1, y2, color);
    } else if (y1 == y2) {
        this->lineH(x1, y1, x2, color);
    } else if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->drawLine(x1, y1, x2, y2, color);
    } else {
        tft.drawLine(x1, y1, x2, y2, color);
    }
}

void VGS::GFX::box(int x, int y, int width, int height, unsigned short color)
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->drawRect(x, y, width, height, color);
    } else {
        tft.drawRect(x, y, width, height, color);
    }
}

void VGS::GFX::boxf(int x, int y, int width, int height, unsigned short color)
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->fillRect(x, y, width, height, color);
    } else {
        tft.fillRect(x, y, width, height, color);
    }
}

void VGS::GFX::image(int x, int y, int width, int height, unsigned short* buffer)
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->pushImage(x, y, width, height, buffer);
    } else {
        tft.pushImage(x, y, width, height, buffer);
    }
}

void VGS::GFX::push(int x, int y, GFX* gfx)
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->pushSprite(x, y);
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
    this->paused = true;
}

void VGS::BGM::resume()
{
    this->paused = false;
}

void VGS::BGM::load(const void* buffer, size_t size)
{
    vgsLock();
    bgmLoaded = ((VGSDecoder*)this->context)->load(buffer, size);
    this->resume();
    vgsUnlock();
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

VGS::VGS()
{
    this->halt = false;
    this->is60Fps = false;
}

VGS::~VGS()
{
}

void VGS::delay(int ms)
{
    usleep(ms * 1000);
}

void setup1()
{
    i2s.setBCLK(UDA1334A_PIN_BCLK);
    i2s.setDATA(UDA1334A_PIN_DIN);
    i2s.setBitsPerSample(16);
    i2s.setBuffers(16, 128, 0); // 2048 bytes
    i2s.begin(22050);
}

void loop1()
{
    static int16_t buffer[2][VGS_BUFFER_SIZE];
    static int page = 0;
    static int index = 0;
    if (0 == index) {
        page = 1 - page;
    } else if (VGS_BUFFER_SIZE / 2 == index) {
        vgsLock();
        auto context = (VGSDecoder*)vgs.bgm.getContext();
        if (context && !vgs.bgm.isPaused()) {
            context->execute(buffer[1 - page], VGS_BUFFER_SIZE * 2);
        } else {
            memset(buffer[1 - page], 0, VGS_BUFFER_SIZE * 2);
        }
        vgsUnlock();
    }
    i2s.write16(buffer[page][index], buffer[page][index]);
    index = (index + 1) % VGS_BUFFER_SIZE;
}

void setup()
{
    vgs.bgm.setMasterVolume(16);
    vgs_setup();
}

void loop()
{
    unsigned int loopCount = 0;
    const int wait60fps[3] = {17, 17, 16};

    while (!vgs.halt) {
        loopCount++;
        auto start = std::chrono::system_clock::now();
        vgsio.touch.on = tft.getTouch(&vgs.io.touch.x, &vgs.io.touch.y, 20);
        if (vgs.is60FpsMode()) {
            vgs.gfx.startWrite();
        }
        vgs_loop();
        if (vgs.is60FpsMode()) {
            vgs.gfx.endWrite();
            std::chrono::duration<double> diff = std::chrono::system_clock::now() - start;
            int ms = (int)(diff.count() * 1000);
            int wait = wait60fps[loopCount % 3];
            if (ms < wait) {
                vgs.delay((wait - ms) * 1000);
            }
        }
    }
}

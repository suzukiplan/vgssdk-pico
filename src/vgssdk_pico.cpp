#include "FT6336U.hpp"
#include "vgsdecv.hpp"
#include "vgssdk.h"
#include <I2S.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define VGS_BUFFER_SIZE 4096

#define abs_(x) (x >= 0 ? (x) : -(x))
#define sgn_(x) (x >= 0 ? (1) : (-1))

VGS vgs;
static VGSDecoder vgsdec;
static TFT_eSPI tft(240, 320);
static FT6336U ctp(&Wire, CTP_SDA, CTP_SCL, CTP_RST, CTP_INT);
static I2S i2s(OUTPUT);
static semaphore_t vgsSemaphore;
static bool cpu0SetupEnd = false;
static bool bgmLoaded = false;
static unsigned short vdp_display_buf[46080];
static VGS::VDP::RAM vdp_vram;

inline void vgsLock() { sem_acquire_blocking(&vgsSemaphore); }
inline void vgsUnlock() { sem_release(&vgsSemaphore); }

VGS::GFX::GFX()
{
}

VGS::GFX::GFX(int width, int height)
{
    this->counter = 0;
    this->vDisplay.width = width;
    this->vDisplay.height = height;
    auto sprite = new TFT_eSprite(&tft);
    sprite->createSprite(width, height);
    sprite->setColorDepth(16);
    this->vDisplay.buffer = sprite;
    this->clearViewport();
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

void VGS::GFX::startWriteSimulatorOnly()
{
}

void VGS::GFX::endWriteSimulatorOnly()
{
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

void VGS::GFX::image(int x, int y, int width, int height, const unsigned short* buffer)
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->pushImage(x, y, width, height, (uint16_t*)buffer);
    } else {
        tft.pushImage(x, y, width, height, (uint16_t*)buffer);
    }
}

void VGS::GFX::image(int x, int y, int width, int height, const unsigned short* buffer, unsigned short transparent)
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->pushImage(x, y, width, height, (uint16_t*)buffer, transparent);
    } else {
        tft.pushImage(x, y, width, height, (uint16_t*)buffer, transparent);
    }
}

void VGS::GFX::push(int x, int y)
{
    if (this->isVirtual()) {
        ((TFT_eSprite*)this->vDisplay.buffer)->pushSprite(x, y);
    }
}

bool VGS::VDP::create(int width, int height)
{
    if (sizeof(vdp_display_buf) < width * height * 2) {
        return false;
    }
    this->display.width = width;
    this->display.height = height;
    this->display.buf = vdp_display_buf;
    this->vram = &vdp_vram;
    memset(this->display.buf, 0, width * height * 2);
    memset(this->vram, 0, sizeof(VDP::RAM));
    return true;
}

void VGS::VDP::render(int x, int y)
{
    this->bgToDisplay();
    this->spriteToDisplay();
    vgs.gfx.image(x, y, this->display.width, this->display.height, this->display.buf);
}

VGS::BGM::BGM()
{
    this->context = &vgsdec;
    bgmLoaded = false;
}

VGS::BGM::~BGM() {}
void VGS::BGM::pause() { this->paused = true; }
void VGS::BGM::resume() { this->paused = false; }
int VGS::BGM::getMasterVolume() { return vgsdec.getMasterVolume(); }
void VGS::BGM::setMasterVolume(int masterVolume) { vgsdec.setMasterVolume(masterVolume); }
void VGS::BGM::fadeout() { vgsdec.fadeout(); }
bool VGS::BGM::isPlayEnd() { return vgsdec.isPlayEnd(); }
int VGS::BGM::getLoopCount() { return vgsdec.getLoopCount(); }
unsigned char VGS::BGM::getTone(int cn) { return vgsdec.getTone(cn & 0xFF); }
unsigned char VGS::BGM::getKey(int cn) { return vgsdec.getKey(cn & 0xFF); }
unsigned int VGS::BGM::getLengthTime() { return vgsdec.getLengthTime(); }
unsigned int VGS::BGM::getLoopTime() { return vgsdec.getLoopTime(); }
unsigned int VGS::BGM::getDurationTime() { return vgsdec.getDurationTime(); }
int VGS::BGM::getIndex() { return vgsdec.getIndex(); }

void VGS::BGM::load(const void* buffer, size_t size)
{
    vgsLock();
    bgmLoaded = vgsdec.load(buffer, size);
    this->resume();
    vgsUnlock();
}

void VGS::BGM::seekTo(int time, void (*callback)(int percent))
{
    vgsLock();
    vgsdec.seekTo(time, callback);
    this->resume();
    vgsUnlock();
}

VGS::SoundEffect::SoundEffect()
{
    memset(&this->context, 0, sizeof(this->context));
    this->context.masterVolume = 100;
}

void VGS::SoundEffect::play(const short* buffer, size_t size)
{
    vgsLock();
    this->context.ptr = buffer;
    this->context.count = size / 2;
    this->context.cursor = 0;
    vgsUnlock();
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
    delay(ms);
}

void VGS::led(bool on)
{
    digitalWrite(25, on ? HIGH : LOW);
}

void VGS::setFrameRate(int frameRate)
{
    this->frameRate = 0;
}

void setup1()
{
    while (!cpu0SetupEnd) {
        delay(1);
    }
}

static inline void playSoundEffect(short* buffer, int count)
{
    if (!vgs.eff.context.ptr || vgs.eff.context.masterVolume < 1) {
        return;
    }
    for (int i = 0; i < count; i++) {
        int wav = buffer[i];
        int snd = vgs.eff.context.ptr[vgs.eff.context.cursor];
        snd *= vgs.eff.context.masterVolume;
        snd /= 100;
        wav += snd;
        if (32767 < wav) {
            wav = 32767;
        } else if (wav < -32768) {
            wav = -32768;
        }
        buffer[i] = (short)wav;
        vgs.eff.context.cursor++;
        if (vgs.eff.context.count <= vgs.eff.context.cursor) {
            vgs.eff.context.ptr = nullptr;
            return;
        }
    }
}

void loop1()
{
    static int16_t buffer[2][VGS_BUFFER_SIZE];
    static int page = 0;
    static int index = 0;
    if (0 == index) {
        page = 1 - page;
    } else if (VGS_BUFFER_SIZE / 2 == index) {
        if (bgmLoaded && !vgs.bgm.isPaused()) {
            vgsLock();
            vgsdec.execute(buffer[1 - page], VGS_BUFFER_SIZE * 2);
            playSoundEffect(buffer[1 - page], VGS_BUFFER_SIZE);
            vgsUnlock();
        } else {
            memset(buffer[1 - page], 0, VGS_BUFFER_SIZE * 2);
            vgsLock();
            playSoundEffect(buffer[1 - page], VGS_BUFFER_SIZE);
            vgsUnlock();
        }
    }
    i2s.write16(buffer[page][index], buffer[page][index]);
    index = (index + 1) % VGS_BUFFER_SIZE;
}

void setup()
{
    pinMode(25, OUTPUT);
    vgs.led(true);

    // LCD のバックライトを点灯
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // initialize UDA1334A I2S (DAC)
    i2s.setBCLK(DAC_BCLK);
    i2s.setDATA(DAC_DIN);
    i2s.setBitsPerSample(16);
    i2s.setBuffers(16, 128, 0); // 2048 bytes
    i2s.begin(22050);

    sem_init(&vgsSemaphore, 1, 1);
    tft.init();
    tft.invertDisplay(true);
    tft.startWrite();

    // ディスプレイの向きを初期化
    tft.setRotation(VGSGFX_ROTATION);
    vgs.gfx.clear(0);
    tft.endWrite();

    // initialize FT6336U I2C (Capacitive Touch Panel)
    ctp.begin();

    // initialize VGS
    vgs.bgm.setMasterVolume(16);
    vgs.eff.setMasterVolume(16);
    vgs_setup();

    // LED off
    delay(200);
    vgs.led(false);
    cpu0SetupEnd = true;
}

void loop()
{
    vgsLock();
    ctp.scan();
    vgsUnlock();
    vgs.io.touch.on = ctp.status.on;
#if VGSGFX_ROTATION == 0
    vgs.io.touch.x = ctp.status.x;
    vgs.io.touch.y = ctp.status.y;
#elif VGSGFX_ROTATION == 1
    vgs.io.touch.x = ctp.status.y;
    vgs.io.touch.y = 239 - ctp.status.x;
#elif VGSGFX_ROTATION == 2
    vgs.io.touch.x = 239 - ctp.status.x;
    vgs.io.touch.y = 319 - ctp.status.y;
#elif VGSGFX_ROTATION == 3
    vgs.io.touch.x = 319 - ctp.status.y;
    vgs.io.touch.y = ctp.status.x;
#endif
    vgs_loop();
}

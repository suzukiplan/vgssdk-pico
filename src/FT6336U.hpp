/**
 * FT6336U Capacitive touch panel driver for RP2040 (RaspberryPi Pico)
 * 
 * Copyright (c) 2023 Yoji Suzuki.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef INCLUDE_FT6336U_HPP
#define INCLUDE_FT6336U_HPP
#include <Wire.h>
#include <string.h>

#define FT6336U_ADDR 0x38

class FT6336U
{
  private:
    int pinSDA;
    int pinSCL;
    int pinRST;
    int pinINT;
    TwoWire* wire;
    uint8_t regTable[8];
    bool touching;

    void reset()
    {
        digitalWrite(this->pinRST, LOW);
        digitalWrite(this->pinRST, HIGH);
    }

    uint8_t read(uint8_t addr)
    {
        this->wire->beginTransmission(FT6336U_ADDR);
        this->wire->write(addr);
        if (this->wire->endTransmission()) {
            return 0x00;
        }
        this->wire->requestFrom(FT6336U_ADDR, 1);
    }

  public:
    struct Status {
        bool on;
        int x;
        int y;
    } status;

    FT6336U(TwoWire* wire, int pinSDA, int pinSCL, int pinRST, int pinINT)
    {
        this->wire = wire;
        this->pinSDA = pinSDA;
        this->pinSCL = pinSCL;
        this->pinRST = pinRST;
        this->pinINT = pinINT;
        memset(&status, 0, sizeof(status));
        this->touching = false;
    }

    void begin()
    {
        //--------------------------------------------------------
        // You need remove comment of the following code,
        // if you are using none default SDA/SCL ports of I2C0
        // (note: default are SDA = 4 and SCL = 5 in RP2040)
        //--------------------------------------------------------
        //this->wire->setSDA(this->pinSDA);
        //this->wire->setSCL(this->pinSCL);
        this->wire->begin();
        pinMode(this->pinINT, INPUT);
        pinMode(this->pinRST, OUTPUT);
        this->reset();
    }

    void scan()
    {
        if (!this->touching) {
            if (-1 == digitalRead(this->pinINT)) {
                return; // not touching & not interrupted
            }
        }
        this->wire->beginTransmission(FT6336U_ADDR);
        this->wire->write(0);
        this->wire->endTransmission();
        this->wire->requestFrom(FT6336U_ADDR, sizeof(this->regTable));
        for (int i = 0; i < sizeof(this->regTable); i++) {
            this->regTable[i] = this->wire->read();
        }
        auto touchCount = this->regTable[0x02] & 0x0F;
        this->touching = 0 < touchCount;
        this->status.on = this->touching;
        if (this->touching) {
            status.x = this->regTable[0x03] & 0x0F;
            status.x <<= 8;
            status.x |= this->regTable[0x04];
            status.y = this->regTable[0x05] & 0x0F;
            status.y <<= 8;
            status.y |= this->regTable[0x06];
        }
    }
};

#endif

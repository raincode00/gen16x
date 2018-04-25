/*
    gen16x
    Copyright (C) 2018 Jahrain Jackson

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#pragma once


#ifdef _MSC_VER
#define GEN16X_PACK_STRUCT(name) \
    __pragma(pack(push, 1)) struct name __pragma(pack(pop))
#elif (__GNUC__)
#define GEN16X_PACK_STRUCT(name) struct __attribute__((packed)) name
#else
#define GEN16X_PACK_STRUCT(name) struct name
#endif


#define GEN16X_LAYER_NONE           0x00
#define GEN16X_LAYER_DIRECT         0x01
#define GEN16X_LAYER_TILES          0x02
#define GEN16X_LAYER_SPRITES        0x03


#define GEN16X_BLENDMODE_NONE       0x00
#define GEN16X_BLENDMODE_ALPHA      0x01
#define GEN16X_BLENDMODE_ADD        0x02
#define GEN16X_BLENDMODE_SUBTRACT   0x03
#define GEN16X_BLENDMODE_MULTIPLY   0x04

#define GEN16X_TILE8                0x03
#define GEN16X_TILE16               0x04

#define GEN16X_FLAG_TRANSFORM       0b00000001
#define GEN16X_FLAG_CLAMP_X         0b00000010
#define GEN16X_FLAG_CLAMP_Y         0b00000100
#define GEN16X_FLAG_REPEAT_X        0b00001000
#define GEN16X_FLAG_REPEAT_Y        0b00010000

#define GEN16X_FLAG_SPRITE_ENABLED  0b10000000
#define GEN16X_FLAG_SPRITE_VFLIP    0b01000000
#define GEN16X_FLAG_SPRITE_HFLIP    0b00100000


#define GEN16X_SPRITE_WIDTH_MASK    0b00001111
#define GEN16X_SPRITE_HEIGHT_MASK   0b11110000

#define GEN16X_MAX_SCREEN_HEIGHT    256
#define GEN16X_MAX_SCREEN_WIDTH     512
#define GEN16X_MAX_SPRITES          256
#define GEN16X_MAX_SPRITES_PER_ROW  32

#define GEN16X_MAKE_SPRITE_SIZE(w, h)   (((w) & GEN16X_SPRITE_WIDTH_MASK) | ((h) << 4))

#define GEN16X_MAX_DSP_CHANNELS     16

#define GEN16X_DSP_GAIN_NONE        0x0
#define GEN16X_DSP_GAIN_DIRECT      0x1
#define GEN16X_DSP_GAIN_LINEAR      0x2
#define GEN16X_DSP_GAIN_EXPONENTIAL 0x3
#define GEN16X_DSP_MAX_VOLUME       16384
#define GEN16X_DSP_BASE_PITCH       64

#define GEN16X_DSP_OSC_NONE         0x0
#define GEN16X_DSP_OSC_SINE         0x1
#define GEN16X_DSP_OSC_SAW          0x2
#define GEN16X_DSP_OSC_SQUARE       0x3
#define GEN16X_DSP_OSC_NOISE        0x4


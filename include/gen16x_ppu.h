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

#include "gen16x_defs.h"

struct gen16x_color32 {
    union {
        unsigned int color_i;
        struct {
            unsigned char b;
            unsigned char g;
            unsigned char r;
            unsigned char a;
        };
    };
};

struct gen16x_transform {
    union {
        struct {
            int x;
            int y;
            int cx;
            int cy;
            int a;
            int b;
            int c;
            int d;
        };
        struct {
            int m[4][2];
        };
    };
    static const int base = 12;
};

struct gen16x_sprite {
    short x;
    short y;
    short scale_x;
    short scale_y;
    unsigned char flags;
    unsigned char priority;
    unsigned char size;
    unsigned char palette_offset;
    unsigned short tile_index;
};


struct gen16x_layer_header {
    unsigned char layer_type;
    unsigned char blend_mode;
    unsigned int vram_offset;
    union {
        struct {
            unsigned char flags;
            unsigned char priority;
            short scroll_x;
            short scroll_y;
            short width;
            short height;
            
        } direct_layer;
        struct {
            unsigned short flags;
            unsigned char tile_size;
            unsigned char priority;
            unsigned char tilemap_width;
            unsigned char tilemap_height;
            unsigned char palette_offset;
            unsigned int tilemap_vram_offset;
            gen16x_transform transform;
        } tile_layer;
        struct {
            gen16x_sprite sprites[GEN16X_MAX_SPRITES];
        } sprite_layer;
        
        
    };
};
struct gen16x_ppu;

typedef void(*gen16x_row_callback_t)(gen16x_ppu*, unsigned int);

struct gen16x_ppu {
    unsigned short screen_width;
    unsigned short screen_height;
    unsigned int framebuffer_offset;
    gen16x_row_callback_t row_callback;
    gen16x_layer_header layers[6];
    gen16x_color32 cgram32[256];
    unsigned char vram[512*512*4];
};



void gen16x_ppu_render(gen16x_ppu* ppu);

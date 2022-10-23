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
    static const int base = 8;
};

struct gen16x_sprite {
    short x;
    short y;
    short scale_x;
    short scale_y;
    unsigned char flags;
    unsigned char priority;
    unsigned char size_w : 4;
    unsigned char size_h : 4;
    unsigned char palette_offset;
    unsigned short tile_index;
};


struct gen16x_layer_header {
    unsigned int layer_type : 4;
    unsigned int blend_mode : 4;
    unsigned int vram_offset : 24;
    union {
        struct {
            unsigned char flags : 4;
            unsigned char priority : 4;
            short scroll_x;
            short scroll_y;
            short width;
            short height;
            
        } direct_layer;
        struct {
            unsigned int flags : 5;
            unsigned int tile_size : 3;
            unsigned int priority : 4;
            unsigned int tilemap_width : 4;
            unsigned int tilemap_height : 4;
            unsigned int palette_offset : 8;
            unsigned int tilemap_vram_offset;
            gen16x_transform transform;
        } tile_layer;
        struct {
            unsigned int sprites_base : 8;
            unsigned int num_sprites : 8;
        } sprite_layer;
    };
};
struct gen16x_ppu;

typedef void (__cdecl *gen16x_row_callback_t)(gen16x_ppu*, unsigned int, void*);

struct gen16x_ppu {
    unsigned short screen_width;
    unsigned short screen_height;
    gen16x_row_callback_t row_callback;
    void* row_callback_user;
    gen16x_layer_header layers[6];
    gen16x_sprite sprites[GEN16X_MAX_SPRITES];
    gen16x_color32 cgram32[256];
    unsigned char vram[512*256*4];
    unsigned char frambuffer[512*512*4];
};



extern "C" void gen16x_ppu_render(gen16x_ppu * ppu);

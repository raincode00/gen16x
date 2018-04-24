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
#define GEN16X_DSP_MAX_GAIN         16384
#define GEN16X_DSP_MAX_VOLUME       16384
#define GEN16X_DSP_BASE_PITCH       1024


#ifdef _MSC_VER
#define GEN16X_PACK_STRUCT(name) \
    __pragma(pack(push, 1)) struct name __pragma(pack(pop))
#elif (__GNUC__)
#define GEN16X_PACK_STRUCT(name) struct __attribute__((packed)) name
#else
#define GEN16X_PACK_STRUCT(name) struct name
#endif



GEN16X_PACK_STRUCT(gen16x_color32) {
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

GEN16X_PACK_STRUCT(gen16x_transform) {
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
GEN16X_PACK_STRUCT(gen16x_layer_tiles) {
    unsigned char tile_palette[256*256];    //holds 256 16x16 tiles
    unsigned char tile_map[256 * 256];
};

GEN16X_PACK_STRUCT(gen16x_sprite) {
    short x;
    short y;
    unsigned char flags;
    unsigned char priority;
    unsigned char size;
    unsigned char palette_offset;
    unsigned short tile_index;
}; //10 bytes

GEN16X_PACK_STRUCT(gen16x_layer_sprites) {
    unsigned char sprite_palette[1024*128];  // holds 1024 16x16 sprites or 4096 8x8 sprites
};

GEN16X_PACK_STRUCT(gen16x_layer_direct) {
    unsigned char map[512*256];
};

GEN16X_PACK_STRUCT(gen16x_layer_header) {
    unsigned char layer_type;
    unsigned char blend_mode;
    unsigned int vram_offset;
    union {
        struct {
            short flags;
            short scroll_x;
            short scroll_y;
            short width;
            short height;
        } direct_layer;
        struct {
            unsigned char tile_size;
            unsigned char reserved;
            unsigned short tilemap_width;
            unsigned short tilemap_height;
            unsigned short flags;
            gen16x_transform transform;
        } tile_layer;
        struct {
            gen16x_sprite sprites[GEN16X_MAX_SPRITES];
        } sprite_layer;
        
        
    };
};
struct gen16x_ppu;

typedef void(*gen16x_row_callback_t)(gen16x_ppu*, unsigned int);

GEN16X_PACK_STRUCT (gen16x_ppu) {
    unsigned short screen_width;
    unsigned short screen_height;
    gen16x_row_callback_t row_callback;
    gen16x_layer_header layers[6];
    unsigned int framebuffer_offset;
    gen16x_color32 cgram32[256];
    unsigned char vram[512*512*4];
};



void gen16x_ppu_render(gen16x_ppu* ppu);





GEN16X_PACK_STRUCT(gen16x_dsp_channel) {
   
    unsigned char enabled;
    unsigned char mute;
    
    short l_volume;
    short r_volume;
    short pitch;
    
    unsigned char reverb_enable;
    short reverb_delay;
    short reverb_count;
    short reverb_l_volume;
    short reverb_r_volume;
    unsigned int reverb_offset;
    unsigned int reverb_size;
    
    
    unsigned char gain_type;
    short gain_rate;
    short gain_target;
    short current_gain_value;
    
    unsigned char voice_loop;
    unsigned char voice_playing;
    unsigned char stop;
    unsigned int voice_offset;
    unsigned int voice_samples;
    unsigned int voice_time;
    
    
};

GEN16X_PACK_STRUCT(gen16x_spu) {
    
    unsigned char flushed;
    unsigned char underrun;
    int l_volume;
    int r_volume;
    
    int time_setting;
    int time_counter;
    int current_output_position;
    
    int output_offset;
    int output_samples;
    gen16x_dsp_channel channels[GEN16X_MAX_DSP_CHANNELS];
    
    short sram[0x40000];
};



void gen16x_spu_tick(gen16x_spu* spu);

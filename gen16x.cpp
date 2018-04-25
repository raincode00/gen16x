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

#include <string.h>
#include <cmath>
#include "gen16x.h"

inline int clamp0(int x, int b) {
    x = x < 0 ? 0 : (x > b ? b : x);
    return x;
}

inline bool in_range(int x, int low, int high) {
    return ((x - high)*(x - low) <= 0);
}

inline bool out_of_range(int x, int low, int high) {
    return ((x - high)*(x - low) > 0);
}
unsigned short argb32_to_argb16(gen16x_color32 c) {
    c.a = c.a >> 7;
    c.r = c.r >> 3;
    c.g = c.g >> 3;
    c.b = c.b >> 3;
    return (c.a << 15) | (c.r << 10) | (c.g << 5) | c.b;
}

gen16x_color32 argb16_to_argb32(unsigned short c) {
    unsigned int rmask = 0b0111110000000000;
    unsigned int gmask = 0b0000001111100000;
    unsigned int bmask = 0b0000000000011111;

    gen16x_color32 c32;
    c32.a = ((c >> 15)*0xFF);
    c32.r = (((c & rmask) >> 10) << 3) | 0x7;
    c32.g = (((c & gmask) >> 5) << 3);
    c32.b = ((c & bmask) << 3) | 0x7;
    return c32;
}

template<int blendmode>
inline void write_pixel(const gen16x_color32 &src, gen16x_color32 &dst_color);

template<>
inline void write_pixel<GEN16X_BLENDMODE_NONE>(const gen16x_color32 &src, gen16x_color32 &dst_color) {
    if (src.a) {
        dst_color = src;
    }
}
template<>
inline void write_pixel<GEN16X_BLENDMODE_ALPHA>(const gen16x_color32 &src, gen16x_color32 &dst_color) {
    int dst[4];

    dst[1] = dst_color.r;
    dst[2] = dst_color.g;
    dst[3] = dst_color.b;
    dst[1] = (((int)src.r*(src.a)) + ((int)dst[1] * (255 - src.a))) >> 8;
    dst[2] = (((int)src.g*(src.a)) + ((int)dst[2] * (255 - src.a))) >> 8;
    dst[3] = (((int)src.b*(src.a)) + ((int)dst[3] * (255 - src.a))) >> 8;
    dst_color.r = (unsigned char)dst[1];
    dst_color.g = (unsigned char)dst[2];
    dst_color.b = (unsigned char)dst[3];
    return;
}

template<>
inline void write_pixel<GEN16X_BLENDMODE_ADD>(const gen16x_color32 &src, gen16x_color32 &dst_color) {
    int dst[4];
    dst[1] = dst_color.r;
    dst[2] = dst_color.g;
    dst[3] = dst_color.b;
    dst[1] = ((int)src.r*((int)src.a) >> 8) + dst[1];
    dst[2] = ((int)src.g*((int)src.a) >> 8) + dst[2];
    dst[3] = ((int)src.b*((int)src.a) >> 8) + dst[3];
    dst[1] = dst[1] > 255 ? 255 : dst[1];
    dst[2] = dst[2] > 255 ? 255 : dst[2];
    dst[3] = dst[3] > 255 ? 255 : dst[3];
    dst_color.r = (unsigned char)dst[1];
    dst_color.g = (unsigned char)dst[2];
    dst_color.b = (unsigned char)dst[3];
    return;
}
template<>
inline void write_pixel<GEN16X_BLENDMODE_MULTIPLY>(const gen16x_color32 &src, gen16x_color32 &dst_color) {
    int dst[4];
    dst[1] = dst_color.r;
    dst[2] = dst_color.g;
    dst[3] = dst_color.b;
    dst[1] = (((int)src.a*((int)src.r)* dst[1]) >> 16) + (((255 - src.a)*dst[1])>>8);
    dst[2] = (((int)src.a*((int)src.g)* dst[2]) >> 16) + (((255 - src.a)*dst[2])>>8);
    dst[3] = (((int)src.a*((int)src.b)* dst[3]) >> 16) + (((255 - src.a)*dst[3])>>8);
    dst_color.r = (unsigned char)(dst[1]);
    dst_color.g = (unsigned char)(dst[2]);
    dst_color.b = (unsigned char)(dst[3]);
    return;
}

template<>
inline void write_pixel<GEN16X_BLENDMODE_SUBTRACT>(const gen16x_color32 &src, gen16x_color32 &dst_color) {
    int dst[4];
    dst[1] = dst_color.r;
    dst[2] = dst_color.g;
    dst[3] = dst_color.b;
    dst[1] = dst[1] - (((int)src.r*src.a)>>8);
    dst[2] = dst[2] - (((int)src.g*src.a)>>8);
    dst[3] = dst[3] - (((int)src.b*src.a)>>8);
    dst[1] = dst[1] < 0 ? 0 : dst[1];
    dst[2] = dst[2] < 0 ? 0 : dst[2];
    dst[3] = dst[3] < 0 ? 0 : dst[3];
    dst_color.r = (unsigned char)dst[1];
    dst_color.g = (unsigned char)dst[2];
    dst_color.b = (unsigned char)dst[3];
    return;
}
  

template<int blendmode>
void render_row_direct(gen16x_ppu* ppu, int layer_index, int row_index, int col_start, int col_end, unsigned int* row_pixels) {
    auto &layer = ppu->layers[layer_index];
    gen16x_layer_direct * layer_direct = (gen16x_layer_direct*)(ppu->vram + layer.vram_offset);
    
    bool rep_x = (bool)(layer.direct_layer.flags & GEN16X_FLAG_REPEAT_X);
    bool rep_y = (bool)(layer.direct_layer.flags & GEN16X_FLAG_REPEAT_Y);
    int y0 = row_index + layer.direct_layer.scroll_y;
    bool oob_y = (y0 < 0 || y0 >= layer.direct_layer.height);

    if (oob_y) {
        if (rep_y) {
            y0 = y0 % layer.direct_layer.width;
        } else {
            return;
        }
    }

    unsigned char* row_src = &layer_direct->map[y0*layer.direct_layer.width];

    if (!rep_x && -layer.direct_layer.scroll_x > col_start) {
        col_start = -layer.direct_layer.scroll_x;
    }
    if (!rep_x && layer.direct_layer.width - layer.direct_layer.scroll_x < col_end) {
        col_end = layer.direct_layer.width - layer.direct_layer.scroll_x;
    }
    int scroll_wrap = -layer.direct_layer.scroll_x;
    if (rep_x) {
        scroll_wrap = (scroll_wrap % layer.direct_layer.width);
    }
    
    for (int x = col_start; x < col_end; ++x) {
        int x0 = x - scroll_wrap;
        
        bool oob_x = (x0 < 0 || x0 >= layer.direct_layer.width);
        if (oob_x) {
            if (rep_x) {
                int x_sign = (int)(x0 > 0) - (int)(x0 < 0);
                x0 = x0 - x_sign* layer.direct_layer.width;
            } else {
                continue;
            }
        }
        write_pixel<blendmode>(ppu->cgram32[row_src[x0]], *((gen16x_color32*)&row_pixels[x]));
    }
}

template<unsigned char tile_size_shift, int blendmode>
void render_row_tiles(gen16x_ppu* ppu, int layer_index, int row_index, int col_start, int col_end, unsigned int* row_pixels) {
    auto &layer = ppu->layers[layer_index];
    gen16x_layer_tiles * layer_tiles = (gen16x_layer_tiles*)(ppu->vram + layer.vram_offset);

    const unsigned char* tile_map = layer_tiles->tile_map;
    int y = row_index;
    
    const unsigned char tile_size = (1 << tile_size_shift);
    const unsigned char tile_index_shift = 4 + tile_size_shift;
    const unsigned char tile_size_mask = tile_size - 1;

    unsigned char tilemap_width_shift = layer.tile_layer.tilemap_width;
    unsigned char tilemap_wh[2] = {
        (unsigned char)(1 << tilemap_width_shift),
        (unsigned char)(1 << layer.tile_layer.tilemap_height)
    };
    
    int tilemap_wh_mask[2] = {
        tilemap_wh[0] - 1,
        tilemap_wh[1] - 1
    };

    unsigned int tilemap_wh_mask_inv[2] = {
        ~((unsigned int)tilemap_wh_mask[0]),
        ~((unsigned int)tilemap_wh_mask[1])
    };

    bool clamp[2] = {
        (bool)(layer.tile_layer.flags & GEN16X_FLAG_CLAMP_X),
        (bool)(layer.tile_layer.flags & GEN16X_FLAG_CLAMP_Y)
    };
    bool rep[2] = {
                (bool)(layer.tile_layer.flags & GEN16X_FLAG_REPEAT_X),
                (bool)(layer.tile_layer.flags & GEN16X_FLAG_REPEAT_Y)
    };

    bool not_clamp_or_rep[2] = {
        !clamp[0] && !rep[0],
        !clamp[1] && !rep[1],
    };

    bool apply_tf = (bool)(layer.tile_layer.flags & GEN16X_FLAG_TRANSFORM);

    gen16x_transform tf = layer.tile_layer.transform;

    for (int x = col_start; x < col_end; ++x) {

        int xy[2] = { x,y };
        int xy0[2];

        if (apply_tf) {
            for (int i = 0; i < 2; i++) {
                xy0[i]  = ((((tf.m[2+i][0]*(xy[0] + tf.m[0][0] - tf.m[1][0]))
                           + (tf.m[2+i][1]*(xy[1] + tf.m[0][1] - tf.m[1][1]))
                    ) >> tf.base) + tf.m[1][i]);
            }
        } else {
            for (int i = 0; i < 2; i++) {
                xy0[i] = xy[i] + tf.m[0][i];
            }
        }

        int tile[2] = { (xy0[0] >> tile_size_shift), (xy0[1] >> tile_size_shift) };

        bool tile_oob[2] = {
            (bool)((unsigned int)tile[0] & tilemap_wh_mask_inv[0]),
            (bool)((unsigned int)tile[1] & tilemap_wh_mask_inv[1])
        };
        
        bool discard = (tile_oob[0] && not_clamp_or_rep[0]) 
                    || (tile_oob[1] && not_clamp_or_rep[1]);
        
        if (discard) {
            continue;
        }

        if (tile_oob[0] && clamp[0]) {
            tile[0] = clamp0(tile[0], tilemap_wh_mask[0]);
        }  else {
            tile[0] = tile[0] & tilemap_wh_mask[0];
        }

        if (tile_oob[1] && clamp[1]) {
            tile[1] = clamp0(tile[1], tilemap_wh_mask[1]);
        } else {
            tile[1] = tile[1] & tilemap_wh_mask[1];
        }
        unsigned int tile_offset = (tile[1] << tilemap_width_shift) + tile[0];
        unsigned int tile_index = tile_map[tile_offset];

        unsigned char tile_sub[2] = {
            (unsigned char)((unsigned char)xy0[0] & tile_size_mask),
            (unsigned char)((unsigned char)xy0[1] & tile_size_mask)
        };

        unsigned int tile_pixel_offset = (tile_index << tile_index_shift) | (tile_sub[1] << tile_size_shift) | (tile_sub[0]);
        unsigned char tile_pixel = layer_tiles->tile_palette[tile_pixel_offset];
        write_pixel<blendmode>(ppu->cgram32[tile_pixel], *((gen16x_color32*)&row_pixels[x]));
    }
}

template<int blendmode>
void render_row_sprites(gen16x_ppu* ppu, int layer_index, int row_index, int col_start, int col_end, unsigned int* row_pixels) {
    auto &layer = ppu->layers[layer_index];
    gen16x_layer_sprites* layer_sprites = (gen16x_layer_sprites*)(ppu->vram + layer.vram_offset);

   
    unsigned char sprites_to_draw[GEN16X_MAX_SPRITES_PER_ROW];

    int num_sprites = 0;

    for (int s = 0; s < GEN16X_MAX_SPRITES; s++) {
        gen16x_sprite& sprite = layer.sprite_layer.sprites[s];
        
        if (!(sprite.flags & GEN16X_FLAG_SPRITE_ENABLED)) {
            continue;
        }
        
        int s_y = sprite.y;
        int s_sw = (sprite.size & GEN16X_SPRITE_WIDTH_MASK);
        int s_sh = ((sprite.size & GEN16X_SPRITE_HEIGHT_MASK) >> 4);
        int s_w = 1 << s_sw;
        int s_h = 1 << s_sh;
        
        if (out_of_range(row_index, s_y, s_y + s_h - 1)) {
            continue;
        }
        int s_x = sprite.x;
        if ((s_x >= ppu->screen_width) || (s_x + s_w < 0)) {
            continue;
        }
        if (num_sprites >= GEN16X_MAX_SPRITES_PER_ROW) {
            break;
        }
        sprites_to_draw[num_sprites++] = (char)s;


    }

    for (int c = 0; c < num_sprites; c++) {
        int sprite_index = sprites_to_draw[c];
        gen16x_sprite& sprite = layer.sprite_layer.sprites[sprite_index];

        int s_sw = (sprite.size & GEN16X_SPRITE_WIDTH_MASK);
        int s_sh = ((sprite.size & GEN16X_SPRITE_HEIGHT_MASK) >> 4);
        int s_w = 1 << s_sw;
        int s_h = 1 << s_sh;


        int start = sprite.x;
        if (start < col_start) {
            start = col_start + int(sprite.x % 2);
        }
        int end = sprite.x + s_w;
        if (end > col_end) {
            end = col_end;
        }
        int y0 = clamp0(row_index - sprite.y, s_h);
        int s_sub_w_mask = ((1 << (8 - s_sw)) - 1);
        int s_sub_h_mask = ((1 << (8 - s_sh)) - 1);
        int s_sub_y = (y0 >> 3) & s_sub_h_mask;
        int s_sub_y0 = y0 & 7;
        
        for (int x = start; x < end; x += 2) {
            int x0 = (x - sprite.x);
            int s_sub_x = (x0 >> 3) & s_sub_w_mask;
            int s_sub_x0 = (x0 & 0x7);
            int s_sub_index = ((s_sub_y << (s_sw - 3)) + s_sub_x);
            int s_offset = (sprite.tile_index << 6) + (s_sub_index << 6) + (s_sub_y0 << 3) + (s_sub_x0);

            unsigned int sprite_color = (int)layer_sprites->sprite_palette[(s_offset >> 1)&0x1FFFF];
            unsigned int sprite_color0 = sprite_color >> 4;
            unsigned int sprite_color1 = sprite_color & 0xF;
            if (x >= col_start) {
                write_pixel<blendmode>(ppu->cgram32[sprite.palette_offset + sprite_color0], *((gen16x_color32*)&row_pixels[x]));
            }
            if (x + 1 < col_end) {
                write_pixel<blendmode>(ppu->cgram32[sprite.palette_offset + sprite_color1], *((gen16x_color32*)&row_pixels[x + 1]));
            }
        }
    }
}
void gen16x_ppu_render(gen16x_ppu* ppu) {
    for (int y = 0; y < ppu->screen_height; ++y) {
        if (ppu->row_callback) {
            ppu->row_callback(ppu, y);
        }
        
        unsigned int* row_pixels = (unsigned int*)(ppu->vram + ppu->framebuffer_offset) + y*ppu->screen_width;

        memset(row_pixels, 0, ppu->screen_width*4);
        
        for (int l = 0; l < 6; l++) {
            
            #define CASE_BLENDMODE_DIRECT(mode) case (mode):\
                render_row_direct<mode>(ppu, l, y, 0, ppu->screen_width, row_pixels); break

            #define CASE_BLENDMODE_TILES8(mode) case (mode):\
                render_row_tiles<3, mode>(ppu, l, y, 0, ppu->screen_width, row_pixels); break

            #define CASE_BLENDMODE_TILES16(mode) case (mode):\
                render_row_tiles<4, mode>(ppu, l, y, 0, ppu->screen_width, row_pixels); break

            #define CASE_BLENDMODE_SPRITES(mode) case (mode):\
                render_row_sprites<mode>(ppu, l, y, 0, ppu->screen_width, row_pixels); break

            
            #define CASE_ALL_BLENDMODES(type) \
                CASE_BLENDMODE_##type(GEN16X_BLENDMODE_NONE);\
                CASE_BLENDMODE_##type(GEN16X_BLENDMODE_ALPHA);\
                CASE_BLENDMODE_##type(GEN16X_BLENDMODE_ADD);\
                CASE_BLENDMODE_##type(GEN16X_BLENDMODE_SUBTRACT);\
                CASE_BLENDMODE_##type(GEN16X_BLENDMODE_MULTIPLY);

            #define ALL_BLENDMODE_SWITCH(type) switch (ppu->layers[l].blend_mode) {CASE_ALL_BLENDMODES(type)}

            switch (ppu->layers[l].layer_type) {
            case GEN16X_LAYER_DIRECT:
                ALL_BLENDMODE_SWITCH(DIRECT);
                break;
            case GEN16X_LAYER_TILES:
            {
                if (ppu->layers[l].tile_layer.tile_size == GEN16X_TILE8) {
                    ALL_BLENDMODE_SWITCH(TILES8);
                } else if (ppu->layers[l].tile_layer.tile_size == GEN16X_TILE16) {
                    ALL_BLENDMODE_SWITCH(TILES16)
                }
                break;
            }
            case GEN16X_LAYER_SPRITES:
                ALL_BLENDMODE_SWITCH(SPRITES)
                break;
            }
        }
    }
}





static const unsigned char osc_wave[4][256] = {
    {128,131,134,137,140,143,146,149,152,156,159,162,165,168,171,174,
     176,179,182,185,188,191,193,196,199,201,204,206,209,211,213,216,
     218,220,222,224,226,228,230,232,234,236,237,239,240,242,243,245,
     246,247,248,249,250,251,252,252,253,254,254,255,255,255,255,255,
     255,255,255,255,255,255,254,254,253,252,252,251,250,249,248,247,
     246,245,243,242,240,239,237,236,234,232,230,228,226,224,222,220,
     218,216,213,211,209,206,204,201,199,196,193,191,188,185,182,179,
     176,174,171,168,165,162,159,156,152,149,146,143,140,137,134,131,
     128,124,121,118,115,112,109,106,103,99, 96, 93, 90, 87, 84, 81,
     79, 76, 73, 70, 67, 64, 62, 59, 56, 54, 51, 49, 46, 44, 42, 39,
     37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 18, 16, 15, 13, 12, 10,
     9, 8, 7, 6, 5, 4, 3, 3, 2, 1, 1, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3, 4, 5, 6, 7, 8,
     9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25, 27, 29, 31, 33, 35,
     37, 39, 42, 44, 46, 49, 51, 54, 56, 59, 62, 64, 67, 70, 73, 76,
     79, 81, 84, 87, 90, 93, 96, 99, 103,106,109,112,115,118,121,124},
    {128, 130, 132, 134, 136, 138, 140, 142, 144, 146, 148, 150, 152, 154, 156, 158,
     160, 162, 164, 166, 168, 170, 172, 174, 176, 178, 180, 182, 184, 186, 188, 190,
     192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214, 216, 218, 220, 222, 
     224, 226, 228, 230, 232, 234, 236, 238, 240, 242, 244, 246, 248, 250, 252, 254,
     255, 253, 251, 249, 247, 245, 243, 241, 239, 237, 235, 233, 231, 229, 227, 225,
     223, 221, 219, 217, 215, 213, 211, 209, 207, 205, 203, 201, 199, 197, 195, 193,
     191, 189, 187, 185, 183, 181, 179, 177, 175, 173, 171, 169, 167, 165, 163, 161,
     159, 157, 155, 153, 151, 149, 147, 145, 143, 141, 139, 137, 135, 133, 131, 129,
     127, 125, 123, 121, 119, 117, 115, 113, 111, 109, 107, 105, 103, 101,  99,  97, 
      95,  93,  91,  89,  87,  85,  83,  81,  79,  77,  75,  73,  71,  69,  67,  65,
      63,  61,  59,  57,  55,  53,  51,  49,  47,  45,  43,  41,  39,  37,  35,  33,
      31,  29,  27,  25,  23,  21,  19,  17,  15,  13,  11,  9,    7,   5,   3,   1,
       0,   2,   4,   6,   8,  10,  12,  14,  16,  18,  20, 22,   24,  26,  28,  30,
      32,  34,  36,  38,  40,  42,  44,  46,  48,  50,  52, 54,   56,  58,  60,  62,
      64,  66,  68,  70,  72,  74,  76,  78,  80,  82,  84, 86,   88,  90,  92,  94,
      96,  98, 100, 102, 104, 106, 108, 110, 112, 114, 116, 118, 120, 122, 124, 126},
    {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
     255, 239, 223, 207, 191, 175, 159, 143, 127, 111, 95, 79, 63, 47, 31, 15,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240}, 
    {225, 203, 49, 189, 51, 58, 217, 10, 159, 92, 76, 36, 129, 81, 195, 135, 157, 24, 
    188, 156, 249, 233, 78, 199, 142, 48, 16, 105, 71, 97, 186, 151, 151, 253, 4, 24,
    162, 232, 254, 67, 173, 91, 248, 155, 94, 225, 201, 144, 43, 19, 28, 55, 120, 58,
    226, 193, 160, 5, 133, 178, 188, 96, 72, 38, 101, 37, 15, 204, 154, 221, 65, 83,
    215, 223, 209, 162, 96, 168, 132, 35, 203, 138, 135, 165, 237, 174, 173, 211, 103,
    226, 42, 219, 48, 106, 159, 15, 165, 144, 193, 62, 140, 73, 72, 86, 75, 71, 12, 82,
    170, 240, 8, 198, 182, 11, 89, 42, 4, 109, 103, 126, 153, 246, 160, 25, 33, 146, 45,
    194, 242, 251, 114, 51, 21, 178, 75, 135, 29, 103, 211, 110, 182, 198, 213, 6, 185,
    160, 195, 48, 163, 145, 70, 170, 219, 186, 141, 156, 238, 225, 208, 64, 164, 252, 34,
    205, 206, 138, 104, 159, 190, 71, 118, 76, 3, 117, 218, 122, 203, 52, 56, 200, 191,
    254, 173, 25, 157, 176, 21, 37, 113, 229, 161, 206, 8, 213, 216, 189, 204, 49, 212,
    27, 118, 124, 75, 78, 12, 13, 63, 199, 48, 67, 189, 40, 215, 89, 96, 1, 175, 232,
    65, 167, 121, 195, 44, 154, 161, 39, 183, 75, 91, 22, 16, 203, 147, 39, 234, 194, 58, 234, 
    158, 54, 88, 209, 203, 38, 226, 121, 157, 54, 242, 180, 119, 170, 14, 41, 232, 54,}};



inline int fast_abs(int x) {
    return (x >= 0) ? x : -x;
}

inline int mod_wrap(int a, int b) {
    return (b + (a%b)) % b;
}
int midi_note_to_freq(int x, int a) {
    // converts a midi note x tuned to frequency A into a frequency integer
    //int a = 440;
    int b = 69;
    int c = 16;
    int d = 12;
    
    int hd = d/2;
    int k = 64;

    int x0 = x;
    if (x - b < 0) {
        x0 -= d;
    }

    int h = (x0-b)/d;
    
    int g = h >= 0 ? a*(1<<h) : (a/(1<<(-h)));
    int f = ((g*((d*(x-b))/d))/(d)) - g*(h-1);
    int p = hd - fast_abs(hd - mod_wrap(x-b,d));
    int q = (k - ((k*p)/hd));
    int u = k*(k - ((q*q)/k));
    return f -(f*u)/(c*k*k); //f-fuckk!!
}

static int midi_table_440x32[128] = {
    262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 
    698, 740, 784, 831, 880, 932, 988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 
    1661, 1760, 1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520,
    3729, 3951, 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902, 
    8372, 8870, 9397, 9956, 10548, 11175, 11840, 12544, 13290, 14080, 14917, 15804, 16744, 
    17740, 18795, 19912, 21096, 22351, 23680, 25088, 26580, 28160, 29834, 31609, 33488, 
    35479, 37589, 39824, 42192, 44701, 47359, 50175, 53159, 56320, 59669, 63217, 66976, 
    70959, 75178, 79649, 84385, 89402, 94719, 100351, 106318, 112640, 119338, 126434, 
    133952, 141918, 150356, 159297, 168769, 178805, 189437, 200702, 212636, 225280, 
    238676, 252868, 267905, 283835, 300713, 318594, 337539, 357610, 378874, 401403,
};

int midi_note_to_freq_table(int x) {
    return midi_table_440x32[x&0x7F];
}

void gen16x_spu_tick(gen16x_spu* spu) {
    
    int r_acc_signal = 0;
    int l_acc_signal = 0;
    short* output =  (spu->sram + spu->output_offset);
    if (spu->flushed) {
        spu->current_output_position = 0;
        spu->flushed = 0;
    }
    
    for (int i = 0; i < GEN16X_MAX_DSP_CHANNELS; i++) {
        gen16x_dsp_channel &dsp = spu->channels[i];
        
        if (!dsp.enabled) {
            continue;
        }
        short* voice = (spu->sram + dsp.voice_offset);
        
        
        int voice_time = int(dsp.time)/GEN16X_DSP_BASE_PITCH;
        
        if (dsp.voice_stop) {
            if (dsp.voice_playing) {
                dsp.voice_playing = 0;
            }
            dsp.voice_stop = 0;
        }
        if (dsp.voice_playing && (voice_time + 1) >= dsp.voice_samples/2) {
            if (dsp.voice_loop) {
                voice_time %= dsp.voice_samples/2;
            } else {
                dsp.voice_playing = 0;
                voice_time = 0;
            }
            
        }
        if ((spu->time_counter & 0x1) == 0) {
            switch (dsp.gain_type) {
                case GEN16X_DSP_GAIN_DIRECT:
                    dsp.current_gain_value = dsp.gain_target;
                    break;
                case GEN16X_DSP_GAIN_LINEAR:
                {
                    int dir = (dsp.gain_target - dsp.current_gain_value);
                    
                    dir = (dir < 0) ? -1 : ((dir > 0) ? 1 : 0);
                    
                    int new_gain_value = dir*(int)dsp.gain_rate + (int)dsp.current_gain_value;
                    
                    if ((dir < 0 && new_gain_value < dsp.gain_target)
                     || (dir > 0 && new_gain_value > dsp.gain_target)) {
                        dsp.current_gain_value = dsp.gain_target;
                    } else {
                        dsp.current_gain_value = new_gain_value;
                    }
                    break;
                }
                case GEN16X_DSP_GAIN_EXPONENTIAL:
                {
                    dsp.current_gain_value = (((int)dsp.gain_target*dsp.gain_rate) + ((int)dsp.current_gain_value*(GEN16X_DSP_MAX_VOLUME - (int)dsp.gain_rate)))/(GEN16X_DSP_MAX_VOLUME);
                    break;
                }
            }
        }

        
        //if (dsp.voice_playing) {

        int lerp_value = (dsp.time % GEN16X_DSP_BASE_PITCH);

        int l_input_signal0 = 0;
        int l_input_signal1 = 0;
        int r_input_signal0 = 0;
        int r_input_signal1 = 0;

        if (dsp.voice_playing) {
            l_input_signal0 = voice[(voice_time*2)];
            l_input_signal1 = voice[((voice_time+1)* 2)];
            r_input_signal0 = voice[((voice_time)* 2) + 1];
            r_input_signal1 = voice[((voice_time+1)* 2) + 1];
        }
        
        
        
        int l_input_signal = (l_input_signal0*(GEN16X_DSP_BASE_PITCH - lerp_value) + l_input_signal1*(lerp_value))/GEN16X_DSP_BASE_PITCH;

        
        int r_input_signal = (r_input_signal0*(GEN16X_DSP_BASE_PITCH - lerp_value) + r_input_signal1*(lerp_value))/GEN16X_DSP_BASE_PITCH;



        if (dsp.oscillator_type != GEN16X_DSP_OSC_NONE) {

            

            


            int osc_value = 0;
            unsigned int fixed_time = dsp.time/GEN16X_DSP_BASE_PITCH;

            unsigned int fixed_time_sub = (dsp.time%GEN16X_DSP_BASE_PITCH);

            int osc_time0 = (midi_note_to_freq_table(dsp.oscillator_note)*fixed_time)/(spu->sample_rate/8);

            int osc_value0 = 0;

            osc_value0 = (((int)osc_wave[(dsp.oscillator_type - 1)&0x3][(osc_time0 + dsp.oscillator_phase) & 0xFF]) - 127)*256;
            
            //= (((int)osc_wave[(dsp.oscillator_type - 1)&0x3][(osc_time0 + dsp.oscillator_phase) & 0xFF]) - 127)*128;
            
            osc_value = osc_value0;


            l_input_signal += osc_value*dsp.oscillator_amplitude/(GEN16X_DSP_MAX_VOLUME);
            r_input_signal += osc_value*dsp.oscillator_amplitude/(GEN16X_DSP_MAX_VOLUME);
        }

        l_input_signal = l_input_signal*dsp.l_volume/GEN16X_DSP_MAX_VOLUME;
        r_input_signal = r_input_signal*dsp.r_volume/GEN16X_DSP_MAX_VOLUME;

        l_acc_signal += ((l_input_signal)*((int)dsp.current_gain_value))/GEN16X_DSP_MAX_VOLUME;// ((int)dsp.l_volume*l_input_signal*(int)dsp.current_gain_value)/(256*1024);
        r_acc_signal += ((r_input_signal)*((int)dsp.current_gain_value))/GEN16X_DSP_MAX_VOLUME;// ((int)dsp.r_volume*r_input_signal*(int)dsp.current_gain_value)/(256*1024);

        dsp.time += (GEN16X_DSP_BASE_PITCH + dsp.pitch);

        //}
        
    }
    int output_pos = spu->current_output_position++;
    if (output_pos >= spu->output_samples/2) {
        output_pos = (spu->output_samples/2) - 1;
        spu->underrun = 1;
    } else {
        spu->underrun = 0;
    }
    int l_result = (l_acc_signal*spu->l_volume)/(GEN16X_DSP_MAX_VOLUME);
    int r_result = (r_acc_signal*spu->r_volume)/(GEN16X_DSP_MAX_VOLUME);
    if (l_result <= -(65534/2)) {
        l_result = -(65534/2);
    }
    if (l_result >= (65534/2)) {
        l_result = (65534/2);
    }
    if (r_result <= -(65534/2)) {
        r_result = -(65534/2);
    }
    if (r_result >= (65534/2)) {
        r_result = (65534/2);
    }
    output[(output_pos*2)] = (l_result);
    output[(output_pos*2) + 1] = (r_result);
    spu->time_counter++;
}

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
#include "gen16x_ppu.h"

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
    unsigned char* map = ppu->vram + layer.vram_offset;
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

    unsigned char* row_src = map + y0*layer.direct_layer.width;

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

template<unsigned char tile_size_shift, bool apply_tf, int blendmode, int edge_mode_x, int edge_mode_y>
void render_row_tiles(gen16x_ppu* ppu, int layer_index, int row_index, int col_start, int col_end, unsigned int* row_pixels) {
    auto &layer = ppu->layers[layer_index];

    const unsigned char* tile_map = ppu->vram + layer.tile_layer.tilemap_vram_offset;
    const unsigned char* tile_set = ppu->vram + layer.vram_offset;

    int y = row_index;
    
    const unsigned char tile_size = (1 << tile_size_shift);
    const unsigned char tile_index_shift = 2*tile_size_shift;
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

    const bool clamp[2] = {
        edge_mode_x == GEN16X_FLAG_CLAMP_X,
        edge_mode_y == GEN16X_FLAG_CLAMP_Y,
    };
    const bool rep[2] = {
        edge_mode_x == GEN16X_FLAG_REPEAT_X,
        edge_mode_y == GEN16X_FLAG_REPEAT_Y,
    };

    const bool not_clamp_or_rep[2] = {
        !clamp[0] && !rep[0],
        !clamp[1] && !rep[1],
    };

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
        unsigned char tile_pixel = tile_set[tile_pixel_offset];
        write_pixel<blendmode>(ppu->cgram32[tile_pixel], *((gen16x_color32*)&row_pixels[x]));
    }
}

template<int blendmode>
void render_row_sprites(gen16x_ppu* ppu, int layer_index, int row_index, int col_start, int col_end, unsigned int* row_pixels) {
    auto &layer = ppu->layers[layer_index];

    unsigned char* sprite_tiles = ppu->vram + layer.vram_offset;
   
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
        int s_w = ((1 << s_sw));
        int s_h = ((1 << s_sh));

       
        if (out_of_range((((row_index - s_y)*sprite.scale_y) >> 8) + s_y, s_y, s_y + s_h - 1)) {
            continue;
        }
        int s_x = sprite.x;
        if ((s_x >= (((ppu->screen_width - s_x)*sprite.scale_x) >> 8) + s_x)
            || (s_x + s_w < (((0 - s_x)*sprite.scale_x) >> 8) + s_x)) {
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
        int s_w = (1 << s_sw);
        int s_h = (1 << s_sh);


        int start = sprite.x;
        if (start < col_start) {
            start = col_start + int(sprite.x % 2);
        }
        int end = sprite.x + (((s_w) << 8)/sprite.scale_x);
        if (end > col_end) {
            end = col_end;
        }
        int y0 = clamp0(((row_index - sprite.y)*sprite.scale_y) >> 8, s_h);
        int sub_w_mask = ((1 << (8 - s_sw)) - 1);
        int sub_h_mask = ((1 << (8 - s_sh)) - 1);
        int sub_y = (y0 >> 3) & sub_h_mask;
        int sub_y0 = y0 & 7;
        int block_region = (sprite.tile_index << 5);
        int block_sub_region = (sub_y << (s_sw - 3));
        int prev_x0 = -32768;
        gen16x_color32 prev_color;
        for (int x = start; x < end; x++) {

            int x0 = (((x - sprite.x))*sprite.scale_x) >> 8;

            if (x0 != prev_x0) {
                int sub_x = (x0 >> 3) & sub_w_mask;
                int sub_x0 = (x0 & 0x7) >> 1;
                int sub_index = (block_sub_region + sub_x);
                int offset = block_region + (sub_index << 5) + (sub_y0 << 2) + (sub_x0);

                unsigned int sprite_color0 = (int)sprite_tiles[(offset)&0x1FFFF];

                if ((x0 & 1) == 0) {
                    sprite_color0 = sprite_color0 >> 4;
                } else {
                    sprite_color0 = (sprite_color0 & 0xF);
                }
                prev_color = ppu->cgram32[sprite.palette_offset + sprite_color0];
                prev_x0 = x0;
            }
            if (x >= col_start) {
                write_pixel<blendmode>(prev_color, *((gen16x_color32*)&row_pixels[x]));
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

            #define CASE_TILES_IF(tile_mode, blendmode, edge_mode_x, edge_mode_y)\
                if ((ppu->layers[l].tile_layer.flags & edge_mode_x) == edge_mode_x \
                 && (ppu->layers[l].tile_layer.flags & edge_mode_y) == edge_mode_y) {\
                    if ((ppu->layers[l].tile_layer.flags & GEN16X_FLAG_TRANSFORM) == GEN16X_FLAG_TRANSFORM)\
                        render_row_tiles<tile_mode, true, blendmode, edge_mode_x, edge_mode_y>(ppu, l, y, 0, ppu->screen_width, row_pixels);\
                    else render_row_tiles<tile_mode, false, blendmode, edge_mode_x, edge_mode_y>(ppu, l, y, 0, ppu->screen_width, row_pixels);\
                }
            #define CASE_TILES(tile, mode) \
                CASE_TILES_IF(tile, mode, GEN16X_FLAG_CLAMP_X , GEN16X_FLAG_CLAMP_Y  ) else \
                CASE_TILES_IF(tile, mode, GEN16X_FLAG_CLAMP_X , GEN16X_FLAG_REPEAT_Y ) else \
                CASE_TILES_IF(tile, mode, GEN16X_FLAG_CLAMP_X , 0                    ) else \
                CASE_TILES_IF(tile, mode, GEN16X_FLAG_REPEAT_X, GEN16X_FLAG_CLAMP_Y  ) else \
                CASE_TILES_IF(tile, mode, GEN16X_FLAG_REPEAT_X, GEN16X_FLAG_REPEAT_Y ) else \
                CASE_TILES_IF(tile, mode, GEN16X_FLAG_REPEAT_X, 0                    ) else \
                CASE_TILES_IF(tile, mode, 0                   , GEN16X_FLAG_CLAMP_Y  ) else \
                CASE_TILES_IF(tile, mode, 0                   , GEN16X_FLAG_REPEAT_Y ) else \
                CASE_TILES_IF(tile, mode, 0                   , 0)

            #define CASE_BLENDMODE_DIRECT(mode) case (mode):\
                render_row_direct<mode>(ppu, l, y, 0, ppu->screen_width, row_pixels); break

            #define CASE_BLENDMODE_TILES8(mode) case (mode):\
                CASE_TILES(3, mode) \
                break;

            #define CASE_BLENDMODE_TILES16(mode) case (mode):\
                CASE_TILES(4, mode) \
                break;

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
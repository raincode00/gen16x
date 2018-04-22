#include "gen16x.h"

inline int clamp0(int x, int b) {
    x = x < 0 ? 0 : (x > b ? b : x);
    return x;
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

void blend_pixel(const int &blendmode, const gen16x_color32 &src, gen16x_color32 &dst_color) {
    int dst[4];
    dst[1] = dst_color.r;
    dst[2] = dst_color.g;
    dst[3] = dst_color.b;
    switch (blendmode) {
    case GEN16X_BLENDMODE_ALPHA:
        dst[1] = (((int)src.r*src.a) + ((int)dst[1]*(255 - src.a)))/255;
        dst[2] = (((int)src.g*src.a) + ((int)dst[2]*(255 - src.a)))/255;
        dst[3] = (((int)src.b*src.a) + ((int)dst[3]*(255 - src.a)))/255;
        break;
    case GEN16X_BLENDMODE_ADD:
        dst[1] = (int)src.r + dst[1];
        dst[2] = (int)src.g + dst[2];
        dst[3] = (int)src.b + dst[3];

        dst[1] = dst[1] > 255 ? 255 : dst[1];
        dst[2] = dst[2] > 255 ? 255 : dst[2];
        dst[3] = dst[3] > 255 ? 255 : dst[3];
        break;
    case GEN16X_BLENDMODE_MULTIPLY:
        dst[1] = (int)src.r * dst[1] / 255;
        dst[2] = (int)src.g * dst[2] / 255;
        dst[3] = (int)src.b * dst[3] / 255;
        break;
    case GEN16X_BLENDMODE_SUBTRACT:
        dst[1] = dst[1] - (int)src.r;
        dst[2] = dst[2] - (int)src.g;
        dst[3] = dst[3] - (int)src.b;

        dst[1] = dst[1] < 0 ? 0 : dst[1];
        dst[2] = dst[2] < 0 ? 0 : dst[2];
        dst[3] = dst[3] < 0 ? 0 : dst[3];
        break;
    }
    dst_color.r = (unsigned char)dst[1];
    dst_color.g = (unsigned char)dst[2];
    dst_color.b = (unsigned char)dst[3];
}

inline void write_pixel(const int &blendmode, const gen16x_color32 &src_color, gen16x_color32 &dst_color) {
    if (blendmode == GEN16X_BLENDMODE_NONE) {
        int alpha = (src_color.color_i >> 31);
        dst_color = alpha ? src_color : dst_color;
    } else {
        blend_pixel(blendmode, src_color, dst_color);
    }
}

void render_row_direct(gen16x_ppu_state* ppu, int layer_index, int row_index, int col_start, int col_end, unsigned int* row_pixels) {
    auto &layer = ppu->layers[layer_index];
    gen16x_ppu_layer_direct * layer_direct = (gen16x_ppu_layer_direct*)(ppu->vram + layer.vram_offset);
    
    int blendmode = layer.blend_mode;

    bool rep_x = (bool)(layer_direct->flags & GEN16X_FLAG_REPEAT_X);
    bool rep_y = (bool)(layer_direct->flags & GEN16X_FLAG_REPEAT_Y);
    int y0 = row_index + layer_direct->scroll_y;
    bool oob_y = (y0 < 0 || y0 >= layer_direct->height);

    if (oob_y) {
        if (rep_y) {
            y0 = y0 % layer_direct->width;
        } else {
            return;
        }
    }

    unsigned char* row_src = &layer_direct->map[y0*layer_direct->width];

    if (!rep_x && -layer_direct->scroll_x > col_start) {
        col_start = -layer_direct->scroll_x;
    }
    if (!rep_x && layer_direct->width - layer_direct->scroll_x < col_end) {
        col_end = layer_direct->width - layer_direct->scroll_x;
    }
    int scroll_wrap = -layer_direct->scroll_x;
    if (rep_x) {
        scroll_wrap = (scroll_wrap % layer_direct->width);
    }
    
    for (int x = col_start; x < col_end; ++x) {
        int x0 = x - scroll_wrap;
        
        bool oob_x = (x0 < 0 || x0 >= layer_direct->width);
        if (oob_x) {
            if (rep_x) {
                int x_sign = (int)(x0 > 0) - (int)(x0 < 0);
                x0 = x0 - x_sign*layer_direct->width;
            } else {
                continue;
            }
        }
        write_pixel(blendmode, ppu->cgram32[row_src[x0]], *((gen16x_color32*)&row_pixels[x]));
    }
}

template<int tile_size_shift>
void render_row_tiles(gen16x_ppu_state* ppu, int layer_index, int row_index, int col_start, int col_end, unsigned int* row_pixels) {
    auto &layer = ppu->layers[layer_index];
    gen16x_ppu_layer_tiles * layer_tiles = (gen16x_ppu_layer_tiles*)(ppu->vram + layer.vram_offset);
    
    unsigned char blendmode = layer.blend_mode;
    const unsigned char* tile_map = layer_tiles->tile_map;
    const unsigned char* tile_palette = layer_tiles->tile_palette;
    int y = row_index;
    
    const int tile_size = (1 << tile_size_shift);
    const int tile_index_shift = 4 + tile_size_shift;
    const int tile_size_mask = tile_size - 1;

    int tilemap_width_shift = layer_tiles->tilemap_width;
    int tilemap_wh[2] = {
        1 << tilemap_width_shift,
        1 << layer_tiles->tilemap_height
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
        (bool)(layer_tiles->flags & GEN16X_FLAG_CLAMP_X),
        (bool)(layer_tiles->flags & GEN16X_FLAG_CLAMP_Y)
    };
    bool rep[2] = {
                (bool)(layer_tiles->flags & GEN16X_FLAG_REPEAT_X),
                (bool)(layer_tiles->flags & GEN16X_FLAG_REPEAT_Y)
    };

    bool not_clamp_or_rep[2] = {
        !clamp[0] && !rep[0],
        !clamp[1] && !rep[1],
    };

    bool apply_tf = (bool)(layer_tiles->flags & GEN16X_FLAG_TRANSFORM);

    gen16x_ppu_transform tf = layer_tiles->transform;

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

        if (tile_index == 0) {
            continue;
        }

        unsigned int tile_sub[2] = {
            ((unsigned int)xy0[0] & (unsigned int)tile_size_mask),
            ((unsigned int)xy0[1] & (unsigned int)tile_size_mask)
        };

        unsigned int tile_pixel_offset = (tile_index << tile_index_shift) | (tile_sub[1] << tile_size_shift) | (tile_sub[0]);
        unsigned char tile_pixel = tile_palette[tile_pixel_offset];
        write_pixel(blendmode, ppu->cgram32[tile_pixel], *((gen16x_color32*)&row_pixels[x]));
    }
}

void render_row_sprites(gen16x_ppu_state* ppu, int layer_index, int row_index, int col_start, int col_end, unsigned int* row_pixels) {
    auto &layer = ppu->layers[layer_index];
    gen16x_ppu_layer_sprites* layer_sprites = (gen16x_ppu_layer_sprites*)(ppu->vram + layer.vram_offset);

    int blendmode = layer.blend_mode;

    
    const int num_bins = 256;
    const int sprites_per_bin = 16;
    char sprite_bins[num_bins][sprites_per_bin + 1] = {0};
    

    for (int s = 0; s < 128; s++) {
        gen16x_ppu_sprite& sprite = layer_sprites->sprites[s];
        int s_x = sprite.x;
        int s_y = sprite.y;
        int s_s = 1 << sprite.size;

        if ((s_y + s_s < 0) || (s_y >= ppu->screen_height)
            || (s_x + s_s < col_start) || (s_x >= col_start)) {
            continue;
        }


        for (int b = s_y; b < num_bins && b < s_y + s_s; ++b) {
            if (sprite_bins[b][0] < sprites_per_bin) {
                sprite_bins[b][++sprite_bins[b][0]] = (char)s;
            }
        }

    }

    int row_bin = (row_index >> 5) & 0xF;

    int n_sprites = sprite_bins[row_bin][0];
    for (int c = 0; c < n_sprites; c++) {
        int sprite_index = sprite_bins[row_bin][c + 1];
        gen16x_ppu_sprite& sprite = layer_sprites->sprites[sprite_index];
        int sprite_size = 1 << sprite.size;
        int start = sprite.x;
        if (col_start > start) {
            start = col_start;
        }
        int end = sprite.x + sprite_size;
        if (col_end < end) {
            end = col_end;
        }
        int y0 = (row_index - sprite.y);
        int s_sub_y = y0 >> 3;
        int s_sub_y0 = y0 & 0x7;
        
        for (int x = start; x < end; ++x) {
            int x0 = x - start;
            int s_sub_x = x0 >> 3;
            int s_sub_x0 = x0 & 0x7;
            int s_sub_offset = (s_sub_y << sprite.size) + s_sub_x;
            unsigned int s_offset = (sprite.tile_index << 5) 
                                  + (s_sub_offset << (sprite.size - 3))
                                  + (s_sub_y0 << sprite.size)
                                  + (s_sub_x0);

            unsigned int sprite_color = (int)layer_sprites->sprite_palette[s_offset >> 1];
            unsigned int mask_shift = (s_offset & 0x1);
            unsigned int mask = 0xF << mask_shift;
            sprite_color = (sprite_color & mask) >> mask_shift;
            write_pixel(blendmode, ppu->cgram32[sprite.color_palette[sprite_color]], *((gen16x_color32*)&row_pixels[x]));
        }
    }
}
void gen16x_ppu_render(gen16x_ppu_state* ppu) {

    for (int y = 0; y < ppu->screen_height; ++y) {
        if (ppu->row_callback) {
            ppu->row_callback(y);
        }
        unsigned int* row_pixels = (unsigned int*)(ppu->vram + ppu->framebuffer_offset) + y*ppu->screen_width;
        for (int i = 0; i < ppu->screen_width; ++i) {
            row_pixels[i] = 0;
        }
        //memset(row_pixels, 0, ppu->screen_width*4);
        for (int l = 0; l < 6; l++) {
            switch (ppu->layers[l].layer_type) {
            case GEN16X_LAYER_DIRECT:
                render_row_direct(ppu, l, y, 0, ppu->screen_width, row_pixels);
                break;
            case GEN16X_LAYER_TILES:
            {
                gen16x_ppu_layer_tiles * layer_tiles = (gen16x_ppu_layer_tiles*)(ppu->vram + ppu->layers[l].vram_offset);
                if (layer_tiles->tile_size == GEN16X_TILE8) {
                    render_row_tiles<3>(ppu, l, y, 0, ppu->screen_width, row_pixels);
                } else if (layer_tiles->tile_size == GEN16X_TILE16) {
                    render_row_tiles<4>(ppu, l, y, 0, ppu->screen_width, row_pixels);
                }
                break;
            }
            }
        }
    }
}

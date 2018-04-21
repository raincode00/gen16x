// gen16x.cpp : Defines the entry point for the application.
//
#include <cstdlib>
#include <string.h>
#include <cstdio>
#include <cmath>
#include <chrono>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <GL/glew.h>
#include "gen16x.h"
#include "shaders.h"

gen16x_ppu_state g_ppu_state;
SDL_Window* g_window;
class Timer {
public:
    Timer() : beg_(clock_::now()) {}
    void reset() { beg_ = clock_::now(); }
    double elapsed() const {
        return std::chrono::duration_cast<second_>
            (clock_::now() - beg_).count();
    }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> beg_;
};


uint64_t font_8x8[128] = {
    0x0000000000000000, 0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	
    0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	
    0x7E7E7E7E7E7E0000,	0x0000000000000000, 0x0000000000000000, 0x7E7E7E7E7E7E0000,	
    0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	
    0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	
    0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	
    0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	
    0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	0x7E7E7E7E7E7E0000,	
    0x0000000000000000, 0x0808080800080000,	0x2828000000000000,	0x00287C287C280000,	
    0x081E281C0A3C0800,	0x6094681629060000,	0x1C20201926190000,	0x0808000000000000,	
    0x0810202010080000,	0x1008040408100000,	0x2A1C3E1C2A000000,	0x0008083E08080000,	
    0x0000000000081000, 0x0000003C00000000,	0x0000000000080000, 0x0204081020400000,	
    0x1824424224180000,	0x08180808081C0000,	0x3C420418207E0000,	0x3C420418423C0000,	
    0x081828487C080000,	0x7E407C02423C0000,	0x3C407C42423C0000,	0x7E04081020400000,	
    0x3C423C42423C0000,	0x3C42423E023C0000,	0x0000080000080000, 0x0000080000081000,		
    0x0006186018060000,	0x00007E007E000000,	0x0060180618600000,	0x3844041800100000,	
    0x003C449C945C201C,	0x1818243C42420000,	0x7844784444780000,	0x3844808044380000,	
    0x7844444444780000,	0x7C407840407C0000,	0x7C40784040400000,	0x3844809C44380000,	
    0x42427E4242420000,	0x3E080808083E0000,	0x1C04040444380000,	0x4448507048440000,	
    0x40404040407E0000,	0x4163554941410000,	0x4262524A46420000,	0x1C222222221C0000,	
    0x7844784040400000,	0x1C222222221C0200,	0x7844785048440000,	0x1C22100C221C0000,	
    0x7F08080808080000,	0x42424242423C0000,	0x8142422424180000,	0x4141495563410000,	
    0x4224181824420000,	0x4122140808080000,	0x7E040810207E0000,	0x3820202020380000,	
    0x4020100804020000,	0x3808080808380000,	0x1028000000000000,	0x00000000007E0000,		
    0x1008000000000000,	0x003C023E463A0000,	0x40407C42625C0000,	0x00001C20201C0000,		
    0x02023E42463A0000,	0x003C427E403C0000,	0x0018103810100000,	0x0000344C44340438,		
    0x2020382424240000,	0x0800080808080000,	0x0800180808080870,	0x20202428302C0000,	
    0x1010101010180000,	0x0000665A42420000,	0x00002E3222220000,	0x00003C42423C0000,		
    0x00005C62427C4040,	0x00003A46423E0202,	0x00002C3220200000,	0x001C201804380000,	
    0x00103C1010180000,	0x00002222261A0000,	0x0000424224180000,	0x000081815A660000,		
    0x0000422418660000,	0x0000422214081060,	0x00003C08103C0000,	0x1C103030101C0000,
    0x0808080808080800,	0x38080C0C08380000,	0x000000324C000000,	0x7E7E7E7E7E7E0000};


const unsigned char test_tileset[256*256] = {
#include "assets/test_tileset.txt"
};


const unsigned char test_tilemap[32*32] = {
#include "assets/test_tilemap.txt"
};


const unsigned char test_background[1152 * 80] = {
#include "assets/test_background.txt"
};


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

void gen16x_init() {
    memset(&g_ppu_state, 0, sizeof(g_ppu_state));
    g_ppu_state.screen_height = 208;
    g_ppu_state.screen_width = 384;

    int i = 0;
    for (int r = 0; r < 6; r++) {
        for (int g = 0; g < 6; g++) {
            for (int b = 0; b < 6; b++) {
                float rf = float(r) / 5;
                float gf = float(g) / 5;
                float bf = float(b) / 5;

                unsigned int argb[4] = {
                    0xFF,
                    (unsigned int)(rf*255.0f),
                    (unsigned int)(gf*255.0f),
                    (unsigned int)(bf*255.0f),
                };
                int i = r*36 + g*6 + b;
                gen16x_color32 color;
                color.a = argb[0];
                color.r = argb[1];
                color.g = argb[2];
                color.b = argb[3];
                //unsigned int argb32 = (argb[0]) | (argb[1] << 8) | (argb[2] << 16) | (argb[3] << 24);
                //g_ppu_state.cgram[i] = argb32_to_argb16(color);
                g_ppu_state.cgram32[i] = color;

            }
        }
    }

    //g_ppu_state.cgram[0x00] = 0b0111110000011111;
    //g_ppu_state.cgram[0xEF] = 0b1000000000000000;
    //g_ppu_state.cgram[0xFF] = 0b1111111111111111;

    g_ppu_state.cgram32[0x00].color_i = 0x00FF00FF;
    g_ppu_state.cgram32[0xEF].color_i = 0xFF000000;
    g_ppu_state.cgram32[0xFF].color_i = 0xFFFFFFFF;


    int offset = 0;
    g_ppu_state.layers[0].layer_type = GEN16X_LAYER_DIRECT;
    g_ppu_state.layers[0].vram_offset = 0;

 
    gen16x_ppu_layer_direct& direct_layer = *(gen16x_ppu_layer_direct*)(g_ppu_state.vram + g_ppu_state.layers[0].vram_offset);
    

    direct_layer.width = 1152;
    direct_layer.height = 80;

    /*for (int i = 0; i < direct_layer.height; i++) {
        for (int j = 0; j < direct_layer.width; j++) {
            direct_layer.map[i * direct_layer.width + j] = ((i / 16) * direct_layer.width / 16) + j / 16;

            if (i == j || i == direct_layer.width - j - 1) {
                direct_layer.map[i * direct_layer.width + j] = 200;
            }
            direct_layer.map[i*direct_layer.width + j] = test_background[i*direct_layer.width + j];
        }
    }*/
    memcpy(direct_layer.map, test_background, sizeof(test_background));
    
    /*for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            direct_layer.map[(i + 16) * direct_layer.width + j + 16] = test_tileset[i*256 + j];
        }
    }*/

    offset += direct_layer.width*direct_layer.height + 128;

    g_ppu_state.layers[1].layer_type = GEN16X_LAYER_TILES;

    g_ppu_state.layers[1].vram_offset = offset;

    /*g_ppu_state.layers[2].layer_type = GEN16X_LAYER_TILES;
    g_ppu_state.layers[2].video_memory_offset = offset;
    g_ppu_state.layers[2].video_memory_size = sizeof(gen16x_ppu_layer_tiles);

    g_ppu_state.layers[3].layer_type = GEN16X_LAYER_TILES;
    g_ppu_state.layers[3].video_memory_offset = offset;
    g_ppu_state.layers[3].video_memory_size = sizeof(gen16x_ppu_layer_tiles);

    g_ppu_state.layers[4].layer_type = GEN16X_LAYER_TILES;
    g_ppu_state.layers[4].video_memory_offset = offset;
    g_ppu_state.layers[4].video_memory_size = sizeof(gen16x_ppu_layer_tiles);

    g_ppu_state.layers[5].layer_type = GEN16X_LAYER_TILES;
    g_ppu_state.layers[5].video_memory_offset = offset;
    g_ppu_state.layers[5].video_memory_size = sizeof(gen16x_ppu_layer_tiles);
    */

    

    gen16x_ppu_layer_tiles& tile_layer = *(gen16x_ppu_layer_tiles*)(g_ppu_state.vram + g_ppu_state.layers[1].vram_offset);

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int tile_index = i * 16 + j;
            for (int row = 0; row < 16; row++) {
                for (int col = 0; col < 16; col++) {
                    tile_layer.tile_palette[(tile_index << 8) | (row << 4) | (col)] = test_tileset[(i * 16 + row) * 256 + j*16 + col];
                }
                
            }
        }
    }
    
    memcpy(tile_layer.tile_map, test_tilemap, sizeof(test_tilemap));
    tile_layer.tile_size = GEN16X_TILE16;
    tile_layer.flags =  GEN16X_FLAG_TRANSFORM | GEN16X_FLAG_REPEAT_X | GEN16X_FLAG_REPEAT_Y;
    tile_layer.tilemap_width = 5;
    tile_layer.tilemap_height = 5;
    
    
    offset += (sizeof(gen16x_ppu_layer_tiles));


    g_ppu_state.layers[2].layer_type = GEN16X_LAYER_TILES;
    g_ppu_state.layers[2].vram_offset = offset;

    gen16x_ppu_layer_tiles& tile_layer2 = *(gen16x_ppu_layer_tiles*)(g_ppu_state.vram + g_ppu_state.layers[2].vram_offset);

    for (int i = 0; i < 128; i++) {
        uint64_t tmp = font_8x8[i];
        uint64_t mask = 0x8000000000000000L;
        for (int col = 0; col < 64; col++) {
            unsigned char b = (tmp & (mask >> col)) ? 32 : 0x00;
            tile_layer2.tile_palette[(i << 7) | (col)] = b;
        }
    }
    char test_text[] = "hello world";

    memcpy(tile_layer2.tile_map, test_text, sizeof(test_text));
    tile_layer2.tile_size = GEN16X_TILE8;
    tile_layer2.flags = 0;
    tile_layer2.tilemap_width = 4;
    tile_layer2.tilemap_height = 1;

    //tile_layer2.transform.base = 8;
    tile_layer2.transform.x = -10;
    tile_layer2.transform.y = -8;
    tile_layer2.transform.a = 1 << (tile_layer2.transform.base);
    tile_layer2.transform.b = 0;
    tile_layer2.transform.c = 0;
    tile_layer2.transform.d = 1 << (tile_layer2.transform.base);


    offset += (sizeof(gen16x_ppu_layer_tiles));

    g_ppu_state.framebuffer_offset = offset;


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

void render_row_direct(int layer_index, int row_index, int col_start, int col_end, unsigned int* row_pixels) {
    auto &layer = g_ppu_state.layers[layer_index];
    gen16x_ppu_layer_direct * layer_direct = (gen16x_ppu_layer_direct*)(g_ppu_state.vram + layer.vram_offset);
    
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

    int div1 = layer_direct->width >> 4;

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
        write_pixel(blendmode, g_ppu_state.cgram32[row_src[x0]], *((gen16x_color32*)&row_pixels[x]));
    }
}


template<int tile_size_shift>
void render_row_tiles(int layer_index, int row_index, int col_start, int col_end, unsigned int* row_pixels) {
    auto &layer = g_ppu_state.layers[layer_index];

    gen16x_ppu_layer_tiles * layer_tiles = (gen16x_ppu_layer_tiles*)(g_ppu_state.vram + layer.vram_offset);
    
    const gen16x_color32* cgram32 = g_ppu_state.cgram32;
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

        write_pixel(blendmode, cgram32[tile_pixel], *((gen16x_color32*)&row_pixels[x]));
    }

}

struct player_state {
    float pos_x;
    float pos_y;
    float height;
    float rot;
};

player_state player;



void render() {
    
    static int t = -1;
    t++;
    
    gen16x_ppu_layer_tiles* layer_tiles = (gen16x_ppu_layer_tiles*)(g_ppu_state.vram + g_ppu_state.layers[1].vram_offset);
    g_ppu_state.layers[0].layer_type = GEN16X_LAYER_DIRECT;
    g_ppu_state.layers[1].layer_type = GEN16X_LAYER_PASS;
    g_ppu_state.layers[2].layer_type = GEN16X_LAYER_TILES;

    //layer_tiles->transform.base = 12;

    gen16x_ppu_layer_direct& direct_layer = *(gen16x_ppu_layer_direct*)(g_ppu_state.vram + g_ppu_state.layers[0].vram_offset);

    direct_layer.scroll_y = 0;
    direct_layer.scroll_x = (short)((-player.rot/120.0f)*g_ppu_state.screen_width);
    direct_layer.flags |= GEN16X_FLAG_REPEAT_X;

    for (int y = 0; y < g_ppu_state.screen_height; ++y) {        
        if (y > 16) {
            g_ppu_state.layers[2].layer_type = GEN16X_LAYER_PASS;
        }
        int h = y - 80;

        if (h > 0) {

            g_ppu_state.layers[0].layer_type = GEN16X_LAYER_PASS;
            g_ppu_state.layers[1].layer_type = GEN16X_LAYER_TILES;
            float ay = player.height + 16;
                
            float lambda = ay / (float(h + 4.0f));
                
                 
            float p_cos = cosf(player.rot*3.1415926f/180.0f);
            float p_sin = sinf(player.rot*3.1415926f/180.0f);


            float f_x = p_sin;
            float f_y = p_cos;


            layer_tiles->transform.a = (int)(((1 << layer_tiles->transform.base)* 1.0f *  p_cos)*lambda);
            layer_tiles->transform.b = (int)(((1 << layer_tiles->transform.base)* 1.0f *  p_sin)*lambda);
            layer_tiles->transform.c = (int)(((1 << layer_tiles->transform.base)* 1.0f * -p_sin)*lambda);
            layer_tiles->transform.d = (int)(((1 << layer_tiles->transform.base)* 1.0f *  p_cos)*lambda);

            layer_tiles->transform.x =  (int)(player.pos_x - 0*f_x*lambda);
            layer_tiles->transform.y =  (int)(player.pos_y - 0*f_y*lambda);
            layer_tiles->transform.cx = (int)(player.pos_x - 0*f_x*lambda + g_ppu_state.screen_width / 2);
            layer_tiles->transform.cy = (int)(player.pos_y - 0*f_y*lambda + g_ppu_state.screen_height - 16);

        }

        unsigned int* row_pixels = (unsigned int*)(g_ppu_state.vram + g_ppu_state.framebuffer_offset) + y*g_ppu_state.screen_width;
        memset(row_pixels, 0, g_ppu_state.screen_width * 4);
        for (int l = 0; l < 6; l++) {
            switch (g_ppu_state.layers[l].layer_type) {
            case GEN16X_LAYER_DIRECT:
                render_row_direct(l, y, 0, g_ppu_state.screen_width, row_pixels);
                break;
            case GEN16X_LAYER_TILES:
            {
                gen16x_ppu_layer_tiles * layer_tiles = (gen16x_ppu_layer_tiles*)(g_ppu_state.vram + g_ppu_state.layers[l].vram_offset);
                if (layer_tiles->tile_size == GEN16X_TILE8) {
                    render_row_tiles<3>(l, y, 0, g_ppu_state.screen_width, row_pixels);
                } else if (layer_tiles->tile_size == GEN16X_TILE16) {
                    render_row_tiles<4>(l, y, 0, g_ppu_state.screen_width, row_pixels);
                }
                
                break;
            }
            }
        }
    }

    
   

}


bool compile_shader(uint32_t shader_type, uint32_t shader, const char* shader_source) {
    glShaderSource(shader, 1, &shader_source, 0);

    int result;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {
        int log_size;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
        if (log_size > 0) {
            char* error_log = new char[log_size];
            glGetShaderInfoLog(shader, log_size, &log_size, error_log);
            printf("%s\n\n%s\n", shader_source, error_log);
            delete[] error_log;
            return false;
        }
    }

    return true;
}


auto quitting = false;
int SDLCALL watch(void *userdata, SDL_Event* event) {

    if (event->type == SDL_APP_WILLENTERBACKGROUND) {
        quitting = true;
    }

    return 1;
}


int main() {
    
    gen16x_init();
    printf("Initialized %d bytes of vram\n", (int)sizeof(g_ppu_state));
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);


    g_window = SDL_CreateWindow("gen16x", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, g_ppu_state.screen_width, g_ppu_state.screen_height, SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI);

    SDL_SetWindowResizable(g_window, SDL_TRUE);
    
    //SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);

    auto gl_context = SDL_GL_CreateContext(g_window);
    
    SDL_GL_SetSwapInterval(0);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        printf("Error: %s\n", glewGetErrorString(err));
    }
    
    printf("Status: Using OpenGL %s\n", glGetString(GL_VERSION));

    SDL_AddEventWatch(watch, NULL);
    double frame_no = 0.0;
    Timer timer;

    uint32_t framebuffer_texture;
    glGenTextures(1, &framebuffer_texture);

    glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    float quad[] =
    {
        -1.0f,  1.0f,
        -1.0f, -1.0f,
        1.0f,  1.0f,
        1.0f, -1.0f	 
    };

    
    uint32_t quad_va;
    uint32_t quad_vbo;
    glGenVertexArrays(1, &quad_va);

    glBindVertexArray(quad_va);
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    

    
    uint32_t program = glCreateProgram();
    
    uint32_t quad_vsh = glCreateShader(GL_VERTEX_SHADER);
    uint32_t quad_psh = glCreateShader(GL_FRAGMENT_SHADER);

    if (compile_shader(GL_VERTEX_SHADER, quad_vsh, g_quad_vsh)) {
        glAttachShader(program, quad_vsh);
    }
    
    if (compile_shader(GL_FRAGMENT_SHADER, quad_psh, g_quad_psh)) {
        glAttachShader(program, quad_psh);
    }

    int result;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &result);

    if (result == GL_FALSE) {
        int log_size;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
        if (log_size > 0) {
            char* error_log = new char[log_size];
            glGetProgramInfoLog(program, log_size, &log_size, error_log);
            printf("%s\n", error_log);
            delete[] error_log;
            return 1;
        }
    }

    glDeleteShader(quad_vsh);
    glDeleteShader(quad_psh);

    int pos_attr = glGetAttribLocation(program, "position");
    
    glVertexAttribPointer(pos_attr, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(pos_attr);


    glUseProgram(program);
    int texture_uni = glGetUniformLocation(program, "texture");

    int display_size_uni = glGetUniformLocation(program, "display_size");
    int texture_size_uni = glGetUniformLocation(program, "texture_size");


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer_texture);
    
    err = glGetError();
    if (err) {
        printf("OpenGL Error - %d\n", err);
        quitting = true;
    }


    glProgramUniform1i(program, texture_uni, 0);
    glProgramUniform2f(program, texture_size_uni, g_ppu_state.screen_width, g_ppu_state.screen_height);
    int draw_w;
    int draw_h;

    SDL_GL_GetDrawableSize(g_window, &draw_w, &draw_h);
    glProgramUniform2f(program, display_size_uni, (float)draw_w, (float)draw_h);
    glViewport(0, 0, draw_w, draw_h);
    //SDL_SetWindowSize(g_window, g_ppu_state.screen_width, g_ppu_state.screen_height);
    
    
    player.pos_x = 0;
    player.pos_y = 0;
    player.height = 0;
    player.rot = 0;
    float delta_time = 0;

    while (!quitting) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quitting = true;
            } else if (event.type == SDL_WINDOWEVENT) {
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    {
                        //SDL_Surface* window_surface = SDL_GetWindowSurface(g_window);
                        
                        int new_w = event.window.data1;
                        int new_h = event.window.data2;
                        
                        float dpi_scale_w = 1.0f;
                        float dpi_scale_h = 1.0f;
                        
                        int d_w;
                        int d_h;
                        
                        SDL_GL_GetDrawableSize(g_window, &d_w, &d_h);
                        
                        dpi_scale_w = (float)d_w/(float)new_w;
                        dpi_scale_h = (float)d_h/(float)new_h;

                        printf("Attempted resized to: %d x %d\n", new_w, new_h);
                        
                        float scale_w = float(new_w >> 2) / float(g_ppu_state.screen_width >>  2);
                        float scale_h = float(new_h >> 2) / float(g_ppu_state.screen_height >> 2);

                        //scale_w = floor(scale_w + 0.5f);
                        //scale_h = floor(scale_h + 0.5f);

                        new_w = (g_ppu_state.screen_width);
                        new_h = (g_ppu_state.screen_height);

                        if (scale_w > scale_h) {
                            new_w = (int)((float)new_w*scale_h/4.0f) << 2;
                            new_h = (int)((float)new_h*scale_h/4.0f) << 2;
                        } else {
                            new_w = (int)((float)new_w*scale_w/4.0f) << 2;
                            new_h = (int)((float)new_h*scale_w/4.0f) << 2;
                        }

                        int vp_width = new_w;
                        int vp_height = new_h;

                        if (vp_width*dpi_scale_w < g_ppu_state.screen_width || vp_height*dpi_scale_h < g_ppu_state.screen_height) {
                            vp_width = (float)g_ppu_state.screen_width/dpi_scale_w;
                            vp_height = (float)g_ppu_state.screen_height/dpi_scale_h;
                        }
                        
                        int offset_w = (event.window.data1 - vp_width) / 2;
                        int offset_h = (event.window.data2 - vp_height) / 2;
                        
                        glProgramUniform2f(program, display_size_uni, (float)vp_width*dpi_scale_w, (float)vp_height*dpi_scale_h);
                        glViewport(offset_w*dpi_scale_w, offset_h*dpi_scale_h, vp_width*dpi_scale_w, vp_height*dpi_scale_h);

                        Uint32 flags = SDL_GetWindowFlags(g_window);
                        if (!(flags & (SDL_WINDOW_FULLSCREEN|SDL_WINDOW_MAXIMIZED))) {
                            if (offset_w != 0 || offset_h != 0) {
                                SDL_SetWindowSize(g_window, vp_width, vp_height);
                                glViewport(0, 0, vp_width*dpi_scale_w, vp_height*dpi_scale_h);
                            }
                            
                        }
                        printf("Window resized to: %d x %d\n", vp_width, vp_height);
                        break;
                    }
                }

            } else if (event.type == SDL_KEYDOWN) {
                bool mod_ctrl = !!(event.key.keysym.mod & KMOD_CTRL);
                bool mod_shift = !!(event.key.keysym.mod & KMOD_SHIFT);
                bool mod_alt = !!(event.key.keysym.mod & KMOD_ALT);

                switch (event.key.keysym.sym) {
                case SDLK_RETURN:
                    if (mod_alt) {
                        Uint32 flags = SDL_GetWindowFlags(g_window);
                        if (flags & SDL_WINDOW_FULLSCREEN) {
                            SDL_SetWindowFullscreen(g_window, SDL_FALSE);
                        }
                        else {
                            SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                    }
                    break;
                }

            }

        }

        const Uint8 *kbstate = SDL_GetKeyboardState(NULL);

        float speed_scale = 20.0f;
        if (kbstate[SDL_SCANCODE_W]) {
            float forward_x = -sinf(3.1415926f*player.rot/ 180.0f);
            float forward_y = cosf(3.1415926f*player.rot / 180.0f);

            player.pos_x += speed_scale*10.0f*delta_time*forward_x;
            player.pos_y += speed_scale*10.0f*delta_time*-forward_y;


        }

        if (kbstate[SDL_SCANCODE_S]) {
            float forward_x = -sinf(3.1415926f*player.rot/180.0f);
            float forward_y = cosf(3.1415926f*player.rot / 180.0f);

            player.pos_x += 5.0f*speed_scale*delta_time*-forward_x;
            player.pos_y += 5.0f*speed_scale*delta_time*forward_y;

        }

        if (kbstate[SDL_SCANCODE_A]) {
            player.rot += 10.0f*speed_scale*delta_time;
        }

        if (kbstate[SDL_SCANCODE_D]) {
            player.rot -= 10.0f*speed_scale*delta_time;
        }

        if (kbstate[SDL_SCANCODE_F]) {
            player.height -= 4.0f*speed_scale*delta_time;
            if (player.height < 0) player.height = 0;
        }
        if (kbstate[SDL_SCANCODE_R]) {
            player.height += 4.0f*speed_scale*delta_time;
        }

        gen16x_ppu_layer_tiles& tile_layer = *(gen16x_ppu_layer_tiles*)(g_ppu_state.vram + g_ppu_state.layers[1].vram_offset);
        

        render();
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, g_ppu_state.screen_width, g_ppu_state.screen_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, g_ppu_state.vram + g_ppu_state.framebuffer_offset);
        int err = glGetError();
        if (err) {
            printf("OpenGL Error - %d\n", err);
            quitting = true;
        }


     
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);

        
        if (timer.elapsed() >= 1.0/60.0f && frame_no > 0.0) {
            char fps_text[32];
            delta_time = (float)(timer.elapsed() / frame_no);
            sprintf(fps_text, "FPS: %0.2f\n", frame_no / timer.elapsed());
            //printf("%s", fps_text);
            gen16x_ppu_layer_tiles& tile_layer2 = *(gen16x_ppu_layer_tiles*)(g_ppu_state.vram + g_ppu_state.layers[2].vram_offset);
            strncpy((char*)tile_layer2.tile_map, fps_text, 32);
            

            frame_no = 0.0;
            timer.reset();
        }
        frame_no++;
        SDL_GL_SwapWindow(g_window);
        
    }

    
    glDeleteTextures(1, &framebuffer_texture);
    SDL_DelEventWatch(watch, NULL);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(g_window);

    SDL_Quit();

	return 0;
}

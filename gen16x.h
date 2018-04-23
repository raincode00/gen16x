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

struct gen16x_ppu_transform {
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
struct gen16x_ppu_layer_tiles {
    unsigned char tile_palette[256*256];    //holds 256 16x16 tiles
    unsigned char tile_map[256 * 256];
};

struct gen16x_ppu_sprite {
    short x;
    short y;
    unsigned char flags;
    unsigned char priority;
    unsigned char size;
    unsigned char palette_offset;
    unsigned short tile_index;
}; //10 bytes

struct gen16x_ppu_layer_sprites {
    unsigned char sprite_palette[1024*128];  // holds 1024 16x16 sprites or 4096 8x8 sprites
};

struct gen16x_ppu_layer_direct {
    unsigned char map[512*256];
};

struct gen16x_ppu_layer_header {
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
            gen16x_ppu_transform transform;
        } tile_layer;
        struct {
            gen16x_ppu_sprite sprites[GEN16X_MAX_SPRITES];
        } sprite_layer;
        
        
    };
};
struct gen16x_ppu_state;

typedef void(*gen16x_ppu_row_callback_t)(gen16x_ppu_state*, unsigned int);

struct gen16x_ppu_state {
    unsigned short screen_width;
    unsigned short screen_height;
    gen16x_ppu_row_callback_t row_callback;
    gen16x_ppu_layer_header layers[6];
    unsigned int framebuffer_offset;
    gen16x_color32 cgram32[256];
    unsigned char vram[512*512*4];
};

void gen16x_ppu_render(gen16x_ppu_state* ppu);




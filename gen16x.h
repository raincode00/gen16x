#pragma once

#define GEN16X_LAYER_DIRECT         0x01
#define GEN16X_LAYER_TILES          0x02
#define GEN16X_LAYER_SPRITES        0x03
#define GEN16X_LAYER_PASS           0x04

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
#define GEN16X_FLAG_SPRITE_ENABLED  0b00000001

#define GEN16X_MAX_SCREEN_HEIGHT    256
#define GEN16X_MAX_SCREEN_WIDTH     512
#define GEN16X_MAX_SPRITES          256
#define GEN16X_MAX_SPRITES_PER_ROW  15



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
    unsigned char tile_size;
    unsigned char reserved;
    unsigned short tilemap_width;
    unsigned short tilemap_height;
    unsigned short flags;
    gen16x_ppu_transform transform;
    unsigned char tile_palette[256*256];    //holds 256 16x16 tiles
    unsigned char tile_map[256 * 256];
};

struct gen16x_ppu_sprite {
    unsigned char size;
    unsigned char flags;
    short x;
    short y;
    unsigned short tile_index;
    unsigned char color_palette[16];
}; //24 bytes

struct gen16x_ppu_layer_sprites {
    gen16x_ppu_sprite sprites[GEN16X_MAX_SPRITES];
    unsigned char sprite_palette[1024*128];  // holds 1024 16x16 sprites or 4096 8x8 sprites
};

struct gen16x_ppu_layer_direct {
    short flags;
    short scroll_x;
    short scroll_y;
    short width;
    short height;
    unsigned char map[512*256];
};

struct gen16x_ppu_layer_header {
    unsigned char layer_type;
    unsigned char blend_mode;
    unsigned int vram_offset;
};

typedef void(*gen16x_ppu_row_callback_t)(unsigned int);

struct gen16x_ppu_state {
    unsigned short screen_width;
    unsigned short screen_height;
    gen16x_ppu_row_callback_t row_callback;
    gen16x_ppu_layer_header layers[6];
    unsigned int framebuffer_offset;
    gen16x_color32 cgram32[256];
    unsigned char vram[512*512*4 - (2 + 2 + sizeof(gen16x_ppu_row_callback_t) + sizeof(gen16x_ppu_layer_header)*6 + 4 + 4*256)];
};

void gen16x_ppu_render(gen16x_ppu_state* ppu);



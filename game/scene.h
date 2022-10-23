#pragma once
#include <string.h>
#include "gen16x.h"
#include "sprite.h"
#include "world.h"


#define SCENE_TILE_BACKGROUND_LAYER0 0
#define SCENE_TILE_BACKGROUND_LAYER1 1
#define SCENE_SPRITE_LAYER 2
#define SCENE_TILE_FOREGROUND_LAYER 3
#define SCENE_TILE_HUD_LAYER0 4
#define SCENE_TILE_HUD_LAYER1 5
#define SCENE_MAX_SPRITES 1024

struct Scene {
    vec2 camera_pos;
    int view_width;
    int view_height;
    int num_sprites;
    Sprite sprites[SCENE_MAX_SPRITES];
    World world;
};

extern "C" int scene_load_ppu_data(void* ppu_data, int* in_out_offset, int data_size, void* rom_data, int max_num_used, int* in_out_num_used, int* used_offsets, void** used_rom);
extern "C" void scene_load_ppu(Scene* scene, gen16x_ppu* ppu, int* in_out_vram_offset, int* in_out_cgram_offset);
extern "C" void scene_render_ppu(Scene* scene, gen16x_ppu* ppu);
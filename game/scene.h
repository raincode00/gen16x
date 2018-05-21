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


int scene_load_ppu_data(void* ppu_data, int* in_out_offset, int data_size, void* rom_data, int max_num_used, int* in_out_num_used, int* used_offsets, void** used_rom) {
    int used_offset = -1;
    for (int j = 0; j < *in_out_num_used; j++) {
        if (rom_data == used_rom[j]) {
            used_offset = j;
            break;
        }
    }
    int offset = 0;
    offset = *in_out_offset;
    if (used_offset == -1) {
        int used_index = (*in_out_num_used)++;
        if (used_index < max_num_used) {
            used_offsets[used_index] = *in_out_offset;
            used_rom[used_index] = rom_data;
        }

        memcpy(ppu_data, rom_data, data_size);
        
        *in_out_offset += data_size;
    } else {
        offset = used_offsets[used_offset];
    }

    return offset;
}
void scene_load_ppu(Scene* scene, gen16x_ppu* ppu, int* in_out_vram_offset, int* in_out_cgram_offset) {
    int ppu_vram_offset = 0;
    int tile_ppu_layers[5] = {
        0, 1, 3, 4, 5
    };
    int tmp0, tmp1;
    int& cgram_offset = in_out_cgram_offset ? *in_out_cgram_offset : tmp1;
    int& offset = in_out_vram_offset ? *in_out_vram_offset : tmp0;

    int num_used_tilesets = 0;
    int num_used_tilemaps = 0;
    int num_used_palettes = 0;
    int num_used_sprites = 0;

    int used_tilesets_offsets[5] = {-1};
    int used_tilemaps_offsets[5] = {-1};
    int used_palettes_offsets[64] = {-1};
    int used_sprite_offsets[256] = {-1};

    void* used_tilesets_rom[5] = {0};
    void* used_tilemaps_rom[5] = {0};
    void* used_palettes_rom[64] = {0};
    void* used_sprites_rom[64] = {0};
    


    for (int i = 0; i < 5; i++) {
        
        gen16x_layer_header& ppu_layer = ppu->layers[tile_ppu_layers[i]];
        WorldTileLayer& layer = scene->world.tile_layers[i];
        if (!layer.enabled) continue;

        int tmp_offset = scene_load_ppu_data(ppu->vram + offset, &offset, 
            layer.rom_tileset_size, layer.rom_tileset_base, 
            5, &num_used_tilesets, used_tilesets_offsets, used_tilesets_rom);

        layer.ppu_tileset_base = tmp_offset;
        ppu_layer.vram_offset = tmp_offset;

        
        tmp_offset = scene_load_ppu_data(ppu->vram + offset, &offset, 
            layer.rom_tilemap_size, layer.rom_tilemap_base, 
            5, &num_used_tilemaps, used_tilemaps_offsets, used_tilemaps_rom);

        ppu_layer.vram_offset = tmp_offset;
        layer.ppu_tilemap_base = tmp_offset;



        tmp_offset = scene_load_ppu_data(ppu->cgram32 + cgram_offset/4, &cgram_offset, 
            layer.rom_cgram_size, layer.rom_cgram_base,
            64, &num_used_palettes, used_palettes_offsets, used_palettes_rom);

        ppu_layer.tile_layer.palette_offset = tmp_offset/4;
        layer.ppu_cgram_base = tmp_offset/4;


    }

    gen16x_layer_header& ppu_sprites_layer = ppu->layers[SCENE_SPRITE_LAYER];
    ppu_sprites_layer.vram_offset = offset;
    int sprite_base_offset = offset;


    for (int i = 0; i < scene->num_sprites; i++) {
        Sprite& sprite = scene->sprites[i];
        int tmp_offset = scene_load_ppu_data(ppu->vram + offset, &offset, 
            sprite.rom_sprite_size, sprite.rom_sprite_base,
            256, &num_used_sprites, used_sprite_offsets, used_sprites_rom);
        sprite.ppu_sprite_base = (tmp_offset - sprite_base_offset) >> 5;

        tmp_offset = scene_load_ppu_data(ppu->cgram32 + cgram_offset/4, &cgram_offset, 
            sprite.rom_cgram_size, sprite.rom_cgram_base,
            64, &num_used_palettes, used_palettes_offsets, used_palettes_rom);
        sprite.ppu_cgram_base = tmp_offset/4;
        
    }
}

void scene_render_ppu(Scene* scene, gen16x_ppu* ppu) {
    WorldEntity sprite_entities[256];
    vec2 viewport_size = vec2((float)scene->view_width, (float)scene->view_height);
    
    vec2 viewport_min = scene->camera_pos - viewport_size*0.5f;
    vec2 viewport_max = scene->camera_pos + viewport_size*0.5f;

    /*render sprites*/ {
        int sprite_mask = 1 << WORLD_ENTITY_TYPE_SPRITE;
        int num_sprites = world_get_entities(&scene->world, viewport_min, viewport_max, 0xFFFF, sprite_mask, 256, sprite_entities);

        // selection sort sprites
        for (int i = 0; i < num_sprites - 1; i++) {
            const Sprite& s0 = scene->sprites[sprite_entities[i].id];
            for (int j = i + 1; j < num_sprites; j++) {
                if (j != i) {
                    const Sprite& s1 = scene->sprites[sprite_entities[j].id];
                    if (s1.z_order < s0.z_order) {
                        WorldEntity tmp = sprite_entities[i];
                        sprite_entities[i] = sprite_entities[j];
                        sprite_entities[j] = tmp;
                    }
                }

            }
        }

        gen16x_layer_header& sprite_layer = ppu->layers[SCENE_SPRITE_LAYER];
        sprite_layer.layer_type = GEN16X_LAYER_SPRITES;
        int num_visible_sprites = 0;
        for (int i = 0; i < num_sprites; i++) {
            Sprite& sprite = scene->sprites[sprite_entities[i].id];
            if (!sprite.visible) continue;

            int si = num_visible_sprites++;
            
            gen16x_sprite& ppu_sprite = ppu->sprites[si];
            sprite.ppu_sprite_index = si;
            ppu_sprite.flags = GEN16X_FLAG_SPRITE_ENABLED;
            ppu_sprite.palette_offset = sprite.ppu_cgram_base;
            ppu_sprite.size_w = sprite.size_w;
            ppu_sprite.size_h = sprite.size_h;
            ppu_sprite.priority = sprite.priority;
            ppu_sprite.x = short(-viewport_min.x + sprite.pos.x - sprite.origin.x*sprite.scale.x);
            ppu_sprite.y = short(-viewport_min.y + sprite.pos.y - sprite.origin.y*sprite.scale.y);
            ppu_sprite.scale_x = short(256.0f/sprite.scale.x);
            ppu_sprite.scale_y = short(256.0f/sprite.scale.y);

            int sprite_frame_size = ((1<<sprite.size_w)*(1<<sprite.size_h)) >> 6;
            ppu_sprite.tile_index = sprite.ppu_sprite_base + sprite.current_frame*sprite_frame_size;
        }
        sprite_layer.sprite_layer.sprites_base = 0;
        sprite_layer.sprite_layer.num_sprites = num_visible_sprites;
        for (int i = num_visible_sprites; i < 256; i++) {
            ppu->sprites[i].flags = 0;
        }
    }
    
    /*render tiles*/ {

        gen16x_layer_header& ppu_bg0 = ppu->layers[SCENE_TILE_BACKGROUND_LAYER0];
        gen16x_layer_header& ppu_bg1 = ppu->layers[SCENE_TILE_BACKGROUND_LAYER1];
        gen16x_layer_header& ppu_fg = ppu->layers[SCENE_TILE_FOREGROUND_LAYER];
        gen16x_layer_header& ppu_hud0 = ppu->layers[SCENE_TILE_HUD_LAYER0];
        gen16x_layer_header& ppu_hud1 = ppu->layers[SCENE_TILE_HUD_LAYER1];


        ppu_bg0.layer_type = scene->world.tile_layers[0].enabled ? GEN16X_LAYER_TILES : GEN16X_LAYER_NONE;
        ppu_bg0.vram_offset = scene->world.tile_layers[0].ppu_tileset_base;
        ppu_bg0.tile_layer.tile_size = GEN16X_TILE16;
        ppu_bg0.tile_layer.flags = GEN16X_FLAG_REPEAT_X | GEN16X_FLAG_REPEAT_Y;
        ppu_bg0.tile_layer.palette_offset = scene->world.tile_layers[0].ppu_cgram_base;
        ppu_bg0.tile_layer.tilemap_vram_offset = scene->world.tile_layers[0].ppu_tilemap_base;
        ppu_bg0.tile_layer.tilemap_width = scene->world.tile_layers[0].tilemap_width;
        ppu_bg0.tile_layer.tilemap_height = scene->world.tile_layers[0].tilemap_height;
        ppu_bg0.tile_layer.transform.x = int(viewport_min.x - scene->world.tile_layers[0].position.x);
        ppu_bg0.tile_layer.transform.y = int(viewport_min.y - scene->world.tile_layers[0].position.y);
        scene->world.tile_layers[0].ppu_layer = SCENE_TILE_BACKGROUND_LAYER0;

        ppu_bg1.layer_type = scene->world.tile_layers[1].enabled ? GEN16X_LAYER_TILES : GEN16X_LAYER_NONE;
        ppu_bg1.vram_offset = scene->world.tile_layers[1].ppu_tileset_base;
        ppu_bg1.tile_layer.tile_size = GEN16X_TILE16;
        ppu_bg1.tile_layer.flags = GEN16X_FLAG_CLAMP_X | GEN16X_FLAG_CLAMP_Y;
        ppu_bg1.tile_layer.palette_offset = scene->world.tile_layers[1].ppu_cgram_base;
        ppu_bg1.tile_layer.tilemap_vram_offset = scene->world.tile_layers[1].ppu_tilemap_base;
        ppu_bg1.tile_layer.tilemap_width = scene->world.tile_layers[1].tilemap_width;
        ppu_bg1.tile_layer.tilemap_height = scene->world.tile_layers[1].tilemap_height;
        ppu_bg1.tile_layer.transform.x = int(viewport_min.x - scene->world.tile_layers[1].position.x);
        ppu_bg1.tile_layer.transform.y = int(viewport_min.y - scene->world.tile_layers[1].position.y);
        scene->world.tile_layers[1].ppu_layer = SCENE_TILE_BACKGROUND_LAYER1;

        ppu_fg.layer_type = scene->world.tile_layers[2].enabled ? GEN16X_LAYER_TILES : GEN16X_LAYER_NONE;
        ppu_fg.vram_offset = scene->world.tile_layers[2].ppu_tileset_base;
        ppu_fg.tile_layer.tile_size = GEN16X_TILE16;
        ppu_fg.tile_layer.flags = GEN16X_FLAG_CLAMP_X | GEN16X_FLAG_CLAMP_Y;
        ppu_fg.tile_layer.palette_offset = scene->world.tile_layers[2].ppu_cgram_base;
        ppu_fg.tile_layer.tilemap_vram_offset = scene->world.tile_layers[2].ppu_tilemap_base;
        ppu_fg.tile_layer.tilemap_width = scene->world.tile_layers[2].tilemap_width;
        ppu_fg.tile_layer.tilemap_height = scene->world.tile_layers[2].tilemap_height;
        ppu_fg.tile_layer.transform.x = int(viewport_min.x - scene->world.tile_layers[2].position.x);
        ppu_fg.tile_layer.transform.y = int(viewport_min.y - scene->world.tile_layers[2].position.y);
        scene->world.tile_layers[2].ppu_layer = SCENE_TILE_FOREGROUND_LAYER;

        ppu_hud0.layer_type = scene->world.tile_layers[3].enabled ? GEN16X_LAYER_TILES : GEN16X_LAYER_NONE;
        ppu_hud0.vram_offset = scene->world.tile_layers[3].ppu_tileset_base;
        ppu_hud0.tile_layer.tile_size = GEN16X_TILE8;
        ppu_hud0.tile_layer.flags = 0;
        ppu_hud0.tile_layer.palette_offset = scene->world.tile_layers[3].ppu_cgram_base;
        ppu_hud0.tile_layer.tilemap_vram_offset = scene->world.tile_layers[3].ppu_tilemap_base;
        ppu_hud0.tile_layer.tilemap_width = scene->world.tile_layers[3].tilemap_width;
        ppu_hud0.tile_layer.tilemap_height = scene->world.tile_layers[3].tilemap_height;
        ppu_hud0.tile_layer.transform.x = int(-scene->world.tile_layers[3].position.x);
        ppu_hud0.tile_layer.transform.y = int(-scene->world.tile_layers[3].position.y);
        scene->world.tile_layers[3].ppu_layer = SCENE_TILE_HUD_LAYER0;

        ppu_hud1.layer_type = scene->world.tile_layers[4].enabled ? GEN16X_LAYER_TILES : GEN16X_LAYER_NONE;
        ppu_hud1.vram_offset = scene->world.tile_layers[4].ppu_tileset_base;
        ppu_hud1.tile_layer.tile_size = GEN16X_TILE8;
        ppu_hud1.tile_layer.flags = 0;
        ppu_hud1.tile_layer.palette_offset = scene->world.tile_layers[4].ppu_cgram_base;
        ppu_hud1.tile_layer.tilemap_vram_offset = scene->world.tile_layers[4].ppu_tilemap_base;
        ppu_hud1.tile_layer.tilemap_width = scene->world.tile_layers[4].tilemap_width;
        ppu_hud1.tile_layer.tilemap_height = scene->world.tile_layers[4].tilemap_height;
        ppu_hud1.tile_layer.transform.x = int(-scene->world.tile_layers[4].position.x);
        ppu_hud1.tile_layer.transform.y = int(-scene->world.tile_layers[4].position.y);
        scene->world.tile_layers[4].ppu_layer = SCENE_TILE_HUD_LAYER1;
    }

}
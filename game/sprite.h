#pragma once
#include "2dmath.h"
#include "physics.h"
struct SpriteAnimation {
    bool loop;
    float current_time;
    int num_frames;
    int frames[16];
    float frame_times[16];
};
struct Sprite {
    vec2 pos;
    vec2 origin;
    vec2 scale;
    int size_w;
    int size_h;
    int z_order;
    int priority;
    
    bool transparent;
    bool visible;

    bool enable_collisions;
    bool collision_layers[8];
    BoxCollider collider;

    int current_frame;
    int num_frames;
    SpriteAnimation animations[32];

    int ppu_sprite_index;
    int ppu_sprite_base;
    int ppu_cgram_base;
    
    
    int rom_cgram_size;
    void* rom_cgram_base;

    int rom_sprite_size;
    void* rom_sprite_base;
    

};



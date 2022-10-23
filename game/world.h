#pragma once
#include "physics.h"

#define WORLD_NODE_MAX_ENTITIES 8
#define WORLD_ENTITY_TYPE_NONE 0
#define WORLD_ENTITY_TYPE_SPRITE 1
#define WORLD_ENTITY_TYPE_COLLIDER 2


struct WorldEntity {
    unsigned int id : 16;
    unsigned int type : 8;
    unsigned int layer : 4;
};



struct WorldGridNode {
    unsigned char num_entities;
    WorldEntity entities[WORLD_NODE_MAX_ENTITIES];
};

struct WorldStaticCollisionLayer {
    int num_colliders;
    ConvexCollider colliders[256];
};

struct WorldTileLayer {
    bool enabled;
    vec2 position;
    vec2 origin;
    vec2 scale;
    float rotation;

    int tile_size;
    int tilemap_width;
    int tilemap_height;

    int ppu_layer;
    int ppu_tileset_base;
    int ppu_tilemap_base;
    int ppu_cgram_base;
    
    int rom_tileset_size;
    void* rom_tileset_base;

    int rom_tilemap_size;
    void* rom_tilemap_base;

    int rom_cgram_size;
    void* rom_cgram_base;
};

struct World {
    WorldTileLayer  tile_layers[5];
    WorldStaticCollisionLayer collision_layers[4];

    int grid_node_size;
    int grid_width;
    int grid_height;
    
    WorldGridNode grid_nodes[64*64];
};

extern "C" void world_clear_entities(World * world, const vec2 & bounds_min, const vec2 & bounds_max);
extern "C" void world_insert_entity(World * world, const WorldEntity & entity, const vec2 & bounds_min, const vec2 & bounds_max);
extern "C" int world_get_entities(World * world, const vec2& bounds_min, const vec2& bounds_max, unsigned int layer_mask,  unsigned int type_mask, int max_entities, WorldEntity* out_entities);
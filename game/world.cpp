#include "world.h"

void world_clear_entities(World* world, const vec2& bounds_min, const vec2& bounds_max) {
    int node_x = int(bounds_min.x) >> world->grid_node_size;
    int node_y = int(bounds_min.y) >> world->grid_node_size;

    int node_end_x = (int(bounds_max.x) >> world->grid_node_size);
    int node_end_y = (int(bounds_max.y) >> world->grid_node_size);

    if (node_x < 0) node_x = 0;
    if (node_y < 0) node_y = 0;

    if (node_end_x < 0) node_end_x = 0;
    if (node_end_y < 0) node_end_y = 0;

    if (node_x >= world->grid_width) node_x = world->grid_width - 1;
    if (node_y >= world->grid_height) node_y = world->grid_height - 1;

    if (node_end_x >= world->grid_width) node_end_x = world->grid_width - 1;
    if (node_end_y >= world->grid_height) node_end_y = world->grid_height - 1;

    for (int y = node_y; y <= node_end_y; ++y) {
        for (int x = node_x; x <= node_end_x; ++x) {
            int i = y*world->grid_height + x;
            world->grid_nodes[i].num_entities = 0;
        }
    }
}

void world_insert_entity(World* world, const WorldEntity &entity, const vec2& bounds_min, const vec2& bounds_max) {
    int node_x = int(bounds_min.x) >> world->grid_node_size;
    int node_y = int(bounds_min.y) >> world->grid_node_size;

    int node_end_x = (int(bounds_max.x) >> world->grid_node_size);
    int node_end_y = (int(bounds_max.y) >> world->grid_node_size);

    if (node_x < 0) node_x = 0;
    if (node_y < 0) node_y = 0;

    if (node_end_x < 0) node_end_x = 0;
    if (node_end_y < 0) node_end_y = 0;

    if (node_x >= world->grid_width) node_x = world->grid_width - 1;
    if (node_y >= world->grid_height) node_y = world->grid_height - 1;

    if (node_end_x >= world->grid_width) node_end_x = world->grid_width - 1;
    if (node_end_y >= world->grid_height) node_end_y = world->grid_height - 1;

    for (int y = node_y; y <= node_end_y; ++y) {
        for (int x = node_x; x <= node_end_x; ++x) {
            int node_index = y*world->grid_height + x;
            bool found = false;
            WorldGridNode& node = world->grid_nodes[node_index];
            for (int e = 0; e < node.num_entities; ++e) {
                WorldEntity& we = node.entities[e];
                if (we.id == entity.id) {
                    found = true;
                    break;
                }
            }
            if (!found && node.num_entities < WORLD_NODE_MAX_ENTITIES) {
                node.entities[node.num_entities++] = entity;
            }
        }
    }

}



int world_get_entities(World* world, 
  const vec2& bounds_min, const vec2& bounds_max,
  unsigned int layer_mask,  unsigned int type_mask,
  int max_entities, WorldEntity* out_entities) {

    int node_x = int(bounds_min.x) >> world->grid_node_size;
    int node_y = int(bounds_min.y) >> world->grid_node_size;

    int node_end_x = (int(bounds_max.x) >> world->grid_node_size);
    int node_end_y = (int(bounds_max.y) >> world->grid_node_size);

    if (node_x < 0) node_x = 0;
    if (node_y < 0) node_y = 0;

    if (node_end_x < 0) node_end_x = 0;
    if (node_end_y < 0) node_end_y = 0;

    if (node_x >= world->grid_width) node_x = world->grid_width - 1;
    if (node_y >= world->grid_height) node_y = world->grid_height - 1;

    if (node_end_x >= world->grid_width) node_end_x = world->grid_width - 1;
    if (node_end_y >= world->grid_height) node_end_y = world->grid_height - 1;

    int num_found = 0;

    for (int y = node_y; y <= node_end_y; ++y) {
        for (int x = node_x; x <= node_end_x; ++x) {
            int node_index = y*world->grid_height + x;
            WorldGridNode& node = world->grid_nodes[node_index];
            for (int e0 = 0; e0 < node.num_entities; ++e0) {
                bool found = false;
                WorldEntity& we0 = node.entities[e0];
                if (!((1 << we0.layer) & layer_mask)) {
                    continue;
                }
                if (!((1 << we0.type) & type_mask)) {
                    continue;
                }
                for (int e = 0; e < num_found; ++e) {
                    WorldEntity& we = out_entities[e];
                    if (we.id == we0.id) {
                        found = true;
                        break;
                    }
                }
                if (!found && node.num_entities < max_entities) {
                    out_entities[num_found++] = we0;
                }
            }
            
        }
    }
    return num_found;
}
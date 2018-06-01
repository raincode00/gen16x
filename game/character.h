#include "physics.h"
#include "sprite.h"

struct Character {
    int entity_id;
    int sprite_id;
    union {
        unsigned int state;
        struct {
            unsigned int moving : 2;    // 0 - standing, 1 - walking, 2 - running, 3 - other
            unsigned int jumping : 1;
            unsigned int attacking : 1;
            unsigned int hurt : 1;
            unsigned int ability : 4;
            unsigned int talking : 1;
            unsigned int dialog : 1;
            unsigned int faction : 2;   // 0 - neutral, 1 - ally, 2 - enemy, 3 - other
        } state_flags;
    };
    unsigned int state_stack_offset;
    unsigned int state_stack[4];
    unsigned int next_state;
    vec2 position;
    vec2 look_dir;
    float walk_speed;
    float run_speed;
    float collision_radius;
};

unsigned int character_get_state(Character* character) {
    return character->state;
}


void character_set_state(Character* character, unsigned int state) {
    character->next_state = state;
}

void character_push_state(Character* character, unsigned int state) {
    if (character->state_stack_offset >= 4) {
        return;
    }
    character->state_stack[character->state_stack_offset++] = character->state;
    character->next_state = state;
}

void character_pop_state(Character* character) {
    if (character->state_stack_offset <= 0) {
        return;
    }
    character->next_state = character->state_stack[character->state_stack_offset--];
}

void character_update_state(Character* character) {
    
    character->state = character->next_state;
}
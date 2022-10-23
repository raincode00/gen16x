#include "character.h"

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
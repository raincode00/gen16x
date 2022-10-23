#include "gamestate.h"


void gamestate_init_scene(GameState* gamestate) {
    int ppu_vram_offset = 0;
    int ppu_cgram_offset = 0;
    if (gamestate->scene_load_handler) {
        gamestate->scene_load_handler(gamestate, &gamestate->scene, gamestate->scene_handler_param);
    }
    scene_load_ppu(&gamestate->scene, &gamestate->ppu, &ppu_vram_offset, &ppu_cgram_offset);
}

void gamestate_load_level(GameState* gamestate, int level_id) {
    gamestate->next_level_id = level_id;
}

void gamestate_update_scene(GameState* gamestate) {
    if (gamestate->scene_update_handler) {
        gamestate->scene_update_handler(gamestate, &gamestate->scene, gamestate->scene_handler_param);
    }
}

void gamestate_update_players(GameState* gamestate) {
    for (int i = 0; i < GAMESTATE_MAX_PLAYERS; i++) {
        CharacterController& player = gamestate->players[i];
        if (player.active && player.controller_handler) {
            player.controller_handler(gamestate, &player.character, player.controller_param);
        }
    }
}

void gamestate_update_npcs(GameState* gamestate) {
    for (int i = 0; i < GAMESTATE_MAX_NPCS; i++) {
        CharacterController& npc = gamestate->npcs[i];
        if (npc.active && npc.controller_handler) {
            npc.controller_handler(gamestate, &npc.character, npc.controller_param);
        }
    }
}

void gamestate_reset(GameState* gamestate) {
    memset(gamestate, 0, sizeof(GameState));
    gamestate->current_level_id = -1;
    gamestate->next_level_id = 0;
}

void gamestate_tick(GameState* gamestate, float delta_time) {
    gamestate->delta_time = delta_time;

    if (gamestate->next_level_id != -1 && gamestate->current_level_id != gamestate->next_level_id) {
        gamestate->current_level_id = gamestate->next_level_id;
        gamestate_init_scene(gamestate);
        gamestate->next_level_id = -1;
    }

    gamestate_update_scene(gamestate);
    gamestate_update_npcs(gamestate);
    gamestate_update_players(gamestate);

    scene_render_ppu(&gamestate->scene, &gamestate->ppu);

    gamestate->time += delta_time;
    gamestate->frame += 1;
}

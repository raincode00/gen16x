#pragma once

#include "gen16x.h"
#include "input.h"
#include "character.h"
#include "scene.h"

#define GAMESTATE_MAX_NPCS 64
#define GAMESTATE_MAX_PLAYERS 4
struct GameState;
struct PlayerController;

typedef void(*controller_handler_t)(GameState*, Character*, void*);
typedef void(*gamestate_scene_handler_t)(GameState*, Scene*, void*);

struct CharacterController {
    bool active;
    Character character;
    controller_handler_t controller_handler;
    void* controller_param;
};

struct GameState {
    unsigned int frame;
    double time;
    float delta_time;
    int current_level_id;
    int next_level_id;
    Scene scene;
    void* scene_handler_param;
    gamestate_scene_handler_t scene_load_handler;
    gamestate_scene_handler_t scene_update_handler;
    PlayerInput player_inputs[GAMESTATE_MAX_PLAYERS];
    CharacterController players[GAMESTATE_MAX_PLAYERS];
    CharacterController npcs[GAMESTATE_MAX_NPCS];
    gen16x_ppu ppu;
};

extern "C" void gamestate_init_scene(GameState* gamestate);
extern "C" void gamestate_load_level(GameState* gamestate, int level_id);
extern "C" void gamestate_update_scene(GameState* gamestate); 
extern "C" void gamestate_update_players(GameState* gamestate);
extern "C" void gamestate_update_npcs(GameState* gamestate); 
extern "C" void gamestate_reset(GameState* gamestate);
extern "C" void gamestate_tick(GameState* gamestate, float delta_time);
#include <conio.h>

#include "globals.h"
#include "graphics.h"
#include "idle.h"
#include "input.h"
#include "player.h"
#include "map.h"
#include "snore.h"
#include "sound.h"
#include "sprite.h"
#include "ui.h"
#include "utils.h"

// This forces the entry point to live in the space BEFORE the graphics
#pragma code(code)

static unsigned char game_mode = 0xFF;

void reset(void) {
    //clrscr();
    clear_to_black();
    
    init_graphics();
    init_charset();

    // initialize the game state
    init_dungeon_registry();
    init_player();
    init_player_position();  // Find Home and start below it
    idle_init();
    game_mode = 0xFF;

    // begin the game in the overworld
    switch_to_overworld();    
}

int main() {
    // initialize the game
    init_sid_enhanced();
    init_random();
    
    // start the game
    reset();    
    while (1) {     // NOTE: should we have a "QUIT" key to exit the game loop??
        wait_vblank();

        // game mode switched
        if (game_mode != get_game_mode()) {
            game_mode = get_game_mode();

            if (game_mode == MODE_MAP) {
                // transition to the overworld map
                clear_to_black();
                init_graphics();
                idle_reset();                
                init_player_sprite();
                draw_world();
                draw_hud();
                update_player_sprite_pos();
                update_nearby_dungeons(player_get_x(), player_get_y());
            }
            else {
                // play a transition sound
                wait_vblank();

                // transition to the fog of war
                clear_to_black();

                draw_hud();
                continue;
            }
        }

        // per-frame updates
        if (game_mode == MODE_MAP) {
            ui_update();
            snore_update();
            idle_update();
            //draw_hud();

            unsigned char handled = handle_map_input();
            if (handled) {
                switch(handled) {
                    case RESET_GAME:
                        reset();
                        continue;
                    default:
                        // TODO: we may want to handle the gameplay logic here, or at least call to it
                        // reset the idle state after a player action
                        idle_reset();
                        break;
                }                                
            }

            // pulse the player icon (e.g. heartbeat)
            handle_player_pulse();
        }
        else {
            // MODE_FOG
            //draw_hud();

            unsigned char handled = handle_map_input();
            if (handled) {
                /*
                switch(handled) {
                    case RESET_GAME:
                        reset();
                        continue;
                    default:                        
                        break;
                } 
                */                               
            }
        }
    }
    return 0;
}
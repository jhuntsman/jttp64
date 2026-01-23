#include <conio.h>
#include <c64/vic.h>

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
#include "dungeon.h"

// This forces the entry point to live in the space BEFORE the graphics
#pragma code(code)

/* Debug helper removed: normal game loop restored. */

static unsigned char game_mode = 0xFF;

void reset(void) {
    //clrscr();
    clear_to_black();
    
    init_map_graphics();
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
    // normal execution continues
    while (1) {     // NOTE: should we have a "QUIT" key to exit the game loop??
        wait_vblank();
          
        // game mode switched
        if (game_mode != get_game_mode()) {
            game_mode = get_game_mode();

            if (game_mode == MODE_MAP) {
                // transition to the overworld map
                clear_to_black();
                
                init_map_graphics();
                idle_reset();                
                init_player_sprite();
                draw_world();
                draw_hud();
                update_map_player_sprite_pos();
                update_nearby_dungeons(player_get_x(), player_get_y());
            }
            else {
                // play a transition sound
                //wait_vblank();

                // transition to the fog of war
                clear_to_black();
                init_fog_graphics();
                init_charset();
                init_dungeon();
                render_dungeon();                
                // Initialize and position the player sprite for the dungeon view
                //init_player_sprite();
                update_fog_player_sprite_pos();

                //draw_hud();
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
            // MODE_FOG - Dungeon viewport scrolling
            unsigned char handled = handle_fog_input();
            if(handled) {
                // Input has updated camera position and viewport scroll state
                // Render_dungeon will handle full screen redraws when needed
                render_dungeon();
                update_fog_player_sprite_pos();
            }      
            
        }
    }
    return 0;
}
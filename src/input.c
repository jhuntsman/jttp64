#include <c64/cia.h>
#include <c64/vic.h>
#include <conio.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "assets.h"
#include "graphics.h"
#include "sound.h"
#include "input.h"
#include "player.h"
#include "ui.h"
#include "sprite.h"

// Handle joystick input, repeat logic, and movement
unsigned char handle_map_input(void) {
    const unsigned char joy_current = ~cia1.prb & 0x1F;
    static unsigned char last_joy = 0;
    static unsigned char repeat_timer = 0;

    int px = player_get_x();
    int py = player_get_y();
    int nx = px;
    int ny = py;
    int dx = 0;
    int dy = 0;
    unsigned char moved = 0;

     /* handle C64 keyboard navigation: only check when no joystick input is present
         so emulator keyboard->joystick mappings aren't consumed. Support WASD
         + Q/E/Z/C for diagonals. Case-insensitive. Number keys removed for later use. */
    /* keyboard: set dx/dy when no joystick input present so emulator mappings
       aren't consumed. WASD + Q/E/Z/C for diagonals. */
    if (joy_current == 0 && kbhit()) {
        char ch = getch();
        char cl = tolower((unsigned char)ch);

        switch (cl) {
            case '0': return RESET_GAME;  /* reset game */           
            case 'x':
                // exit fog of war/dungeon mode for testing purposes
                if (get_game_mode() == MODE_FOG) {
                    clear_dungeon(get_current_dungeon());
                    switch_to_overworld();
                }
                return 1;
                break;
            case 'q': dx = -1; dy = -1; break; /* up-left */
            case 'w': dx = 0; dy = -1; break;  /* up */
            case 'e': dx = 1; dy = -1; break;  /* up-right */
            case 'a': dx = -1; dy = 0; break;  /* left */
            case 'd': dx = 1; dy = 0; break;   /* right */
            case 'z': dx = -1; dy = 1; break;  /* down-left */
            case 's': dx = 0; dy = 1; break;   /* down */
            case 'c': dx = 1; dy = 1; break;   /* down-right */
            default: dx = 0; dy = 0; break;
        }

        if (dx != 0 || dy != 0) {
            moved = 1;
            repeat_timer = MOVE_DELAY_INITIAL;
        }
    }

    const unsigned char joy_pressed = joy_current & ~last_joy;

    if (joy_pressed) {
        moved = 1;
        repeat_timer = MOVE_DELAY_INITIAL;
    } else if (joy_current) {
        if (repeat_timer == 0) {
            moved = 1;
            repeat_timer = MOVE_DELAY_REPEAT;
        } else {
            repeat_timer--;
        }
    } else {
        repeat_timer = 0;
    }

    last_joy = joy_current;

    if (moved) {
        /* if joystick is active, override dx/dy from key handling */
        if (joy_current) {
            dx = 0; dy = 0;
            if (joy_current & 0x01) dy = -1; /* up */
            if (joy_current & 0x02) dy = 1;  /* down */
            if (joy_current & 0x04) dx = -1; /* left */
            if (joy_current & 0x08) dx = 1;  /* right */
        }

        nx = px + dx;
        ny = py + dy;

        if (nx != px || ny != py) {
            /* bounds check */
            if (nx < 0 || nx >= MAP_W || ny < 0 || ny >= MAP_H) {
                play_thud();
                ui_trigger_border(BORDER_DURATION, COLOR_RED);
                return 1;
            }

            const unsigned char target_tile = map_data[ny * MAP_W + nx];

            if (is_walkable(nx, ny, target_tile)) {
                if(target_tile == TILE_DUNGEON_UNKNOWN) {
                    uint8_t dungeon_index = can_enter_dungeon(nx, ny);
                    if (dungeon_index != NOT_FOUND) {
                        switch_to_fog_of_war(dungeon_index);                        
                    }
                }                
                else {
                    // do not reveal nearby dungeons until after the dungeon is cleared
                    update_nearby_dungeons(nx, ny);
                }

                // move the player sprite to the new position on the map
                player_set_pos(nx, ny);
                ((unsigned char*)SPRITE_PTR)[0] = SPRITE_BLOCK;
                wait_vblank();
                update_player_sprite_pos();                
                return 1;
            } else {
                play_thud();
                ui_trigger_border(BORDER_DURATION, COLOR_RED);
                return 1;
            }
        }
    }
    return 0;
}

#include <c64/cia.h>
#include <c64/vic.h>
#include <conio.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "assets.h"
#include "graphics.h"
#include "sound.h"
#include "snore.h"
#include "input.h"
#include "player.h"
#include "ui.h"
#include "sprite.h"
#include "dungeon.h"

// Handle joystick input, repeat logic, and movement for the overworld map
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

    if (kbhit()) {
        char ch = getch();
        char cl = tolower((unsigned char)ch);

        switch (cl) {
            case '0':
                return RESET_GAME;  /* reset game */
            default: break;
        }

        /* Movement keys honored when no joystick input is present */
        if (joy_current == 0) {
            switch (cl) {
                case 'q': dx = -1; dy = -1; break;
                case 'w': dx = 0; dy = -1; break;
                case 'e': dx = 1; dy = -1; break;
                case 'a': dx = -1; dy = 0; break;
                case 'd': dx = 1; dy = 0; break;
                case 'z': dx = -1; dy = 1; break;
                case 's': dx = 0; dy = 1; break;
                case 'c': dx = 1; dy = 1; break;
                default: dx = 0; dy = 0; break;
            }

            if (dx != 0 || dy != 0) {
                moved = 1;
                repeat_timer = MOVE_DELAY_INITIAL;
            }
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
            /* overworld bounds */
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
                        return 1;
                    }
                } else {
                    update_nearby_dungeons(nx, ny);
                }

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

// Fog-of-war specific input handling: captures test key 'x' to exit fog and
// handles movement against `live_dungeon_map` (0..63 coords).
unsigned char handle_fog_input(void) {
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

    if (kbhit()) {
        char ch = getch();
        char cl = tolower((unsigned char)ch);

        // Allow 'x' to exit fog for testing
        if (cl == 'x') {
            clear_dungeon(get_current_dungeon());
            player_add_xp(50);
            switch_to_overworld();
            return 1;
        }

        /* Movement keys when no joystick input present */
        if (joy_current == 0) {
            switch (cl) {
                case 'q': dx = -1; dy = -1; break;
                case 'w': dx = 0; dy = -1; break;
                case 'e': dx = 1; dy = -1; break;
                case 'a': dx = -1; dy = 0; break;
                case 'd': dx = 1; dy = 0; break;
                case 'z': dx = -1; dy = 1; break;
                case 's': dx = 0; dy = 1; break;
                case 'c': dx = 1; dy = 1; break;
                default: dx = 0; dy = 0; break;
            }

            if (dx != 0 || dy != 0) {
                moved = 1;
                repeat_timer = MOVE_DELAY_INITIAL;
            }
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
        if (joy_current) {
            dx = 0; dy = 0;
            if (joy_current & 0x01) dy = -1;
            if (joy_current & 0x02) dy = 1;
            if (joy_current & 0x04) dx = -1;
            if (joy_current & 0x08) dx = 1;
        }

        nx = px + dx;
        ny = py + dy;

        if (nx != px || ny != py) {
            
            // TODO: dungeon movement
            
        }
    }
    return 0;
}

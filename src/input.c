#include <c64/cia.h>
#include <c64/vic.h>
#include <conio.h>
#include <string.h>

#include "assets.h"
#include "graphics.h"
#include "sound.h"
#include "input.h"
#include "player.h"
#include "ui.h"
#include "sprite.h"

// Handle joystick input, repeat logic, and movement
unsigned char handle_input(void) {
    const unsigned char joy_current = ~cia1.prb & 0x1F;
    static unsigned char last_joy = 0;
    static unsigned char repeat_timer = 0;

    unsigned char nx = player_get_x();
    unsigned char ny = player_get_y();
    unsigned char moved = 0;

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
        if ((joy_current & 0x01) && player_get_y() > 0) ny--;
        if ((joy_current & 0x02) && player_get_y() < MAP_H - 1) ny++;
        if ((joy_current & 0x04) && player_get_x() > 0) nx--;
        if ((joy_current & 0x08) && player_get_x() < MAP_W - 1) nx++;

        if (nx != player_get_x() || ny != player_get_y()) {
            const unsigned char target_tile = map_data[ny * MAP_W + nx];

            if (is_walkable(target_tile)) {
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

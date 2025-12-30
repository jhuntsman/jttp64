#include <conio.h>
#include <stdlib.h>
#include <c64/sid.h>
#include <c64/vic.h>
#include "globals.h"
#include "graphics.h"
#include "sound.h"
#include "map.h"
#include "ui.h"
#include "sprite.h"
#include "input.h"
#include "idle.h"
#include "snore.h"

int main() {
    init_graphics();
    init_charset();

    clrscr();
    init_sid_enhanced();
    idle_init();
    init_player_position();  // Find Home and start below it
    draw_world();
    draw_hud();
    init_player_sprite();
    update_player_sprite_pos();

    while (1) {
        wait_vblank();

        // per-frame updates
        ui_update();
        snore_update();
        idle_update();

        if (handle_input()) {
            idle_reset();
        }

        handle_player_pulse();
    }
    return 0;
}
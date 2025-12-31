#include <c64/sid.h>
#include <c64/vic.h>
#include <conio.h>
#include <player.h>
#include <stdlib.h>

#include "globals.h"
#include "graphics.h"
#include "idle.h"
#include "input.h"
#include "map.h"
#include "snore.h"
#include "sound.h"
#include "sprite.h"
#include "ui.h"

int main() {
    // initialize the game
    init_sid_enhanced();
    init_graphics();
    init_charset();
    clrscr();

    // initialize the game state
    init_player();
    init_player_position();  // Find Home and start below it
    idle_init();

    // the overworld map
    // TODO: this will need to occur whenever the gameplay switches from fog of war
    init_player_sprite();
    draw_world();
    draw_hud();
    update_player_sprite_pos();

    while (1) {     // NOTE: should we have a "QUIT" key to exit the game loop??
        wait_vblank();

        // per-frame updates
        ui_update();
        snore_update();
        idle_update();

        if (handle_input()) {
            // TODO: we may want to handle the gameplay logic here, or at least call to it

            // reset the idle state after a player action
            idle_reset();
        }

        // pulse the player icon (e.g. heartbeat)
        handle_player_pulse();
    }
    return 0;
}
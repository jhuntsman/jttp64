#include "player.h"

static unsigned char p_x = 0;
static unsigned char p_y = 0;

void player_set_pos(unsigned char x, unsigned char y) {
    p_x = x;
    p_y = y;
}

unsigned char player_get_x(void) {
    return p_x;
}

unsigned char player_get_y(void) {
    return p_y;
}

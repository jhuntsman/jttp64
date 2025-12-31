#include "player.h"

// Player Position
static unsigned char p_x = 0;
static unsigned char p_y = 0;

// Player Stats
static unsigned char player_lives = 0;
static unsigned char player_health = 0;
static unsigned char player_level = 0;
static unsigned char player_arrows = 0;

// Player Inventory Flags
static unsigned char player_axe = 0;
static unsigned char player_key = 0;
static unsigned char player_boat = 0;
static unsigned char player_crown_1 = 0;
static unsigned char player_crown_2 = 0;


void init_player(void) {
    // starting lives
    player_lives = 3;

    // starting level
    player_level = 1;

    // starting health
    player_health = 3;

    // initial inventory
    player_arrows = 3;
    player_axe = 0;
    player_key = 0;
    player_boat = 0;
    player_crown_1 = 0;
    player_crown_2 = 0;
}

void reset_player(void) {
    // reset after death
    player_health = 3;

    // arrows are lost when the player dies
    player_arrows = 3;

    // other inventory items remain
}

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

unsigned char player_get_lives(void) {
    return player_lives;
}

unsigned char player_get_health(void) {
    return player_health;
}

unsigned char player_get_level(void) {
    return player_level;
}

unsigned char player_get_arrows(void) {
    return player_arrows;
}

unsigned player_has_axe(void) {
    return player_axe;
}

unsigned player_has_key(void) {
    return player_key;
}

unsigned player_has_boat(void) {
    return player_boat;
}

unsigned player_has_crown_1(void) {
    return player_crown_1;
}

unsigned player_has_crown_2(void) {
    return player_crown_2;
}

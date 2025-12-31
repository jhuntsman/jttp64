// player.h -- player position API
#ifndef JTTP_PLAYER_H
#define JTTP_PLAYER_H

void init_player(void);
void reset_player(void);

void player_set_pos(unsigned char x, unsigned char y);
unsigned char player_get_x(void);
unsigned char player_get_y(void);
unsigned char player_get_lives(void);
unsigned char player_get_health(void);
unsigned char player_get_level(void);
unsigned char player_get_arrows(void);
unsigned player_has_axe(void);
unsigned player_has_key(void);
unsigned player_has_boat(void);
unsigned player_has_crown_1(void);
unsigned player_has_crown_2(void);

#endif // JTTP_PLAYER_H

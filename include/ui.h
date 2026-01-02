// ui.h -- HUD / UI helpers
#ifndef JTTP_UI_H
#define JTTP_UI_H

#define HUD_CHARSET_OFFSET 128
#define HUD_START 880
#define SOLID_BLOCK 29
#define COLOR_HUD COLOR_BLACK

void draw_hud(void);
void hud_update(void);

// Border helper: trigger a temporary border color for `duration` frames
void ui_trigger_border(unsigned char duration, unsigned char color);

// Called each frame to update UI timers (border fade, etc.)
void ui_update(void);
void handle_snore_timing();

#endif // JTTP_UI_H

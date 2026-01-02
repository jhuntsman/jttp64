#include <string.h>
#include <conio.h>
#include <stdint.h>
#include "globals.h"

const uint8_t colr_vic_bg0 = COLOR_LT_GREEN; // Light Green

// --- Place pixels at $2000 ---
#pragma data(charset)
const uint8_t charset_data[2048] = { 
    #embed "assets/charset.bin" 
};

// --- Place tileset and map at $2800 ---
#pragma data(metadata)
const uint8_t chartileset_data[256] = { 
    #embed "assets/tileset.bin"     
};
const uint8_t chartileset_attrib_L1_data[64] = { 
    #embed "assets/tileset_attr_L1.bin"     
};
const uint8_t map_data[220] = { 
    #embed "assets/overworld.bin" 
};


#pragma data(data) // Reset back to normal program space
const uint8_t dungeon_room_data[1701] = {
    #embed "assets/dungeon.bin"
};
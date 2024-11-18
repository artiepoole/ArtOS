#ifndef COLOURS_H
#define COLOURS_H


// Solarised colours definitions
enum PALETTE_t
{
    COLOR_BASE03 = 0x002b36, // Darkest (near black, bluey grey)
    COLOR_BASE02 = 0x073642,
    COLOR_BASE01 = 0x586e75,
    COLOR_BASE00 = 0x657b83,
    COLOR_BASE0 = 0x839496,
    COLOR_BASE1 = 0x93a1a1,
    COLOR_BASE2 = 0xeee8d5,
    COLOR_BASE3 = 0xfdf6e3, // Lightest (cream)
    COLOR_YELLOW = 0xb58900,
    COLOR_ORANGE = 0xcb4b16,
    COLOR_RED = 0xdc322f,
    COLOR_MAGENTA = 0xd33682,
    COLOR_VIOLET = 0x6c71c4,
    COLOR_BLUE = 0x268bd2,
    COLOR_CYAN = 0x2aa198,
    COLOR_GREEN = 0x859900,
};

inline PALETTE_t colour_bkgd = COLOR_BASE03;
inline PALETTE_t colour_frgd = COLOR_BASE0;
inline PALETTE_t colour_accent = COLOR_CYAN;
inline PALETTE_t colour_value = COLOR_MAGENTA;
inline PALETTE_t colour_error = COLOR_RED;

#endif

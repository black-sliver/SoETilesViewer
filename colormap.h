#ifndef COLORMAP_H
#define COLORMAP_H

#include <QRgb>
#include <stdint.h>

struct ColorMap;

struct ColorMap {
    QRgb c[16];

    static ColorMap FromSnes(uint16_t snescolors[16], bool transparent=true) {
        ColorMap map;
        for (uint8_t i=0; i<16; i++) {
            uint16_t c16=snescolors[i];
            uint32_t r = ((c16>> 0) & 0x1f)*8; r += r>>5;
            uint32_t g = ((c16>> 5) & 0x1f)*8; g += g>>5;
            uint32_t b = ((c16>>10) & 0x1f)*8; b += b>>5;
            map.c[i] = (0xff<<24) | (r<<16) | (g<<8) | b;
        }
        if (transparent) map.c[0] = 0;
        return map;
    };
};

#endif // COLORMAP_H

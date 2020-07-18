#ifndef COLORMAP_H
#define COLORMAP_H

#include <QRgb>
#include <QVector>
#include <stdint.h>
#include <string.h> // memcmp

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
    ColorMap() { c[0]=0; for (uint8_t i=1; i<16; i++) c[i]=0xff000000; }

    uint8_t find(QRgb color) const
    {
        for (uint8_t i=0; i<16; i++)
            if (c[i] == color || ( (color>>24)==0 && (c[i]>>24)==0) )
                return i;
        return (uint8_t)-1; // invalid
    }
    bool operator==(const ColorMap& other) const
    {
        return memcmp(c, other.c, sizeof(c)) == 0;
    }
    bool operator!=(const ColorMap& other) const
    {
        return !(*this == other);
    }
    QVector<QRgb> toQVector() const
    {
        QVector<QRgb> vec;
        for (uint8_t i=0; i<16; i++) vec.push_back(c[i]);
        return vec;
    }
};

#endif // COLORMAP_H

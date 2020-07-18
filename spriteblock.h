#ifndef SPRITEBLOCK_H
#define SPRITEBLOCK_H


#include "rom.h"
#include <QByteArray>
#include <string.h>


struct SpriteBlock {
    int i;
    unsigned ptraddr;
    unsigned dataaddr;
    bool compressed;
    uint8_t data[128+128/8]; // this is the maximum amount a block can use in theory
    unsigned romsize;
    QByteArray pixels;

    QString toString()
    {
        return QStringLiteral("#%1 -> $%2 -> $%3 %4B ").arg(i, 4, 10).arg(ptraddr, 0, 16).arg(dataaddr, 0, 16).arg(romsize, 3, 10) + (compressed ? "compressed" : "uncompressed");
    }

    const QByteArray& getPixels() const
    {
        return pixels;
    }
    bool setPixels(const QByteArray& newpixels)
    {
        uint8_t* d = NULL; // uncompressed snes-formatted data
        uint8_t uncompressedData[128];
        if (compressed) {
            d = uncompressedData;
        } else {
            d = data;
        }
        memset(d, 0, 128);

        int n=0;
        for (size_t l=0; l<2; l++) { // 2 8x8 blocks in Y
            for (size_t k=0; k<8; k++) { // 8 rows
                for (size_t j=0; j<2; j++) { // 2 8x8 blocks in X
                    // 8 columns
                    for (int i=7; i>=0; i--) {
                        uint8_t p = newpixels[n];
                        if (p&1) d[(j+2*l)*32+k*2+0]  |= (1<<i);
                        if (p&2) d[(j+2*l)*32+k*2+1]  |= (1<<i);
                        if (p&4) d[(j+2*l)*32+k*2+16] |= (1<<i);
                        if (p&8) d[(j+2*l)*32+k*2+17] |= (1<<i);
                        n++;
                    }
                }
            }
        }
        if (compressed) {
            QByteArray compressedData;
            for (size_t i=0; i<128/16; i++) {
                uint8_t b=0;
                for (size_t j=0; j<8; j++)
                    if (!(d[i*16+2*j] || d[i*16+2*j+1])) b|= (1<<j);
                compressedData.append(b);
                for (size_t j=0; j<8; j++)
                    if (d[i*16+2*j] || d[i*16+2*j+1]) { compressedData.append(d[i*16+2*j]); compressedData.append(d[i*16+2*j+1]); }
            }
            if ((unsigned)compressedData.size() > romsize) return false;
            memset(data, 0, romsize); // clear free data in case we shrink it
            memcpy(data, compressedData.data(), compressedData.size());
        }
#ifdef TEST
        pixels = loadPixels(data, compressed, NULL);
        qDebug("size = %d vs %d\n", pixels.size(), newpixels.size());
        qDebug("diff = %d: \n", memcmp(pixels.data(), newpixels.data(), pixels.size()));
        assert(pixels.size() == newpixels.size() && memcmp(pixels.data(), newpixels.data(), pixels.size()) == 0);
#else
        pixels = newpixels;
#endif
        return true;
    }

private:
    static QByteArray loadPixels(const uint8_t* data, bool compressed, const uint8_t** next=NULL)
    {
        QByteArray res;
        const uint8_t* d = NULL;
        uint8_t uncompressed[128];
        if (compressed) {
            memset(uncompressed, 0, sizeof(uncompressed));
            const uint8_t* din = data;
            uint8_t* dout = uncompressed;
            for (size_t i=0; i<128/2/8; i++) { // 1 bit per output word
                uint8_t bits = *din; din++;
                for (uint8_t bp=0; bp<8; bp++) {
                    uint8_t bit = bits&1; bits>>=1;
                    if (!bit) { // copy word as-is
                        *dout = *din; din++; dout++;
                        *dout = *din; din++; dout++;
                    } else { // word is zero
                        dout += 2;
                    }
                }
            }
            d = uncompressed;
            if (next) *next = din;
        }
        else {
            d = data;
            if (next) *next = data+128;
        }
        for (size_t l=0; l<2; l++) { // 2 8x8 blocks in Y
            for (size_t k=0; k<8; k++) { // 8 rows
                for (size_t j=0; j<2; j++) { // 2 8x8 blocks in X
                    // 8 columns
                    for (int i=7; i>=0; i--) {
                        uint8_t p=0;
                        p |= (d[(j+2*l)*32+k*2+0]&(1<<i)) ? 1 : 0;
                        p |= (d[(j+2*l)*32+k*2+1]&(1<<i)) ? 2 : 0;
                        p |= (d[(j+2*l)*32+k*2+16]&(1<<i)) ? 4 : 0;
                        p |= (d[(j+2*l)*32+k*2+17]&(1<<i)) ? 8 : 0;
                        res.append(p);
                    }
                }
            }
        }
        return res;
    }

public:
    SpriteBlock(int i, Rom* rom) {
        this->i = i;
        ptraddr = 0xec0000 + i*3;
        dataaddr = rom->read24(ptraddr);
        compressed = (dataaddr>>23);
        dataaddr &= ~(1<<23);
        dataaddr += 0xd90000;
        romsize = 0;
        if (compressed) rom->readBlock(this->dataaddr, data, sizeof(data));
        else rom->readBlock(dataaddr, data, 128);
        const uint8_t* next = data;
        pixels = loadPixels(data, compressed, &next);
        romsize = next-data;
    }
    bool save(Rom* rom) {
        return rom->writeBlock(dataaddr, data, romsize);
    }
};


#endif // SPRITEBLOCK_H

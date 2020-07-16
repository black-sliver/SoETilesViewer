#ifndef SPRITEBLOCK_H
#define SPRITEBLOCK_H


#include "rom.h"
#include <QByteArray>


struct SpriteBlock {
    int i;
    unsigned ptraddr;
    unsigned dataaddr;
    bool compressed;
    uint8_t data[128+128/8]; // this is the maximum amount a block can use in theory
    QByteArray pixels;

    QString toString()
    {
        return QStringLiteral("#%1 -> $%2 -> $%3 ").arg(i, 3, 10).arg(ptraddr, 0, 16).arg(dataaddr, 0, 16) + (compressed ? "compressed" : "uncompressed");
    }

    const QByteArray& getPixels() const
    {
        return pixels;
    }

private:
    QByteArray loadPixels() const
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
        }
        else {
            d = data;
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
        this->ptraddr = 0xec0000 + i*3;
        this->dataaddr = rom->read24(this->ptraddr);
        this->compressed = (dataaddr>>23);
        this->dataaddr &= ~(1<<23);
        this->dataaddr += 0xd90000;
        if (compressed) rom->readBlock(this->dataaddr, data, sizeof(data));
        else rom->readBlock(this->dataaddr, data, 128);
        pixels = loadPixels();
    }
};


#endif // SPRITEBLOCK_H

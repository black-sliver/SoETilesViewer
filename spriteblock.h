#ifndef SPRITEBLOCK_H
#define SPRITEBLOCK_H


#include "rom.h"
#include <QByteArray>
#include <string.h>


struct SpriteBlock {
    int i;
    int size;

    unsigned ptraddr;
    unsigned dataaddr;
    bool compressed;
    uint8_t data[128+128/8]; // this is the maximum amount a block can use in theory
    unsigned romsize;
    QByteArray pixels;

    QString toString() // TODO: remove qt dependency
    {
        return QStringLiteral("#%1 -> $%2 -> $%3 %4B ").arg(i, 4, 10).arg(ptraddr, 0, 16).arg(dataaddr, 0, 16).arg(romsize, 3, 10) + (compressed ? "compressed" : "uncompressed");
    }

    const QByteArray& getPixels() const
    {
        return pixels;
    }
    bool setPixels(const QByteArray& newpixels)
    {
        assert(size==8 || size==16); // TODO: implement 8x8 blocks
        size_t subwidth = size/8;
        size_t subblocks = subwidth*subwidth;
        size_t uncompressedSize = 8*8*subblocks/2;

        uint8_t* d = NULL; // uncompressed snes-formatted data
        uint8_t* uncompressedData = new uint8_t[uncompressedSize]; // 4bpp
        if (compressed) {
            d = uncompressedData;
        } else {
            d = data;
        }
        memset(d, 0, uncompressedSize);

        int n=0;
        for (size_t l=0; l<subwidth; l++) { // 1 or 2 8x8 blocks in Y
            for (size_t k=0; k<8; k++) { // 8 rows
                for (size_t j=0; j<subwidth; j++) { // 1 or 2 8x8 blocks in X
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
            int extraWords = 0;
            do {
                compressedData.clear();
                for (size_t i=0; i<uncompressedSize/16; i++) {
                    uint8_t b=0;
                    for (size_t j=0; j<8; j++)
                        if (!(d[i*16+2*j] || d[i*16+2*j+1])) b |= (1<<j); // 1 status bit per word of data, 1=compress-away
                    for (size_t j=0; j<8; j++)
                        if (extraWords>0 && (b & (1<<j))) { b &= ~(1<<j); extraWords--; } // don't compress away to fill original size
                    compressedData.append(b); // append 8 status bits as byte
                    for (size_t j=0; j<8; j++) // append words that are not compressed away
                        if (!(b & (1<<j))) { compressedData.append(d[i*16+2*j]); compressedData.append(d[i*16+2*j+1]); }
                }
                extraWords = ((int)romsize - (int)compressedData.size())/2; // additional words to NOT compress-away to fill original size
                qDebug("Compression pass finished with extrawords = %d\n", extraWords);
            } while (extraWords > 0);
            if ((unsigned)compressedData.size() > romsize) return false; // won't fit
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
        delete[] uncompressedData;
        return true;
    }

private:
    static QByteArray loadPixels(const uint8_t* data, bool compressed, const uint8_t** next=NULL, int size=16)
    {
        assert(size == 8 || size == 16);
        size_t subwidth = size/8;
        size_t subblocks = subwidth*subwidth;
        size_t uncompressedSize = 8*8*subblocks/2;

        QByteArray res;
        const uint8_t* d = NULL;
        uint8_t* uncompressed = new uint8_t[uncompressedSize]; // 4bpp
        if (compressed) {
            memset(uncompressed, 0, uncompressedSize);
            const uint8_t* din = data;
            uint8_t* dout = uncompressed;
            for (size_t i=0; i<uncompressedSize/2/8; i++) { // 1 bit per output word
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
            if (next) *next = data+uncompressedSize;
        }
        for (size_t l=0; l<subwidth; l++) { // 1 or 2 8x8 blocks in Y
            for (size_t k=0; k<8; k++) { // 8 rows
                for (size_t j=0; j<subwidth; j++) { // 1 or 2 8x8 blocks in X
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
        delete[] uncompressed;
        return res;
    }

public:
    SpriteBlock(int i, Rom* rom, int size=16) {
        this->i = i;
        this->size = size;
        if (size==16) { // 16x16 blocks
            ptraddr = 0xec0000 + i*3;
            dataaddr = rom->read24(ptraddr);
            compressed = (dataaddr>>23);
            dataaddr &= ~(1<<23);
            dataaddr += 0xd90000;
        } else if (size==8) { // 8x8 blocks
            ptraddr = 0xd80000 + i*3;
            dataaddr = rom->read24(ptraddr);
            compressed = (dataaddr>>23);
            dataaddr &= ~(1<<23);
            dataaddr += 0xd10000;
        } else {
            assert(false);
        }
        romsize = 0;
        if (compressed) rom->readBlock(this->dataaddr, data, size==16 ? sizeof(data) : sizeof(data)/4);
        else rom->readBlock(dataaddr, data, size==16?128:128/4);
        const uint8_t* next = data;
        pixels = loadPixels(data, compressed, &next, size);
        romsize = next-data;
    }
    bool save(Rom* rom) {
        return rom->writeBlock(dataaddr, data, romsize);
    }
};


#endif // SPRITEBLOCK_H

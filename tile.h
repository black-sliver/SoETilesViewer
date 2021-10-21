#ifndef TILE_H
#define TILE_H


#include "rom.h"
#include "abstracttile.h"
#include <QByteArray>
#include <string.h>
#include <assert.h>


struct Tile : public AbstractTile
{

    virtual ~Tile() {}
    virtual bool setPixels(const QByteArray&) { return false; }

private:
    QByteArray loadPixels(Rom* rom, int size=16)
    {
        assert(size == 16);
        size_t subwidth = size/8;
        size_t subblocks = subwidth*subwidth;
        size_t uncompressedSize = 8*8*subblocks/2;

        QByteArray res;
        uint8_t* decompressed = nullptr;
        const uint8_t* d = data + 1;

        uint8_t tileInfo = rom->read8(dataaddr);
        compressed = tileInfo&0x80;

        if (!compressed)
        {
            uint8_t wordCount = (tileInfo&0x7f) + 1;
            unsigned dataPtr = dataaddr + 1;
            if (wordCount > uncompressedSize/2) {
                qWarning("Tile %d: too much data for 16x16", i);
                //wordCount -= 64; // maybe bit 6 is not part of size?
                wordCount = 64; // or maybe it's just junk
            }
            romsize = 1+wordCount*2;
            rom->readBlock(dataPtr-1, data, romsize); // copy words
            for (unsigned i=romsize; i<uncompressedSize+1; i+=2) {
                data[i+0] = data[i-2]; // repeat last word until full
                data[i+1] = data[i-1];
            }
            if (sizeof(data)>uncompressedSize+1) memset(data+uncompressedSize+1, 0, sizeof(data)-1-uncompressedSize);
        }
        else
        {
            decompressed = new uint8_t[uncompressedSize];
            d = decompressed;
            uint8_t dataOffset = tileInfo&0x7f;
            unsigned dataPtr = dataaddr + dataOffset;
            unsigned cmdPtr = dataaddr + 1;
            bool cmdSecondHalf = false;
            unsigned outPos = 0;

            auto read4cmdBits = [rom,&cmdPtr,&cmdSecondHalf]() {
                uint8_t res = rom->read8(cmdPtr);
                if (cmdSecondHalf) res &= 0x0f;
                else res >>= 4;
                if (cmdSecondHalf) cmdPtr++;
                cmdSecondHalf = !cmdSecondHalf;
                return res;
            };

            while (true)
            {
                uint8_t compressionIndicators = rom->read8(dataPtr++);
                for (uint8_t i=0; i<8; i++) {
                    if (!(compressionIndicators&0x80)) {
                        // uncompressed word
                        decompressed[outPos++] = rom->read8(dataPtr++);
                        decompressed[outPos++] = rom->read8(dataPtr++);
                    } else {
                        // compressed group
                        uint8_t mode = read4cmdBits(); // 4 bits compression mode
                        switch (mode)
                        {
                        case  0: // write 0x0000
                            decompressed[outPos++] = 0x00;
                            decompressed[outPos++] = 0x00;
                            break;
                        case  1: // write 0x00ff
                            decompressed[outPos++] = 0xff;
                            decompressed[outPos++] = 0x00;
                            break;
                        case  2: // write 0xff00
                            decompressed[outPos++] = 0x00;
                            decompressed[outPos++] = 0xff;
                            break;
                        case  3: // write 0xffff
                            decompressed[outPos++] = 0xff;
                            decompressed[outPos++] = 0xff;
                            break;
                        case  4: // write 0x00**, ** from data
                            decompressed[outPos++] = rom->read8(dataPtr++);
                            decompressed[outPos++] = 0x00;
                            break;
                        case  5: // write 0xff**, ** from data
                            decompressed[outPos++] = rom->read8(dataPtr++);
                            decompressed[outPos++] = 0xff;
                            break;
                        case  6: // write 0x**00, ** from data
                            decompressed[outPos++] = 0x00;
                            decompressed[outPos++] = rom->read8(dataPtr++);
                            break;
                        case  7: // write 0x**ff, ** from data
                            decompressed[outPos++] = 0xff;
                            decompressed[outPos++] = rom->read8(dataPtr++);
                            break;
                        case  8: // write 0x****, ** from data (same byte twice)
                        {
                            uint8_t v = rom->read8(dataPtr++);
                            decompressed[outPos++] = v;
                            decompressed[outPos++] = v;
                            break;
                        }
                        case  9: // repeat last word
                        case 10: // repeat last word twice
                        case 11: // repeat last word three times
                        case 12: // repeat last word 4+N times
                        {
                            uint8_t n = mode-9+1 + ((mode==12)? read4cmdBits() : 0);
                            for (uint8_t j=0; j<n; j++) {
                                if (outPos<2) {
                                    decompressed[outPos++] = 0;
                                    decompressed[outPos++] = 0; // no previous word -> zero
                                } else {
                                    decompressed[outPos] = decompressed[outPos-2]; outPos++;
                                    decompressed[outPos] = decompressed[outPos-2]; outPos++;
                                }
                                if (outPos>=uncompressedSize) break;
                            }
                            break;
                        }
                        case 13: // write 0x**RR, ** from data, RR = last word & 0x00ff
                            if (outPos<2) {
                                decompressed[outPos++] = 0; // no previous word -> zero
                            } else {
                                decompressed[outPos] = decompressed[outPos-2]; outPos++;
                            }
                            decompressed[outPos++] = rom->read8(dataPtr++);
                            break;
                        case 14: // write 0xRR**, ** from data, RR = last word & 0xff00
                            decompressed[outPos++] = rom->read8(dataPtr++);
                            if (outPos<2) {
                                decompressed[outPos++] = 0; // no previous word -> zero
                            } else {
                                decompressed[outPos] = decompressed[outPos-2]; outPos++;
                            }
                            break;
                        case 15: // write 0x^^**, ** from data, ^^ bitwise inverse
                        {
                            uint8_t v = rom->read8(dataPtr++);
                            decompressed[outPos++] = v;
                            decompressed[outPos++] = v^0xff;
                            break;
                        }
                        }
                    }
                    if (outPos>=uncompressedSize) break;
                    compressionIndicators<<=1;
                }
                if (outPos>=uncompressedSize) break;
            }
            // actual data starts 1 byte after data address, this has to be handled in save()
            romsize = dataPtr - dataaddr;
            if (romsize > sizeof(data)) {
                qWarning("Tile %d: can not store copy of data. Buffer too small.", i);
                memset(data, 0, sizeof(data));
            } else {
                rom->readBlock(dataaddr, data, romsize);
                if (romsize<sizeof(data)) memset(data+romsize, 0, sizeof(data)-romsize);
            }
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
        if (decompressed) delete[] decompressed;
        return res;
    }

public:
    Tile(int i, Rom* rom, int size=16) {
        this->i = i;
        this->size = size;
        if (size==16) { // 16x16 tiles (not sure if/where 8x8 are)
            ptraddr = 0xee0000 + i*3;
            dataaddr = rom->read24(ptraddr);
            // compression flag is part of data for tiles, data address is absolute pointer
        } else {
            assert(false);
        }
        romsize = 0;
        pixels = loadPixels(rom, size);
    }
    virtual bool save(Rom* rom) const {
        return rom->writeBlock(dataaddr, data, romsize);
    }
};

#endif // TILE_H

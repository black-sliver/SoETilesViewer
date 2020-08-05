#ifndef SPRITEINFO_H
#define SPRITEINFO_H

#include <stdint.h>
#include "rom.h"
#include <vector>
#include <QByteArray>


struct SpriteChunk {
    uint8_t flags;
    int8_t x;
    int8_t y;
    uint16_t block;

    QString toString() const // TODO: remove Qt dependency
    {
        return QStringLiteral("0x%1 @ %2,%3, flags %4 ").arg(block, 4, 16, QLatin1Char('0')).arg(x, 4, 10).arg(y, 4, 10).arg(flags, 2, 16);
    }

    SpriteChunk(size_t addr, Rom* rom) {
        flags = rom->read8(addr+0);
        x     = (int8_t)rom->read8(addr+1);
        y     = (int8_t)rom->read8(addr+2);
        block = rom->read16(addr+3);
    }
    SpriteChunk() {
        flags=0;
        x=0;
        y=0;
        block=0;
    }
};

struct SpriteInfo {
    size_t addr;

    uint8_t dataoff;
    std::vector<SpriteChunk> chunks;

    QString toString() // TODO: remove Qt dependency
    {
        return QStringLiteral("$%1 -> %2,%3 ").arg(addr, 6, 16, QLatin1Char('0')).arg(chunks.size(), 0, 10).arg(dataoff, 0, 10);
    }

    SpriteInfo(size_t addr, Rom* rom) {
        this->addr = addr;
        uint8_t count = rom->read8(addr);
        dataoff = rom->read8(addr+1);
        for (unsigned n=0; n<count; n++) {
            chunks.push_back({addr+dataoff, rom});
            addr += 5;
        }
    }

    size_t size() const {
        return dataoff + chunks.size() * 5;
    }


};

#endif // SPRITEINFO_H

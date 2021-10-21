#ifndef ABSTRACTTILE_H
#define ABSTRACTTILE_H


#include <QByteArray>
#include <QString>
#include <string.h>
#include "rom.h"


struct AbstractTile {
    int i;
    int size;

    unsigned ptraddr;
    unsigned dataaddr;
    bool compressed;
    uint8_t data[128+128/8]; // this is the maximum amount a block can use in theory
    unsigned romsize;
    QByteArray pixels;

    virtual ~AbstractTile() {}

    QString toString() const // TODO: remove qt dependency
    {
        return QStringLiteral("#%1 -> $%2 -> $%3 %4B ").arg(i, 4, 10).arg(ptraddr, 0, 16).arg(dataaddr, 0, 16).arg(romsize, 3, 10) + (compressed ? "compressed" : "uncompressed");
    }

    const QByteArray& getPixels() const
    {
        return pixels;
    }

    virtual bool setPixels(const QByteArray&)
    {
        return false;
    }

    virtual bool save(Rom*) const
    {
        return false;
    }

};


#endif // ABSTRACTTILE_H

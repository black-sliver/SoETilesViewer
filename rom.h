#ifndef ROM_H
#define ROM_H


#include <QString>
#include <stdint.h>
#include <stdio.h>
#include <QFile>


class Rom
{
public:
    Rom(QString filename)
        : _f(filename)
    {
        _ok = _f.open(QIODevice::ReadOnly);
    }

private:

public:
    unsigned mapaddr(unsigned addr) {
        return addr & ~(0xc00000);
    }

    uint8_t read8(unsigned addr)
    {
        addr = mapaddr(addr) + _romoff;
        uint8_t buf[] = {0};
        if (_f.seek(addr) && _f.read((char*)&buf, sizeof(buf))) return buf[0];
        return 0; // TODO: warn
    }
    uint16_t read16(unsigned addr)
    {
        addr = mapaddr(addr) + _romoff;
        uint8_t buf[] = {0,0};
        if (_f.seek(addr) && _f.read((char*)&buf, sizeof(buf))) {
            uint16_t res  = buf[1];
            res<<=8; res += buf[0];
            return res;
        }
        return 0; // TODO: warn
    }
    uint32_t read24(unsigned addr)
    {
        addr = mapaddr(addr) + _romoff;
        uint8_t buf[] = {0,0,0};
        if (_f.seek(addr) && _f.read((char*)&buf, sizeof(buf))) {
            uint32_t res  = buf[2];
            res<<=8; res += buf[1];
            res<<=8; res += buf[0];
            return res;
        }
        return 0; // TODO: warn
    }
    bool readBlock(unsigned addr, void* dst, size_t len) {
        addr = mapaddr(addr) + _romoff;
        if (_f.seek(addr) && _f.read((char*)dst, len)) return true;
        memset(dst, 0, sizeof(len));
        return false;
    }
    bool isOpen() const { return _ok; }
private:
    bool _ok = false;
    unsigned _romoff = 0; // TODO: detect header
    QFile _f;
};

#endif // ROM_H

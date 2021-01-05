#ifndef ROM_H
#define ROM_H


#include <QString>
#include <stdint.h>
#include <stdio.h>
#include <QFile>


#ifdef _MSC_VER
#define initializerlistconstexpr const
#else
#define initializerlistconstexpr constexpr
#endif

initializerlistconstexpr
auto BASE_SIZES = {0x100000,0x200000,0x300000,0x400000,0x500000,0x600000,0x700000,0x800000};
constexpr auto HEADER_LEN = 512;

class Rom
{
public:
    Rom(QString filename)
        : _f(filename)
    {
        _ok = _f.open(QIODevice::ReadOnly); // will reopen RW later
        bool validSize = false;
        for (auto basesize : BASE_SIZES) {
            if (_f.size() == basesize+HEADER_LEN) {
                _romoff = HEADER_LEN;
                validSize = true;
                break;
            } else if (_f.size() == basesize) {
                validSize = true;
                break;
            }
        }
        if (!validSize) {
            _f.close();
            _ok = false;
        }
    }
    ~Rom()
    {
        _f.close();
        _ok = false;
    }

private:

public:
    unsigned mapaddr(unsigned addr) const {
        return addr & ~(0xc00000);
    }
    bool addrValid(unsigned addr) const {
        addr = mapaddr(addr) + _romoff;
        return (addr<_f.size());
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
    uint32_t read32(unsigned addr)
    {
        addr = mapaddr(addr) + _romoff;
        uint8_t buf[] = {0,0,0,0};
        if (_f.seek(addr) && _f.read((char*)&buf, sizeof(buf))) {
            uint32_t res  = buf[3];
            res<<=8; res += buf[2];
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
    QString readString(unsigned addr, size_t maxlen=256) {
        addr = mapaddr(addr) + _romoff;
        QString res;
        if (_f.seek(addr)) {
            char c;
            do {
                c = (char)_f.read(1)[0];
                if (!c) break;
                res += c;
            } while (true);
        }
        return res;
    }
    bool writeBlock(unsigned addr, void* src, size_t len) {
        addr = mapaddr(addr) + _romoff;
        if (_ok && !_f.isWritable()) { // reopen RW
            _f.close();
            _ok = _f.open(QIODevice::ReadWrite) && _f.isWritable();
        }
        if (!_ok) return false;
        if (_f.seek(addr) && (size_t)_f.write((char*)src, len)==len) {
            _f.flush();
            return true;
        }
        return false;
    }
    bool isOpen() const { return _ok; }
    bool saveAs(QString filename) {
        if (!_ok) return false;
        _f.close();
        qDebug("Orig closed\n");
        if (QFile::remove(filename)) qDebug("Destination removed\n");
        if (!QFile::copy(_f.fileName(), filename)) return false;
        qDebug("File copied\n");
        _f.setFileName(filename);
        _ok = _f.open(QIODevice::ReadOnly);
        qDebug(_ok ? "New file opened\n" : "Could not open new file\n");
        return _ok;
    }
private:
    bool _ok = false;
    unsigned _romoff = 0; // TODO: detect header
    QFile _f;
};

#endif // ROM_H

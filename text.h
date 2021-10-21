#ifndef TEXT_H
#define TEXT_H


#include "rom.h"
#include <string>


struct Text {
    int i;
    std::string text;
    bool valid;
    uint32_t ptraddr;
    uint32_t dataaddr;
    uint32_t datalen;
    bool compressed;

    virtual ~Text() {};
    Text(int i, Rom* rom) {
        this->i = i;
        valid = true;
        ptraddr = 0x91d000 + i*3;
        dataaddr = rom->read24(ptraddr);
        compressed = !!(dataaddr & 0x800000);
        dataaddr = 0xc00000 + (dataaddr & 0x7fff) + ((dataaddr & 0x7f8000)<<1); // convert to hirom mapping
        if (dataaddr>0xcfffff) valid = false;
        else text = get_text_from_data(rom, dataaddr, compressed, &datalen);
        if (text.length() > 2000) valid = false;
    }
private:
    static std::string get_text_from_data(Rom* rom, uint32_t addr, bool mode, uint32_t* lenout=nullptr)
    {
        if (lenout) *lenout = 0;
        if (addr>0xcfffff) return "";
        uint32_t startaddr = addr;
        std::string data;
        if (mode) {
            uint8_t next_plain = 0;
            do {
                uint8_t d = rom->read8(addr);
                if (next_plain) {
                    next_plain--;
                    if (!d && !next_plain) break;
                    data += (char)d;
                } else if (d==0xc0) {
                    addr++;
                    uint32_t wordpp = 0x91f46c + ((uint16_t)rom->read8(addr)<<1);
                    uint32_t wordp = 0x91f7d5 + rom->read16(wordpp);
                    for (char c; (c=rom->read8(wordp)); wordp++)  {
                        data += c;
                    }
                } else if ((d&0xc0)==0xc0) {
                    d = (d<<1)&0x7e;
                    uint32_t wordpp = 0x91f3ec + d;
                    uint32_t wordp = 0x91f66c + rom->read16(wordpp);
                    for (char c; (c=rom->read8(wordp)); wordp++)  {
                        data += c;
                    }
                } else if ((d&0xc0)==0x40) {
                    next_plain = d&0x3f;
                } else if ((d&0xc0)==0x80) {
                    d<<=1;
                    char c;
                    c = (char)rom->read8(0x91f32e + d);
                    data += c;
                    c = (char)rom->read8(0x91f32f + d);
                    if (!c) break;
                    data += c;
                } else if ((d&0xc0)==0) {
                    char c = (char)rom->read8(0x91f3ae + d);
                    if (!c) break;
                    data += c;
                }
                addr++;
            } while (true);
        } else {
            for (char c; (c=rom->read8(addr)); addr++)  {
                data += c;
            }
        }
        if (lenout) *lenout = addr - startaddr + 1;

        // make raw data look nice
        std::string printable;
        char c1=0, c2=0;
        bool inPause=false;
        for (auto c: data) {
            if ((uint8_t)c==0x80 && (uint8_t)c2==0x80 && inPause) {
                // c1 = duration?
                if (c1>=0x20 && c1<0x7f)
                    printable.erase(printable.length()-7, 7);
                else
                    printable.erase(printable.length()-12, 12);
                char buf[3]; snprintf(buf, sizeof(buf), "%02x", (uint8_t)c1);
                printable+="[PAUSE:" + std::string(buf) + "]";
            }
            else if (c>=0x20 && c<0x7f) printable+=c;
            else if (c=='\n') printable+="\n";
            else {
                char buf[7]; snprintf(buf, sizeof(buf), "[0x%02hhx]", (uint8_t)c);
                printable += buf;
            }

            if ((uint8_t)c==0x80) inPause=!inPause;
            c2 = c1;
            c1 = c;
        }
        return printable;
    }

    static std::string get_text(Rom* rom, uint32_t addr)
    {
        addr = rom->read24(addr); // read "real" text address from pointer to text address table
        bool mode = (addr & 0x800000); // msbit set means compressed
        addr = 0xc00000 + (addr & 0x7fff) + ((addr & 0x7f8000)<<1); // convert to hirom mapping
        return get_text_from_data(rom, addr, mode);
    }
};

#endif // TEXT_H

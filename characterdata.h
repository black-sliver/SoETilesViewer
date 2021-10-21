#ifndef CHARACTERDATA_H
#define CHARACTERDATA_H


#include "rom.h"
#include <QByteArray>
#include <string.h>
#include <assert.h>


struct CharacterData final {
    int i;
    uint32_t nameptr;
    uint16_t unknown03;
    uint16_t unknown05;
    uint16_t flags;
    uint16_t palette;
    uint16_t unknown0b;
    uint16_t unknown0d;
    uint16_t hp; // 16 or 32bit?
    uint16_t unknown11;
    uint16_t aggro_range;
    uint16_t aggro_chance; // in 256
    uint16_t unknown17;
    uint16_t attack;
    uint16_t defense;
    uint16_t magic_defense;
    uint16_t evade; // player hit_rate * enemy evade ~= hit chance
    uint16_t hit_rate; // enemy hit_rate * player evade ~= hit chance
    uint32_t exp;
    uint16_t money;
    uint8_t prize_chance;
    uint16_t unknown2a;
    uint16_t charge_limit;
    uint16_t charge_rate;
    uint16_t attack_proc;
    uint16_t anim_stand;
    uint16_t anim_walk;
    uint16_t anim_run;
    uint16_t anim_atk0;
    uint16_t anim_atk1;
    uint16_t anim_atk2;
    uint16_t anim_atk3;
    uint16_t anim_damage;
    uint16_t anim_death;
    uint16_t anim_spoils;
    uint16_t anim_block;

    QString name;
    QString namehint;
    int namelength=0;

    QString toString() // TODO: remove qt dependency
    {
#ifdef CHARACTER_STRING_WITH_ID
        return QStringLiteral("#%1 %2 @$%3").arg(i, 4, 10).arg(name.isEmpty() ? namehint : name).arg(getAddr(), 0, 16);
#else
        return QStringLiteral("%2 @$%3").arg(name.isEmpty() ? namehint : name).arg(getAddr(), 0, 16);
#endif
    }
private:
    static const int BASE_ADDR = 0x8eB678;
    static const int BOY_NAME_PTR = 0x7e2210;
    static const int DOG_NAME_PTR = 0x7e2234;
    static const int SIZE = 74;
public:
    CharacterData(int i, Rom* rom) {
        this->i = i;
        nameptr = rom->read24(getNamePtrAddr());
        unknown03 = rom->read16(getUnknown03Addr());
        unknown05 = rom->read16(getUnknown05Addr());
        flags = rom->read24(getFlagsAddr());
        palette = rom->read16(getPaletteAddr());
        unknown0b = rom->read16(getUnknown0bAddr());
        unknown0d = rom->read16(getUnknown0dAddr());
        hp = rom->read16(getHPAddr());
        unknown11 = rom->read16(getUnknown11Addr());
        aggro_range = rom->read16(getAggroRangeAddr());
        aggro_chance = rom->read16(getAggroChanceAddr());
        unknown17 = rom->read16(getUnknown17Addr());
        attack = rom->read16(getAttackAddr());
        defense = rom->read16(getDefenseAddr());
        magic_defense = rom->read16(getMagicDefenseAddr());
        evade = rom->read16(getEvadeAddr());
        hit_rate = rom->read16(getHitRateAddr());
        exp = rom->read32(getExpAddr());
        money = rom->read16(getMoneyAddr());
        prize_chance = rom->read8(getPrizeChanceAddr());
        unknown2a = rom->read16(getUnknown2aAddr());
        charge_limit = rom->read16(getChargeLimitAddr());
        charge_rate = rom->read16(getChargeRateAddr());
        attack_proc = rom->read16(getAttackProcAddr());
        anim_stand = rom->read16(getAnimStandAddr());
        anim_walk = rom->read16(getAnimWalkAddr());
        anim_run = rom->read16(getAnimRunAddr());
        anim_atk0 = rom->read16(getAnimAtk0Addr());
        anim_atk1 = rom->read16(getAnimAtk1Addr());
        anim_atk2 = rom->read16(getAnimAtk2Addr());
        anim_atk3 = rom->read16(getAnimAtk3Addr());
        anim_damage = rom->read16(getAnimDamageAddr());
        anim_death = rom->read16(getAnimDeathAddr());
        anim_spoils = rom->read16(getAnimSpoilsAddr());
        anim_block = rom->read16(getAnimBlockAddr());

        if (nameptr>=0x800000 && nameptr<0xd00000) {
            name = rom->readString(nameptr);
            namelength = name.length();
            while (rom->addrValid(nameptr+namelength+1) && rom->read8(nameptr+namelength+1)==0) namelength++;
        } else if (nameptr == BOY_NAME_PTR) {
            namehint = "<Boy Name>";
        } else if (nameptr == DOG_NAME_PTR) {
            namehint = "<Dog Name>";
        } else {
            namehint = QStringLiteral("$%1").arg(nameptr, 0, 16);
        }

    }
    bool save(Rom* rom) {
        (void)rom;
        qDebug("Not implemented!");
        assert(false);
    }
    uint32_t getAddr() const { return BASE_ADDR + i * SIZE; }
    uint32_t getNamePtrAddr() const { return getAddr() + 0x00; }
    uint32_t getUnknown03Addr() const { return getAddr() + 0x03; }
    uint32_t getUnknown05Addr() const { return getAddr() + 0x05; }
    uint32_t getFlagsAddr() const { return getAddr() + 0x07; }
    uint32_t getPaletteAddr() const { return getAddr() + 0x09; }
    uint32_t getUnknown0bAddr() const { return getAddr() + 0x0b; }
    uint32_t getUnknown0dAddr() const { return getAddr() + 0x0d; }
    uint32_t getHPAddr() const { return getAddr() + 0x0f; }
    uint32_t getUnknown11Addr() const { return getAddr() + 0x11; }
    uint32_t getUnknown17Addr() const { return getAddr() + 0x17; }
    uint32_t getAggroRangeAddr() const { return getAddr() + 0x13; }
    uint32_t getAggroChanceAddr() const { return getAddr() + 0x15; }
    uint32_t getAttackAddr() const { return getAddr() + 0x19; }
    uint32_t getDefenseAddr() const { return getAddr() + 0x1b; }
    uint32_t getMagicDefenseAddr() const { return getAddr() + 0x1d; }
    uint32_t getEvadeAddr() const { return getAddr() + 0x1f; }
    uint32_t getHitRateAddr() const { return getAddr() + 0x21; }
    uint32_t getExpAddr() const { return getAddr() + 0x23; }
    uint32_t getMoneyAddr() const { return getAddr() + 0x27; }
    uint32_t getPrizeChanceAddr() const { return getAddr() + 0x29; }
    uint32_t getUnknown2aAddr() const { return getAddr() + 0x2a; }
    uint32_t getChargeLimitAddr() const { return getAddr() + 0x2c; }
    uint32_t getChargeRateAddr() const { return getAddr() + 0x2e; }
    uint32_t getAttackProcAddr() const { return getAddr() + 0x30; }
    uint32_t getAnimStandAddr() const { return getAddr() + 0x32; }
    uint32_t getAnimWalkAddr() const { return getAddr() + 0x34; }
    uint32_t getAnimRunAddr() const { return getAddr() + 0x36; }
    uint32_t getAnimAtk0Addr() const { return getAddr() + 0x38; }
    uint32_t getAnimAtk1Addr() const { return getAddr() + 0x3a; }
    uint32_t getAnimAtk2Addr() const { return getAddr() + 0x3c; }
    uint32_t getAnimAtk3Addr() const { return getAddr() + 0x3e; }
    uint32_t getAnimDamageAddr() const { return getAddr() + 0x40; }
    uint32_t getAnimDeathAddr() const { return getAddr() + 0x42; }
    uint32_t getAnimSpoilsAddr() const { return getAddr() + 0x44; }
    uint32_t getAnimBlockAddr() const { return getAddr() + 0x46; }

};
#endif // CHARACTERDATA_H

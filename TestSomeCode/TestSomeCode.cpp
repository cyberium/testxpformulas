// TestSomeCode.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <string>


#define MAX_PLAYER_LEVEL 60
#define MAX_CREATURE_LEVEL 70

typedef unsigned int uint32;
typedef int int32;
class Map
{
    public:
        Map(bool _isRaid) : isRaidMap(_isRaid) {}

        bool IsNoRaid() const { return isRaidMap; }

        bool isRaidMap;
};

class Unit
{
    public:
        Unit(uint32 _currLevel, Map* _currMap) : CurrentLevel(_currLevel), CurrentMap(_currMap) {}
        bool IsTotem() const { return false; }
        bool IsPet() const { return false; }
        bool IsNoXp() const { return false; }
        bool IsCritter() const { return false; }
        uint32 getLevel() const { return CurrentLevel; }
        Map* GetMap() const { return CurrentMap; }

        uint32 CurrentLevel;
        Map* CurrentMap;
};

class Player : public Unit
{
    public:
        Player(uint32 _currLevel, Map* _currMap) : Unit(_currLevel, _currMap) {}
};

class CreatureInfo
{
    public:
        CreatureInfo() : ExperienceMultiplier(1.0f) {}
        float ExperienceMultiplier;
};

class Creature : public Unit
{
    public:
        Creature(uint32 _currLevel, Map* _currMap, bool _isElite, CreatureInfo* _CInfo) : Unit(_currLevel, _currMap), isElite(_isElite), CInfo(_CInfo) {}
        bool IsElite() const { return isElite; }
        CreatureInfo* GetCreatureInfo() const { return CInfo; }



        bool isElite;
        CreatureInfo* CInfo;
};

enum ConfigIndexes
{
    CONFIG_FLOAT_RATE_XP_KILL = 0
};

class World
{
    public:

        float const getConfig(ConfigIndexes index) { return 1; }
};

World sWorld;

namespace XP
{
    inline bool IsTrivialLevelDifference(uint32 unitLvl, uint32 targetLvl)
    {
        if (unitLvl > targetLvl)
        {
            const uint32 diff = (unitLvl - targetLvl);
            switch (unitLvl / 5)
            {
            case 0:     // 0-4
            case 1:     // 5-9
                return (diff > 4);
            case 2:     // 10-14
            case 3:     // 15-19
                return (diff > 5);
            case 4:     // 20-24
            case 5:     // 25-29
                return (diff > 6);
            case 6:     // 30-34
            case 7:     // 35-39
                return (diff > 7);
            case 8:     // 40-44
                return (diff > 8);
            case 9:     // 45-49
                return (diff > 9);
            case 10:    // 50-54
                return (diff > 10);
            case 11:    // 55-59
                return (diff > 11);
            default:    // 60+
                return (diff > 12);
            }
        }
        return false;
    }

    enum XPColorChar { RED, ORANGE, YELLOW, GREEN, GRAY };

    inline uint32 GetGrayLevel(uint32 pl_level)
    {
        if (pl_level <= 5)
            return 0;
        if (pl_level <= 39)
            return pl_level - 5 - pl_level / 10;
        if (pl_level <= 59)
            return pl_level - 1 - pl_level / 5;
        return pl_level - 9;
    }

    inline XPColorChar GetColorCode(uint32 pl_level, uint32 mob_level)
    {
        if (mob_level >= pl_level + 5)
            return RED;
        if (mob_level >= pl_level + 3)
            return ORANGE;
        if (mob_level >= pl_level - 2)
            return YELLOW;
        if (mob_level > GetGrayLevel(pl_level))
            return GREEN;
        return GRAY;
    }

    inline uint32 GetZeroDifference(uint32 unit_level)
    {
        if (unit_level < 8)  return 5;
        if (unit_level < 10) return 6;
        if (unit_level < 12) return 7;
        if (unit_level < 16) return 8;
        if (unit_level < 20) return 9;
        if (unit_level < 30) return 11;
        if (unit_level < 40) return 12;
        if (unit_level < 45) return 13;
        if (unit_level < 50) return 14;
        if (unit_level < 55) return 15;
        if (unit_level < 60) return 16;
        return 17;
    }

    inline float BaseGain(uint32 unit_level, uint32 mob_level)
    {
        const uint32 nBaseExp = unit_level * 5 + 45;
        if (mob_level >= unit_level)
        {
            uint32 nLevelDiff = mob_level - unit_level;
            if (nLevelDiff > 4)
                nLevelDiff = 4;
            return nBaseExp * (1.0f + (0.05f * nLevelDiff));
        }
        uint32 gray_level = GetGrayLevel(unit_level);
        if (mob_level > gray_level)
        {
            uint32 ZD = GetZeroDifference(unit_level);
            uint32 nLevelDiff = unit_level - mob_level;
            return nBaseExp * (1.0f - (float(nLevelDiff) / ZD));
        }
        return 0;
    }

    inline uint32 Gain(Unit const* unit, Creature* target)
    {
        if (target->IsTotem() || target->IsPet() || target->IsNoXp() || target->IsCritter())
            return 0;

        float xp_gain = BaseGain(unit->getLevel(), target->getLevel());
        if (xp_gain == 0.0f)
            return 0;

        if (target->IsElite())
        {
            if (target->GetMap()->IsNoRaid())
                xp_gain *= 2.5f;
            else
                xp_gain *= 2.0f;
        }

        xp_gain *= target->GetCreatureInfo()->ExperienceMultiplier;

        return (uint32)(std::round(xp_gain * sWorld.getConfig(CONFIG_FLOAT_RATE_XP_KILL)));
    }

    inline float xp_in_group_rate(uint32 count, bool isRaid)
    {
        // TODO: this formula is completely guesswork only based on a logical assumption
        switch (count)
        {
        case 0:
        case 1:
        case 2:
            return 1.0f;
        case 3:
            return 1.166f;
        case 4:
            return 1.3f;
        case 5:
            return 1.4f;
        default:
            return std::max(1.f - count * 0.05f, 0.01f);
        }
    }
}

namespace OLDXP
{
    inline bool IsTrivialLevelDifference(uint32 unitLvl, uint32 targetLvl)
    {
        if (unitLvl > targetLvl)
        {
            const uint32 diff = (unitLvl - targetLvl);
            switch (unitLvl / 5)
            {
            case 0:     // 0-4
            case 1:     // 5-9
                return (diff > 4);
            case 2:     // 10-14
            case 3:     // 15-19
                return (diff > 5);
            case 4:     // 20-24
            case 5:     // 25-29
                return (diff > 6);
            case 6:     // 30-34
            case 7:     // 35-39
                return (diff > 7);
            case 8:     // 40-44
                return (diff > 8);
            case 9:     // 45-49
                return (diff > 9);
            case 10:    // 50-54
                return (diff > 10);
            case 11:    // 55-59
                return (diff > 11);
            default:    // 60+
                return (diff > 12);
            }
        }
        return false;
    }

    enum XPColorChar { RED, ORANGE, YELLOW, GREEN, GRAY };

    inline uint32 GetGrayLevel(uint32 pl_level)
    {
        if (pl_level <= 5)
            return 0;
        if (pl_level <= 39)
            return pl_level - 5 - pl_level / 10;
        if (pl_level <= 59)
            return pl_level - 1 - pl_level / 5;
        return pl_level - 9;
    }

    inline XPColorChar GetColorCode(uint32 pl_level, uint32 mob_level)
    {
        if (mob_level >= pl_level + 5)
            return RED;
        if (mob_level >= pl_level + 3)
            return ORANGE;
        if (mob_level >= pl_level - 2)
            return YELLOW;
        if (mob_level > GetGrayLevel(pl_level))
            return GREEN;
        return GRAY;
    }

    inline uint32 GetZeroDifference(uint32 pl_level)
    {
        if (pl_level < 8)  return 5;
        if (pl_level < 10) return 6;
        if (pl_level < 12) return 7;
        if (pl_level < 16) return 8;
        if (pl_level < 20) return 9;
        if (pl_level < 30) return 11;
        if (pl_level < 40) return 12;
        if (pl_level < 45) return 13;
        if (pl_level < 50) return 14;
        if (pl_level < 55) return 15;
        if (pl_level < 60) return 16;
        return 17;
    }

    inline uint32 BaseGain(uint32 pl_level, uint32 mob_level)
    {
        const uint32 nBaseExp = 45;
        if (mob_level >= pl_level)
        {
            uint32 nLevelDiff = mob_level - pl_level;
            if (nLevelDiff > 4)
                nLevelDiff = 4;
            return ((pl_level * 5 + nBaseExp) * (20 + nLevelDiff) / 10 + 1) / 2;
        }
        uint32 gray_level = GetGrayLevel(pl_level);
        if (mob_level > gray_level)
        {
            uint32 ZD = GetZeroDifference(pl_level);
            return (pl_level * 5 + nBaseExp) * (ZD + mob_level - pl_level) / ZD;
        }
        return 0;
    }

    inline uint32 Gain(Player* player, Creature* target)
    {
        if (target->IsTotem() || target->IsPet() || target->IsNoXp() || target->IsCritter())
            return 0;

        uint32 xp_gain = BaseGain(player->getLevel(), target->getLevel());
        if (xp_gain == 0)
            return 0;

        if (target->IsElite())
        {
            if (target->GetMap()->IsNoRaid())
                xp_gain *= 2.5;
            else
                xp_gain *= 2;
        }

        xp_gain *= target->GetCreatureInfo()->ExperienceMultiplier;

        return (uint32)(xp_gain * sWorld.getConfig(CONFIG_FLOAT_RATE_XP_KILL));
    }

    inline float xp_in_group_rate(uint32 count, bool isRaid)
    {
        // TODO: this formula is completely guesswork only based on a logical assumption
        switch (count)
        {
        case 0:
        case 1:
        case 2:
            return 1.0f;
        case 3:
            return 1.166f;
        case 4:
            return 1.3f;
        case 5:
            return 1.4f;
        default:
            return std::max(1.f - count * 0.05f, 0.01f);
        }
    }
}

struct Report
{
    Report(Unit* _unit, Creature* _creature, uint32 _oldXP, uint32 _newXP) : unit(_unit), creature(_creature), oldXP(_oldXP), newXP(_newXP)
    {
        difference = (oldXP > newXP) ? oldXP - newXP : newXP - oldXP;
    }


    Unit* unit;
    Creature* creature;
    int32 difference;
    uint32 oldXP;
    uint32 newXP;
};

int main()
{
    std::vector<Map> MapVector;
    std::vector<Player> PlayerOrPets;
    std::vector<Creature> CreatureVector;

    MapVector.reserve(2);
    MapVector.emplace_back(false);
    MapVector.emplace_back(true);

    PlayerOrPets.reserve(MAX_PLAYER_LEVEL);
    for (uint32 i = 0; i < MAX_PLAYER_LEVEL; ++i)
    {
        int uLevel = i + 1;
        PlayerOrPets.emplace_back(uLevel, &MapVector[0]);
    }

    CreatureVector.reserve(MAX_CREATURE_LEVEL * 4); // normal, elite, normal in raid map, elite in raid map
    CreatureInfo defaultCInfo;

    // O .. MAX_CREATURE_LEVEL => normal creature
    for (uint32 i = 0; i < MAX_CREATURE_LEVEL; ++i)
    {
        int uLevel = i + 1;

        CreatureVector.emplace_back(uLevel, &MapVector[0], false, &defaultCInfo);
    }

    // MAX_CREATURE_LEVEL .. MAX_CREATURE_LEVEL*2 => elite creature
    for (uint32 i = MAX_CREATURE_LEVEL; i < MAX_CREATURE_LEVEL * 2; ++i)
    {
        int uLevel = i + 1;
        CreatureVector.emplace_back(uLevel, &MapVector[0], true, &defaultCInfo);
    }

    // MAX_CREATURE_LEVEL *2 .. MAX_CREATURE_LEVEL*3 => normal creature in raid map
    for (uint32 i = MAX_CREATURE_LEVEL * 2; i < MAX_CREATURE_LEVEL; ++i)
    {
        int uLevel = i + 1;
        CreatureVector.emplace_back(uLevel, &MapVector[1], false, &defaultCInfo);
    }

    // MAX_CREATURE_LEVEL*3 .. MAX_CREATURE_LEVEL*4 => elite creature in raid map
    for (uint32 i = MAX_CREATURE_LEVEL * 3; i < MAX_CREATURE_LEVEL * 2; ++i)
    {
        int uLevel = i + 1;
        CreatureVector.emplace_back(uLevel, &MapVector[1], true, &defaultCInfo);
    }


    std::vector<Report> ReportVector;
    std::cout << std::endl;
    for (uint32 i = 0 ; i < PlayerOrPets.size(); ++i)
    {

        for (uint32 j = 0; j < CreatureVector.size(); j++)
        {
            uint32 oldXP = OLDXP::Gain(&PlayerOrPets[i], &CreatureVector[j]);
            uint32 newXP = XP::Gain(&PlayerOrPets[i], &CreatureVector[j]);

            if (oldXP == newXP)
            {
                std::cout << ".";
            }
            else
            {
                std::cout << "!";
                ReportVector.emplace_back(&PlayerOrPets[i], &CreatureVector[j], oldXP, newXP);
            }

        }
        std::cout << std::endl;

    }

    std::cout << "Identified difference" << std::endl;
    std::cout << "| Player Level | Creature level | Difference | Old XP | New XP | Is Elite? | Is raid Map? |" << std::endl;
    for (auto& report : ReportVector)
    {
        std::string elite = report.creature->isElite ? "yes" : "no";
        std::string raidMap = report.creature->GetMap()->IsNoRaid() ? "no" : "yes";
        printf("|%13u |%15u |%11d |%7u |%7u |%10s |%13s |\n", report.unit->getLevel(), report.creature->getLevel(), report.difference, report.oldXP, report.newXP, elite.c_str(), raidMap.c_str());

    }
}

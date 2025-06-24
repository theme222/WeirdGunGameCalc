// Why did I decide to write this in c++? well... I wanted to challenge myself because python would just be wayy too boring
// It's also because I wanted the speed of c but I decided writing structs was a bad idea
// who am I kidding it's because I hate myself
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <filesystem>
#include <queue>
#include <sys/types.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <thread>
#include <variant>
#include "include/json.hpp" // "nlohmann/json.hpp"
#include "include/CLI11.hpp" // "CLIUtils"

#define chrono std::chrono
using json = nlohmann::json;
typedef std::pair<float, float> fpair;

#define DEFAULTVECTORSIZE 100000
#define ALLMULTFLAG 4294967295 // 2^32 - 1
#define NILMIN -69420.5f
#define NILMAX 69420.5f
#define NILRANGE {NILMIN, NILMAX} // Hehehheheh who says programmers can't have fun
#define NILINT -1
#define CATEGORYCOUNT 7 // This is mostly for looking at what values are based on the category count for easy refactor

namespace Input
{
    std::filesystem::path fileDir = "Data";
    std::string outpath = "Results.txt";
    std::string sortType = "TTK";
    std::string method = "PRUNE";

    // These handle if the users force a specific part to be included
    std::string forceBarrel;
    std::string forceMagazine;
    std::string forceCore;
    std::string forceStock;
    std::string forceGrip;

    std::vector<std::string> banBarrel;
    std::vector<std::string> banMagazine;
    std::vector<std::string> banCore;
    std::vector<std::string> banStock;
    std::vector<std::string> banGrip;

    std::vector<std::string> includeCategories;

    int threadsToMake = 4;
    int howManyTopGunsToDisplay = 10;

    fpair damageRange = NILRANGE;
    fpair magazineRange = NILRANGE;
    fpair spreadHipRange = NILRANGE;
    fpair spreadAimRange = NILRANGE;
    fpair recoilHipRange = NILRANGE;
    fpair recoilAimRange = NILRANGE;
    fpair movementSpeedRange = NILRANGE;
    fpair fireRateRange = NILRANGE;
    fpair healthRange = NILRANGE;
    fpair pelletRange = NILRANGE;
    fpair timeToAimRange = NILRANGE;
    fpair reloadRange = NILRANGE;
    fpair detectionRadiusRange = NILRANGE;
    fpair dropoffStudsRange = NILRANGE;
}


namespace Fast // Namespace to contain any indexing that uses the integer representation of categories and mults
{
    // penalties[coreCategory][partCategory]
    float penalties[CATEGORYCOUNT][CATEGORYCOUNT];

    std::map<std::string, int> fastifyCategory = {
        {"Assault Rifle", 0},
        {"Sniper", 1},
        {"SMG", 2},
        {"LMG", 3},
        {"Weird", 4},
        {"Shotgun", 5},
        {"BR", 6}
    };

    bool includeCategories_fast[CATEGORYCOUNT] = {false, false, false, false, false, false, false};
    int forceParts_fast[5] = {NILINT, NILINT, NILINT, NILINT, NILINT}; // Barrel Magazine Grip Stock Core
    std::vector<int> banParts_fast[5];  // TODO: Check if this needs to be a more efficient data structure

    enum MultFlags {
        DAMAGE = 1<<0,
        DROPOFFSTUDS = 1<<1,
        RELOAD = 1<<2,
        MAGAZINESIZE = 1<<3,
        FIRERATE = 1<<4,
        TIMETOAIM = 1<<5,
        MOVEMENTSPEEDMODIFIER = 1<<6,
        SPREADAIM = 1<<7,
        SPREADHIP = 1<<8,
        RECOILAIM = 1<<9,
        RECOILHIP = 1<<10,
        PELLETS = 1<<11,
        HEALTH = 1<<12,
        DETECTIONRADIUS = 1<<13,
    };

    bool MoreIsBetter(MultFlags propertyFlag)
    {
        switch (propertyFlag)
        {
        case DAMAGE:
        case MOVEMENTSPEEDMODIFIER:
        case HEALTH:
        case FIRERATE:
        case PELLETS:
        case DROPOFFSTUDS:
            return true;
        case RELOAD:
        case RECOILAIM:
        case RECOILHIP:
        case SPREADAIM:
        case SPREADHIP:
        case DETECTIONRADIUS:
            return false;
        default:
            throw std::invalid_argument("Invalid property flag");
        }
    }

    std::map<std::string, int> fastifyName = {};

    void PartExists(std::string partName)
    {
        if (!fastifyName.contains(partName))
            throw std::invalid_argument("Part " + partName + " doesn't exist");
    }

    void InitializeIncludeCategories()
    {
        for (auto& category : Input::includeCategories)
        {
            if (category == "AR") category = "Assault Rifle";
            if (!fastifyCategory.contains(category))
                throw std::invalid_argument("Category " + category + " doesn't exist");
            includeCategories_fast[fastifyCategory[category]] = true;
        }
    }

    void InitializeForceAndBanParts()
    {
        using namespace Input;

        if (forceBarrel != "")
        {
            PartExists(forceBarrel);
            forceParts_fast[0] = fastifyName[forceBarrel];
        }
        if (forceMagazine != "")
        {
            PartExists(forceMagazine);
            forceParts_fast[1] = fastifyName[forceMagazine];
        }
        if (forceGrip != "")
        {
            PartExists(forceGrip);
            forceParts_fast[2] = fastifyName[forceGrip];
        }
        if (forceStock != "")
        {
            PartExists(forceStock);
            forceParts_fast[3] = fastifyName[forceStock];
        }
        if (forceCore != "")
        {
            PartExists(forceCore);
            forceParts_fast[4] = fastifyName[forceCore];
        }

        for (auto barrel: banBarrel)
        {
            PartExists(barrel);
            banParts_fast[0].push_back(fastifyName[barrel]);
        }
        for (auto magazine: banMagazine)
        {
            PartExists(magazine);
            banParts_fast[1].push_back(fastifyName[magazine]);
        }
        for (auto grip: banGrip)
        {
            PartExists(grip);
            banParts_fast[2].push_back(fastifyName[grip]);
        }
        for (auto stock: banStock)
        {
            PartExists(stock);
            banParts_fast[3].push_back(fastifyName[stock]);
        }
        for (auto core: banCore)
        {
            PartExists(core);
            banParts_fast[4].push_back(fastifyName[core]);
        }

    }

    bool RangeFilter(float value, fpair range)
    {
        if (range.first != NILMIN && range.second != NILMAX) return range.first <= value && value <= range.second;
        else if (range.first != NILMIN) return range.first <= value;
        else if (range.second != NILMAX) return value <= range.second;
        else return true;
    }

    // If this increases compile time by too much I'll revert it back to just int
    template<typename T>
    bool VectorContains(std::vector<T>& vec, T value)
    {
        return std::find(vec.begin(), vec.end(), value) != vec.end();
    }
}

class Part
{
public:
    std::string category;
    std::string name;

    int category_fast = 0;
    int name_fast = 0;

    float damage = 0;
    float fireRate = 0;
    float spread = 0;
    float recoil = 0;
    float reloadSpeed = 0;
    float movementSpeed = 0;
    float health = 0;
    float pellets = 0;
    float detectionRadius = 0;
    float range = 0;

    Part() {}
    Part(float defaultValue): damage(defaultValue), fireRate(defaultValue), spread(defaultValue), recoil(defaultValue), reloadSpeed(defaultValue), pellets(defaultValue), detectionRadius(defaultValue), range(defaultValue) {}
    Part(const json &jsonObject): category(jsonObject["Category"]), name(jsonObject["Name"])
    {
        category_fast = Fast::fastifyCategory[category];
        if (!Fast::fastifyName.contains(name)) Fast::fastifyName[name] = Fast::fastifyName.size();
        name_fast = Fast::fastifyName[name];

        if (jsonObject.contains("Damage"))
            damage = jsonObject["Damage"];
        if (jsonObject.contains("Fire_Rate"))
            fireRate = jsonObject["Fire_Rate"];
        if (jsonObject.contains("Spread"))
            spread = jsonObject["Spread"];
        if (jsonObject.contains("Recoil"))
            recoil = jsonObject["Recoil"];
        if (jsonObject.contains("Pellets"))
            pellets = jsonObject["Pellets"];
        if (jsonObject.contains("Movement_Speed"))
            movementSpeed = jsonObject["Movement_Speed"];
        if (jsonObject.contains("Reload_Speed"))
            reloadSpeed = jsonObject["Reload_Speed"];
        if (jsonObject.contains("Health"))
            health = jsonObject["Health"];
        if (jsonObject.contains("Detection_Radius"))
            detectionRadius = jsonObject["Detection_Radius"];
        if (jsonObject.contains("Range"))
            range = jsonObject["Range"];
    }

    float GetMult(Fast::MultFlags propertyFlag)
    {
        using namespace Fast;
        switch(propertyFlag)
        {
            case DAMAGE:
                return damage;
            case FIRERATE:
                return fireRate;
            case SPREADAIM:
            case SPREADHIP:
                return spread;
            case RECOILAIM:
            case RECOILHIP:
                return recoil;
            case RELOAD:
                return reloadSpeed;
            case MOVEMENTSPEEDMODIFIER:
                return movementSpeed;
            case HEALTH:
                return health;
            case PELLETS:
                return pellets;
            case DETECTIONRADIUS:
                return detectionRadius;
            case DROPOFFSTUDS:
                return range;
            default:
                throw std::invalid_argument("Property not found: " + std::to_string(propertyFlag));
        }
    }

};

class Barrel : public Part
{
public:
    Barrel() = default;
    Barrel(const json &jsonObject) : Part(jsonObject)
    {
    }
};

class Magazine : public Part
{
public:
    float reloadTime = 0;
    float magazineSize = 0; // This is technically supposed to be an int but it would be confusing to keep track off

    Magazine() = default;
    Magazine(const json &jsonObject) : Part(jsonObject)
    {
        if (jsonObject.contains("Reload_Time"))
            reloadTime = jsonObject["Reload_Time"];
        if (jsonObject.contains("Magazine_Size"))
            magazineSize = jsonObject["Magazine_Size"];
    }
};

class Grip : public Part
{
public:
    Grip() = default;
    Grip(const json &jsonObject) : Part(jsonObject)
    {
    }
};

class Stock : public Part
{
public:
    Stock() = default;
    Stock(const json &jsonObject) : Part(jsonObject)
    {
    }
};

class Core
{
public:
    std::string category;
    std::string name;

    int category_fast;
    int name_fast;

    fpair damage = fpair(0, 0);
    fpair dropoffStuds = fpair(0, 0);
    float fireRate = 0;
    float hipfireSpread = 0;
    float adsSpread = 0;
    float timeToAim = 0;
    float movementSpeedModifier = 0;
    float health = 0;
    float pellets = 1;
    float detectionRadius = 0;
    fpair recoilHipHorizontal = fpair(0, 0);
    fpair recoilHipVertical = fpair(0, 0);
    fpair recoilAimHorizontal = fpair(0, 0);
    fpair recoilAimVertical = fpair(0, 0);

    Core() {}
    Core(const json &jsonObject) : category(jsonObject["Category"]), name(jsonObject["Name"])
    {
        category_fast = Fast::fastifyCategory[category];
        if (!Fast::fastifyName.contains(name)) Fast::fastifyName[name] = Fast::fastifyName.size();
        name_fast = Fast::fastifyName[name];

        if (!jsonObject.contains("Damage"))
        {}
        else if (jsonObject["Damage"].is_array())
        {
            damage.first = jsonObject["Damage"][0];
            damage.second = jsonObject["Damage"][1];
        }
        else
        {
            damage.first = jsonObject["Damage"];
            damage.second = jsonObject["Damage"];
        }

        if (jsonObject.contains("Dropoff_Studs"))
        {
            dropoffStuds.first = jsonObject["Dropoff_Studs"][0];
            dropoffStuds.second = jsonObject["Dropoff_Studs"][1];
        }

        if (jsonObject.contains("Fire_Rate"))
            fireRate = jsonObject["Fire_Rate"];

        if (jsonObject.contains("Hipfire_Spread"))
            hipfireSpread = jsonObject["Hipfire_Spread"];
        if (jsonObject.contains("ADS_Spread"))
            adsSpread = jsonObject["ADS_Spread"];
        if (jsonObject.contains("Time_To_Aim"))
            timeToAim = jsonObject["Time_To_Aim"];
        if (jsonObject.contains("Movement_Speed_Modifier"))
            movementSpeedModifier = jsonObject["Movement_Speed_Modifier"];
        if (jsonObject.contains("Pellets"))
            pellets = jsonObject["Pellets"];
        if (jsonObject.contains("Detection_Radius"))
            detectionRadius = jsonObject["Detection_Radius"];

        if (jsonObject.contains("Recoil_Hip_Horizontal"))
        {
            recoilHipHorizontal.first = jsonObject["Recoil_Hip_Horizontal"][0];
            recoilHipHorizontal.second = jsonObject["Recoil_Hip_Horizontal"][1];
        }
        if (jsonObject.contains("Recoil_Hip_Vertical"))
        {
            recoilHipVertical.first = jsonObject["Recoil_Hip_Vertical"][0];
            recoilHipVertical.second = jsonObject["Recoil_Hip_Vertical"][1];
        }
        if (jsonObject.contains("Recoil_Aim_Horizontal"))
        {
            recoilAimHorizontal.first = jsonObject["Recoil_Aim_Horizontal"][0];
            recoilAimHorizontal.second = jsonObject["Recoil_Aim_Horizontal"][1];
        }
        if (jsonObject.contains("Recoil_Aim_Vertical"))
        {
            recoilAimVertical.first = jsonObject["Recoil_Aim_Vertical"][0];
            recoilAimVertical.second = jsonObject["Recoil_Aim_Vertical"][1];
        }

    }
};


class Gun
{
public:
    Barrel *barrel = nullptr;
    Magazine *magazine = nullptr;
    Grip *grip = nullptr;
    Stock *stock = nullptr;
    Core *core = nullptr;

    fpair damage;
    fpair dropoffStuds;
    float pellets;
    float reloadTime;
    float magazineSize;
    float health;
    float fireRate;
    float timeToAim;
    float movementSpeedModifier;
    float hipfireSpread;
    float adsSpread;
    float detectionRadius;
    fpair recoilHipHorizontal;
    fpair recoilHipVertical;
    fpair recoilAimHorizontal;
    fpair recoilAimVertical;

    Gun() {}
    Gun(Core *core) : core(core) {}
    Gun(Barrel *barrel, Magazine *magazine, Grip *grip, Stock *stock, Core *core) : barrel(barrel), magazine(magazine), grip(grip), stock(stock), core(core) {}


    float CalculatePenalty(Fast::MultFlags propertyFlag, Part *part)
    {
        float baseMult = part->GetMult(propertyFlag);
        if (part->name_fast == core->name_fast) // This actually evaluates first weirdly enough (This also technically can never return due to iterator optimizations)
            return 0;
        if (!Fast::MoreIsBetter(propertyFlag) && baseMult > 0)
            return baseMult;
        if (Fast::MoreIsBetter(propertyFlag) && baseMult < 0)
            return baseMult;

        return baseMult * Fast::penalties[core->category_fast][part->category_fast];
    }

    float GetTotalMult(Fast::MultFlags propertyFlag)
    {
        float mult = 1;
        if (barrel != nullptr)
            mult *= CalculatePenalty(propertyFlag, barrel) / 100 + 1;
        if (magazine != nullptr)
            mult *= CalculatePenalty(propertyFlag, magazine) / 100 + 1;
        if (grip != nullptr)
            mult *= CalculatePenalty(propertyFlag, grip) / 100 + 1;
        if (stock != nullptr)
            mult *= CalculatePenalty(propertyFlag, stock) / 100 + 1;
        return mult;
    }

    float GetTotalAdd(Fast::MultFlags propertyFlag) // Speed is additive (bruh)
    {
        float add = 0;
        if (barrel != nullptr)
            add += CalculatePenalty(propertyFlag, barrel);
        if (magazine != nullptr)
            add += CalculatePenalty(propertyFlag, magazine);
        if (grip != nullptr)
            add += CalculatePenalty(propertyFlag, grip);
        if (stock != nullptr)
            add += CalculatePenalty(propertyFlag, stock);
        return add;
    }

    // Core and magazine copy functions are seperated for greedy algorithm
    void CopyCoreValues(uint32_t flags)
    {
        using namespace Fast;
        if (flags & DAMAGE) damage = core->damage;
        if (flags & PELLETS) pellets = core->pellets;
        if (flags & DROPOFFSTUDS) dropoffStuds = core->dropoffStuds;
        if (flags & FIRERATE) fireRate = core->fireRate;
        if (flags & TIMETOAIM) timeToAim = core->timeToAim;
        if (flags & MOVEMENTSPEEDMODIFIER) movementSpeedModifier = core->movementSpeedModifier;
        if (flags & HEALTH) health = core->health;
        if (flags & DETECTIONRADIUS) detectionRadius = core->detectionRadius;
        if (flags & SPREADAIM) adsSpread = core->adsSpread;
        if (flags & SPREADHIP) hipfireSpread = core->hipfireSpread;
        if (flags & RECOILHIP)
        {
            recoilHipHorizontal = core->recoilHipHorizontal;
            recoilHipVertical = core->recoilHipVertical;
        }
        if (flags & RECOILAIM)
        {
            recoilAimHorizontal = core->recoilAimHorizontal;
            recoilAimVertical = core->recoilAimVertical;
        }
    }

    void CopyMagazineValues(uint32_t flags)
    {
        using namespace Fast;
        if (flags & MAGAZINESIZE) magazineSize = magazine->magazineSize;
        if (flags & RELOAD) reloadTime = magazine->reloadTime;
    }

    void CopyValues(uint32_t flags)
    {
        if (core != nullptr) CopyCoreValues(flags);
        if (magazine != nullptr) CopyMagazineValues(flags);
    }

    void CalculateGunStats(uint32_t flags)
    {
        using namespace Fast;
        if (flags & PELLETS)
        {
            pellets *= GetTotalMult(PELLETS);
            pellets = ceilf(pellets);
        }
        if (flags & DAMAGE)
        {
            // Current best estimate for pellet damage calculation
            float weirdAssMultThing = powf(powf(1.01, pellets) * powf(0.9925, pellets-1), 5);
            damage.first *= GetTotalMult(DAMAGE) * weirdAssMultThing;
            damage.second *= GetTotalMult(DAMAGE) * weirdAssMultThing;
        }
        if (flags & FIRERATE) fireRate *= GetTotalMult(FIRERATE);
        if (flags & MOVEMENTSPEEDMODIFIER) movementSpeedModifier += GetTotalAdd(MOVEMENTSPEEDMODIFIER); // Bruhhhhhhhhh
        if (flags & HEALTH) health += GetTotalAdd(HEALTH);
        if (flags & RELOAD) reloadTime *= GetTotalMult(RELOAD);
        if (flags & DETECTIONRADIUS) detectionRadius *= GetTotalMult(DETECTIONRADIUS);
        if (flags & SPREADAIM) adsSpread *= GetTotalMult(SPREADAIM);
        if (flags & SPREADHIP) hipfireSpread *= GetTotalMult(SPREADHIP);
        if (flags & DROPOFFSTUDS)
        {
            dropoffStuds.first *= GetTotalMult(DROPOFFSTUDS);
            dropoffStuds.second *= GetTotalMult(DROPOFFSTUDS);
        }
        if (flags & RECOILHIP)
        {
            float recoilMult = GetTotalMult(RECOILHIP);
            recoilHipHorizontal.first *= recoilMult;
            recoilHipHorizontal.second *= recoilMult;
            recoilHipVertical.first *= recoilMult;
            recoilHipVertical.second *= recoilMult;
        }
        if (flags & RECOILAIM)
        {
            float recoilMult = GetTotalMult(RECOILAIM);
            recoilAimHorizontal.first *= recoilMult;
            recoilAimHorizontal.second *= recoilMult;
            recoilAimVertical.first *= recoilMult;
            recoilAimVertical.second *= recoilMult;
        }
    }

    void CopyAllValues()
    {
        CopyValues(ALLMULTFLAG);
    }

    void CalculateAllGunStats()
    {
        CalculateGunStats(ALLMULTFLAG);
    }

    // Used by Prune algorithm //
    float GetPartialMult(Fast::MultFlags propertyFlag, Part *part)
    {
        return CalculatePenalty(propertyFlag, part) / 100 + 1;
    }

    float GetPartialAdd(Fast::MultFlags propertyFlag, Part *part)
    {
        return CalculatePenalty(propertyFlag, part);
    }

    void CalculatePartialGunStats(uint32_t flags, Part *part)
    {
        using namespace Fast;
        if (flags & PELLETS)
            pellets *= GetPartialMult(PELLETS, part);
        if (flags & DAMAGE)
        {
            damage.first *= GetPartialMult(DAMAGE, part) * powf(1.01, 1.25);
            damage.second *= GetPartialMult(DAMAGE, part) * powf(1.01, 1.25);
        }
        if (flags & FIRERATE) fireRate *= GetPartialMult(FIRERATE, part);
        if (flags & MOVEMENTSPEEDMODIFIER) movementSpeedModifier += GetPartialAdd(MOVEMENTSPEEDMODIFIER, part);
        if (flags & HEALTH) health += GetPartialAdd(HEALTH, part);
        if (flags & RELOAD) reloadTime *= GetPartialMult(RELOAD, part);
        if (flags & DETECTIONRADIUS) detectionRadius *= GetPartialMult(DETECTIONRADIUS, part);
        if (flags & SPREADAIM) adsSpread *= GetPartialMult(SPREADAIM, part);
        if (flags & SPREADHIP) hipfireSpread *= GetPartialMult(SPREADHIP, part);
        if (flags & DROPOFFSTUDS)
        {
            dropoffStuds.first *= GetPartialMult(DROPOFFSTUDS, part);
            dropoffStuds.second *= GetPartialMult(DROPOFFSTUDS, part);
        }
        if (flags & RECOILHIP)
        {
            float recoilMult = GetPartialMult(RECOILHIP, part);
            recoilHipHorizontal.first *= recoilMult;
            recoilHipHorizontal.second *= recoilMult;
            recoilHipVertical.first *= recoilMult;
            recoilHipVertical.second *= recoilMult;
        }
        if (flags & RECOILAIM)
        {
            float recoilMult = GetPartialMult(RECOILAIM, part);
            recoilAimHorizontal.first *= recoilMult;
            recoilAimHorizontal.second *= recoilMult;
            recoilAimVertical.first *= recoilMult;
            recoilAimVertical.second *= recoilMult;
        }
    }
    // Used by Prune algorithm //
};

float CalcTTK(const Gun &gun)
{
    return (ceil(100 / gun.damage.first) - 1) / gun.fireRate * 60;
}

std::ostream &operator<<(std::ostream &os, const fpair &fp)
{
    os << fp.first << " - " << fp.second;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Gun &gun)
{
    std::string barrelName = gun.barrel ? gun.barrel->name : "None";
    std::string magazineName = gun.magazine ? gun.magazine->name : "None";
    std::string gripName = gun.grip ? gun.grip->name : "None";
    std::string stockName = gun.stock ? gun.stock->name : "None";
    std::string coreName = gun.core ? gun.core->name : "None";

    os << "Gun contains"
       << "[ Barrel: " << barrelName << ", Magazine: " << magazineName << ", Grip: " << gripName << ", Stock: " << stockName << ", Core: " << coreName << " ]\n"
       << "damage: " << gun.damage << "\n"
       << "dropoffStuds: " << gun.dropoffStuds << "\n"
       << "pellets: " << gun.pellets << "\n"
       << "fireRate: " << gun.fireRate << "\n"
       << "adsSpread: " << gun.adsSpread << "\n"
       << "hipfireSpread: " << gun.hipfireSpread << "\n"
       << "reloadTime: " << gun.reloadTime << "\n"
       << "magazineSize: " << gun.magazineSize << "\n"
       << "health: " << gun.health << "\n"
       << "recoilHipVertical: " << gun.recoilHipVertical << "\n"
       << "recoilAimVertical: " << gun.recoilAimVertical << "\n"
       << "movementSpeed: " << gun.movementSpeedModifier << "\n"
       << "detectionRadius: " << gun.detectionRadius << "\n"
       << "TTK: " << CalcTTK(gun) << " Seconds\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Part &part)
{
    os << "Name: " << part.name << "\n"
       << "Damage: " << part.damage << "\n"
       << "Range: " << part.range << "\n"
       << "Fire Rate: " << part.fireRate << "\n"
       << "Spread: " << part.spread << "\n"
       << "Recoil: " << part.recoil << "\n"
       << "Movement Speed: " << part.movementSpeed << "\n"
       << "Reload Speed: " << part.reloadSpeed << "\n"
       << "Health: " << part.health << "\n"
       << "Pellets: " << part.pellets << "\n"
       << "Detection Radius: " << part.detectionRadius << "\n";
    return os;
}

std::ostream &operator<<(std::ostream &os, const std::pair<Part, Part> &pair)
{
    os << pair.first << "\n"
       << pair.second << "\n";
    return os;
}

Part operator*(Part part1, Part part2) // Used in greedy
{
    part1.damage *= part2.damage;
    part1.fireRate *= part2.fireRate;
    part1.spread *= part2.spread;
    part1.recoil *= part2.recoil;
    part1.movementSpeed += part2.movementSpeed;
    part1.reloadSpeed *= part2.reloadSpeed;
    part1.health += part2.health;
    part1.pellets *= part2.pellets;
    part1.detectionRadius *= part2.detectionRadius;
    part1.range *= part2.range;
    return part1;
}


/*
IMPORTANT: SORTING NEEDS TO BE THE INVERSE OF WHAT IS NORMALLY CONSIDERED GOOD
THE CODE WILL BE POPPING THE !WORST! GUNS OUT FIRST ONLY KEEPING TOP 10
SORTING NEEDS TO BE LIKE THIS :
    return gun1 < gun2 (Less is better)
    return gun1 > gun2 (More is better)
*/

struct SortByTTK{
    bool operator()(const Gun& gun1, const Gun& gun2) const {
        return CalcTTK(gun1) < CalcTTK(gun2);
    }
};

struct SortByFireRate{
    bool operator()(const Gun& gun1, const Gun& gun2) const {
        return gun1.fireRate > gun2.fireRate;
    }
};

struct SortByADSSpread{
    bool operator()(const Gun& gun1, const Gun& gun2) const {
        return gun1.adsSpread < gun2.adsSpread;
    }
};


int barrelCount;
int magazineCount;
int gripCount;
int stockCount;
int coreCount;
uint64_t totalCombinations;

Barrel barrelList[128];
Magazine magazineList[128];
Grip gripList[128];
Stock stockList[128];
Core coreList[128];


namespace PQ
{
    enum SortingType {
        SORTBYTTK = 0,
        SORTBYFIRERATE = 1,
        SORTBYSPREAD = 2
    };

    SortingType currentSortingType = SORTBYTTK;
    typedef std::priority_queue<Gun, std::vector<Gun>, SortByTTK> TTK;
    typedef std::priority_queue<Gun, std::vector<Gun>, SortByFireRate> FireRate;
    typedef std::priority_queue<Gun, std::vector<Gun>, SortByADSSpread> Spread;
    typedef std::variant< TTK, FireRate, Spread > Variant;

    Variant topGuns;

    void SetCurrentSortingType(std::string type)
    {
        if (type == "TTK")
            currentSortingType = SORTBYTTK;
        else if (type == "FIRERATE")
            currentSortingType = SORTBYFIRERATE;
        else if (type == "SPREAD")
            currentSortingType = SORTBYSPREAD;
        else
            throw std::runtime_error("Invalid sorting type");
    }

    bool HasMember(Variant& pq)
    {
        switch (PQ::currentSortingType)
        {
            case SORTBYTTK:
                return std::get<TTK>(pq).size() > 0;
            case SORTBYFIRERATE:
                return std::get<FireRate>(pq).size() > 0;
            case SORTBYSPREAD:
                return std::get<Spread>(pq).size() > 0;
            default:
                throw std::runtime_error("Invalid sorting type");
        }
    }


    Variant Create()
    {
        switch(currentSortingType)
        {
            case SORTBYTTK:
                return TTK();
            case SORTBYFIRERATE:
                return FireRate();
            case SORTBYSPREAD:
                return Spread();
            default:
                throw std::runtime_error("Invalid sorting type");
        }
    }

    Gun Pop(Variant& pq)
    {
        switch(currentSortingType)
        {
            case SORTBYTTK:
            {
                TTK& pqttk = std::get<TTK>(pq);
                Gun gun = pqttk.top();
                pqttk.pop();
                return gun;
            }
            case SORTBYFIRERATE:
            {
                FireRate& pqfireRate = std::get<FireRate>(pq);
                Gun gun = pqfireRate.top();
                pqfireRate.pop();
                return gun;
            }
            case SORTBYSPREAD:
            {
                Spread& pqspread = std::get<Spread>(pq);
                Gun gun = pqspread.top();
                pqspread.pop();
                return gun;
            }
            default:
                throw std::runtime_error("Invalid sorting type");
        }
    }


    void Push(Variant& pq, const Gun& gun)
    {
        switch(currentSortingType)
        {
            case SORTBYTTK:
            {
                TTK& pqttk = std::get<TTK>(pq);
                pqttk.push(gun);
                if (pqttk.size() > Input::howManyTopGunsToDisplay) pqttk.pop();
                break;
            }
            case SORTBYFIRERATE:
            {
                FireRate& pqfireRate = std::get<FireRate>(pq);
                pqfireRate.push(gun);
                if (pqfireRate.size() > Input::howManyTopGunsToDisplay) pqfireRate.pop();
                break;
            }
            case SORTBYSPREAD:
            {
                Spread& pqspread = std::get<Spread>(pq);
                pqspread.push(gun);
                if (pqspread.size() > Input::howManyTopGunsToDisplay) pqspread.pop();
                break;
            }
            default:
                throw std::runtime_error("Invalid sorting type");
        }
    }

    int Size(Variant& pq)
    {
        switch(currentSortingType)
        {
            case SORTBYTTK:
                return std::get<TTK>(pq).size();
            case SORTBYFIRERATE:
                return std::get<FireRate>(pq).size();
            case SORTBYSPREAD:
                return std::get<Spread>(pq).size();
            default:
                throw std::runtime_error("Invalid sorting type");
        }
    }
}

PQ::Variant threadPQ[16];
std::thread threads[16];
int validGunInThread[16];

namespace BruteForce
{
    using namespace Fast;
    class Iterator
    {
    public:

        int barrelIndex = 0;
        int magazineIndex = 0;
        int gripIndex = 0;
        int stockIndex = 0;
        int coreIndex = 0;
        int threadId = 0;

        Iterator()
        { }
        Iterator(int threadId): threadId(threadId), coreIndex(threadId)
        { }

        bool Step(Barrel*& b, Magazine*& m, Grip*& g, Stock*& s, Core*& c) // Returns true if another combination still exists
        {
            b = barrelList + barrelIndex;
            m = magazineList + magazineIndex;
            g = gripList + gripIndex;
            s = stockList + stockIndex;
            c = coreList + coreIndex;

            int coreName = coreList[coreIndex].name_fast;

            do {
                barrelIndex++;
            } while (barrelIndex < barrelCount && barrelList[barrelIndex].name_fast == coreName);

            if (barrelIndex < barrelCount) return true;

            barrelIndex = 0;
            do {
                magazineIndex++;
            } while (magazineIndex < magazineCount && magazineList[magazineIndex].name_fast == coreName);

            if (magazineIndex < magazineCount) return true;

            magazineIndex = 0;
            do {
                gripIndex++;
            } while (gripIndex < gripCount && gripList[gripIndex].name_fast == coreName);

            if (gripIndex < gripCount) return true;

            gripIndex = 0;
            do {
                stockIndex++;
            } while (stockIndex < stockCount && stockList[stockIndex].name_fast == coreName);

            if (stockIndex < stockCount) return true;

            stockIndex = 0;
            printf("Processed core index: %d\n", coreIndex);

            do {
                coreIndex++;
            } while (coreIndex % Input::threadsToMake != threadId);

            return coreIndex < coreCount;
        }
    };

    namespace Filter
    {
        uint32_t currentflags = 0;
        void InitializeMultFlag()
        {
            fpair nilrange = NILRANGE;
            if (Input::pelletRange != nilrange) currentflags |= PELLETS;
            if (Input::damageRange != nilrange) currentflags |= DAMAGE | PELLETS;
            if (Input::magazineRange != nilrange) currentflags |= MAGAZINESIZE;
            if (Input::movementSpeedRange != nilrange) currentflags |= MOVEMENTSPEEDMODIFIER;
            if (Input::spreadHipRange != nilrange) currentflags |= SPREADHIP;
            if (Input::spreadAimRange != nilrange) currentflags |= SPREADAIM;
            if (Input::recoilHipRange != nilrange) currentflags |= RECOILHIP;
            if (Input::recoilAimRange != nilrange) currentflags |= RECOILAIM;
            if (Input::fireRateRange != nilrange) currentflags |= FIRERATE;
            if (Input::healthRange != nilrange) currentflags |= HEALTH;
            if (Input::reloadRange != nilrange) currentflags |= RELOAD;
            if (Input::detectionRadiusRange != nilrange) currentflags |= DETECTIONRADIUS;
            if (Input::dropoffStudsRange != nilrange) currentflags |= DROPOFFSTUDS;
            switch (PQ::currentSortingType)
            {
                case PQ::SORTBYTTK:
                    currentflags |= DAMAGE | PELLETS;
                case PQ::SORTBYFIRERATE:
                    currentflags |= FIRERATE;
                    break;
                case PQ::SORTBYSPREAD:
                    currentflags |= SPREADAIM;
                    break;
            }
        }

        bool PreFilter(Barrel* b, Magazine* m, Grip* g, Stock* s, Core* c)
        {
            using Fast::RangeFilter;

            // Make sure the parts are the selected parts
            if (Fast::forceParts_fast[0] != NILINT && b->name_fast != Fast::forceParts_fast[0]) return false;
            if (Fast::forceParts_fast[1] != NILINT && m->name_fast != Fast::forceParts_fast[1]) return false;
            if (Fast::forceParts_fast[2] != NILINT && g->name_fast != Fast::forceParts_fast[2]) return false;
            if (Fast::forceParts_fast[3] != NILINT && s->name_fast != Fast::forceParts_fast[3]) return false;
            if (Fast::forceParts_fast[4] != NILINT && c->name_fast != Fast::forceParts_fast[4]) return false;

            if (VectorContains(Fast::banParts_fast[0], b->name_fast)) return false;
            if (VectorContains(Fast::banParts_fast[1], m->name_fast)) return false;
            if (VectorContains(Fast::banParts_fast[2], g->name_fast)) return false;
            if (VectorContains(Fast::banParts_fast[3], s->name_fast)) return false;
            if (VectorContains(Fast::banParts_fast[4], c->name_fast)) return false;

            if (!Fast::includeCategories_fast[c->category_fast]) return false;
            if (!RangeFilter(c->timeToAim, Input::timeToAimRange)) return false;
            if (!RangeFilter(m->magazineSize, Input::magazineRange)) return false;
            return true;
        }

        bool PostFilter(Gun& gun)
        {
            using Fast::RangeFilter;
            if (!RangeFilter(gun.damage.first, Input::damageRange)) return false;
            if (!RangeFilter(gun.adsSpread, Input::spreadAimRange)) return false;
            if (!RangeFilter(gun.hipfireSpread, Input::spreadHipRange)) return false;
            if (!RangeFilter(gun.recoilAimVertical.second, Input::recoilAimRange)) return false;
            if (!RangeFilter(gun.recoilHipVertical.second, Input::recoilHipRange)) return false;
            if (!RangeFilter(gun.movementSpeedModifier, Input::movementSpeedRange)) return false;
            if (!RangeFilter(gun.fireRate, Input::fireRateRange)) return false;
            if (!RangeFilter(gun.health, Input::healthRange)) return false;
            if (!RangeFilter(gun.pellets, Input::pelletRange)) return false;
            if (!RangeFilter(gun.reloadTime, Input::reloadRange)) return false;
            if (!RangeFilter(gun.detectionRadius, Input::detectionRadiusRange)) return false;
            if (!RangeFilter(gun.dropoffStuds.first, Input::dropoffStudsRange)) return false;
            return true;
        }

    }


    void Run(int threadId)
    {
        Barrel* b; Magazine* m; Grip* g; Stock* s; Core* c;

        printf("Starting thread: %d\n", threadId);

        threadPQ[threadId] = PQ::Create();
        Iterator iter(threadId);

        while (iter.Step(b, m, g, s, c))
        {
            if (!Filter::PreFilter(b, m, g, s, c)) continue;

            Gun currentGun(b, m, g, s, c);
            currentGun.CopyValues(Filter::currentflags);
            currentGun.CalculateGunStats(Filter::currentflags);

            if (!Filter::PostFilter(currentGun)) continue;

            validGunInThread[threadId]++;
            PQ::Push(threadPQ[threadId], currentGun);
        }
    }
};

namespace Prune
{
    // Implement a new algorithm to calculate gun stats by pruning impossible combinations via the use of the highlow data structure
    // Iteration looks like this: core -> magazine -> barrel -> grip -> stock

    namespace Filter
    {
        using namespace Fast;
        uint32_t currentflags = 0;
        void InitializeMultFlag()
        {
            fpair nilrange = NILRANGE;
            if (Input::pelletRange != nilrange) currentflags |= PELLETS;
            if (Input::damageRange != nilrange) currentflags |= DAMAGE | PELLETS;
            if (Input::magazineRange != nilrange) currentflags |= MAGAZINESIZE;
            if (Input::movementSpeedRange != nilrange) currentflags |= MOVEMENTSPEEDMODIFIER;
            if (Input::spreadHipRange != nilrange) currentflags |= SPREADHIP;
            if (Input::spreadAimRange != nilrange) currentflags |= SPREADAIM;
            if (Input::recoilHipRange != nilrange) currentflags |= RECOILHIP;
            if (Input::recoilAimRange != nilrange) currentflags |= RECOILAIM;
            if (Input::fireRateRange != nilrange) currentflags |= FIRERATE;
            if (Input::healthRange != nilrange) currentflags |= HEALTH;
            if (Input::reloadRange != nilrange) currentflags |= RELOAD;
            if (Input::detectionRadiusRange != nilrange) currentflags |= DETECTIONRADIUS;
            if (Input::dropoffStudsRange != nilrange) currentflags |= DROPOFFSTUDS;
            switch (PQ::currentSortingType)
            {
                case PQ::SORTBYTTK:
                    currentflags |= DAMAGE | PELLETS;
                case PQ::SORTBYFIRERATE:
                    currentflags |= FIRERATE;
                    break;
                case PQ::SORTBYSPREAD:
                    currentflags |= SPREADAIM;
                    break;
            }
        }

        bool CoreFilter(Core *core) // These filters require no gun calculation and can be immediately checked on the part itself
        {
            if (VectorContains(Fast::banParts_fast[4], core->name_fast)) return false;
            if (Fast::forceParts_fast[4] != NILINT && Fast::forceParts_fast[4] != core->name_fast) return false;
            if (!Fast::includeCategories_fast[core->category_fast]) return false;
            if (!Fast::RangeFilter(core->timeToAim, Input::timeToAimRange)) return false;
            return true;
        }

        bool MagazineFilter(Magazine *magazine)
        {
            if (VectorContains(Fast::banParts_fast[1], magazine->name_fast)) return false;
            if (Fast::forceParts_fast[1] != NILINT && Fast::forceParts_fast[1] != magazine->name_fast) return false;
            if (!Fast::RangeFilter(magazine->magazineSize, Input::magazineRange)) return false;
            return true;
        }

        bool BarrelFilter(Barrel *barrel)
        {
            if (VectorContains(Fast::banParts_fast[0], barrel->name_fast)) return false;
            if (Fast::forceParts_fast[0] != NILINT && Fast::forceParts_fast[0] != barrel->name_fast) return false;
            return true;
        }

        bool GripFilter(Grip *grip)
        {
            if (VectorContains(Fast::banParts_fast[2], grip->name_fast)) return false;
            if (Fast::forceParts_fast[2] != NILINT && Fast::forceParts_fast[2] != grip->name_fast) return false;
            return true;
        }

        bool StockFilter(Stock *stock)
        {
            if (VectorContains(Fast::banParts_fast[3], stock->name_fast)) return false;
            if (Fast::forceParts_fast[3] != NILINT && Fast::forceParts_fast[3] != stock->name_fast) return false;
            return true;
        }

        bool FilterGunStats(const Gun& gun)
        {
            using Fast::RangeFilter;
            if (!Fast::includeCategories_fast[gun.core->category_fast]) return false;
            if (!RangeFilter(gun.timeToAim, Input::timeToAimRange)) return false;
            if (!RangeFilter(gun.magazineSize, Input::magazineRange)) return false;
            if (!RangeFilter(gun.damage.first, Input::damageRange)) return false;
            if (!RangeFilter(gun.adsSpread, Input::spreadAimRange)) return false;
            if (!RangeFilter(gun.hipfireSpread, Input::spreadHipRange)) return false;
            if (!RangeFilter(gun.recoilAimVertical.second, Input::recoilAimRange)) return false;
            if (!RangeFilter(gun.recoilHipVertical.second, Input::recoilHipRange)) return false;
            if (!RangeFilter(gun.movementSpeedModifier, Input::movementSpeedRange)) return false;
            if (!RangeFilter(gun.fireRate, Input::fireRateRange)) return false;
            if (!RangeFilter(gun.health, Input::healthRange)) return false;
            if (!RangeFilter(gun.pellets, Input::pelletRange)) return false;
            if (!RangeFilter(gun.reloadTime, Input::reloadRange)) return false;
            if (!RangeFilter(gun.detectionRadius, Input::detectionRadiusRange)) return false;
            if (!RangeFilter(gun.dropoffStuds.first, Input::dropoffStudsRange)) return false;
            return true;
        }
    }

    namespace HighLow
    {
        /*
        This namespace revolves around the key data structure used in the Prune algorithm.
        It revolves around bestPossibleCombo which is a three-dimensional array containing
        Part objects that contain the best multiplier combination both low and high, for
        every core category and from a 4 part combo to a 1 part combo.
        */

        // bestPossibleCombo[coreCategoryFast][low / high][1 -> 4]
        Part bestPossibleCombo[CATEGORYCOUNT][2][4] = { // stfu I don't wanna hear it
            {{Part(1), Part(1), Part(1), Part(1)}, {Part(1), Part(1), Part(1), Part(1)}},
            {{Part(1), Part(1), Part(1), Part(1)}, {Part(1), Part(1), Part(1), Part(1)}},
            {{Part(1), Part(1), Part(1), Part(1)}, {Part(1), Part(1), Part(1), Part(1)}},
            {{Part(1), Part(1), Part(1), Part(1)}, {Part(1), Part(1), Part(1), Part(1)}},
            {{Part(1), Part(1), Part(1), Part(1)}, {Part(1), Part(1), Part(1), Part(1)}},
            {{Part(1), Part(1), Part(1), Part(1)}, {Part(1), Part(1), Part(1), Part(1)}},
            {{Part(1), Part(1), Part(1), Part(1)}, {Part(1), Part(1), Part(1), Part(1)}}
        };

        void SetHighLowParts(Gun& gun, Part* partToCheck, Part& lowestMultPart, Part& highestMultPart)
        {
            using namespace Fast;
            if (gun.GetPartialMult(DAMAGE, partToCheck) < lowestMultPart.damage) lowestMultPart.damage = gun.GetPartialMult(DAMAGE, partToCheck);
            if (gun.GetPartialMult(FIRERATE, partToCheck) < lowestMultPart.fireRate) lowestMultPart.fireRate = gun.GetPartialMult(FIRERATE, partToCheck);
            if (gun.GetPartialMult(SPREADAIM, partToCheck) < lowestMultPart.spread) lowestMultPart.spread = gun.GetPartialMult(SPREADAIM, partToCheck);
            if (gun.GetPartialMult(RECOILAIM, partToCheck) < lowestMultPart.recoil) lowestMultPart.recoil = gun.GetPartialMult(RECOILAIM, partToCheck);
            if (gun.GetPartialMult(RELOAD, partToCheck) < lowestMultPart.reloadSpeed) lowestMultPart.reloadSpeed = gun.GetPartialMult(RELOAD, partToCheck);
            if (gun.GetPartialAdd(MOVEMENTSPEEDMODIFIER, partToCheck) < lowestMultPart.movementSpeed) lowestMultPart.movementSpeed = gun.GetPartialAdd(MOVEMENTSPEEDMODIFIER, partToCheck);
            if (gun.GetPartialAdd(HEALTH, partToCheck) < lowestMultPart.health) lowestMultPart.health = gun.GetPartialAdd(HEALTH, partToCheck);
            if (gun.GetPartialMult(PELLETS, partToCheck) < lowestMultPart.pellets) lowestMultPart.pellets = gun.GetPartialMult(PELLETS, partToCheck);
            if (gun.GetPartialMult(DETECTIONRADIUS, partToCheck) < lowestMultPart.detectionRadius) lowestMultPart.detectionRadius = gun.GetPartialMult(DETECTIONRADIUS, partToCheck);
            if (gun.GetPartialMult(DROPOFFSTUDS, partToCheck) < lowestMultPart.range) lowestMultPart.range = gun.GetPartialMult(DROPOFFSTUDS, partToCheck);

            if (gun.GetPartialMult(DAMAGE, partToCheck) > highestMultPart.damage) highestMultPart.damage = gun.GetPartialMult(DAMAGE, partToCheck);
            if (gun.GetPartialMult(FIRERATE, partToCheck) > highestMultPart.fireRate) highestMultPart.fireRate = gun.GetPartialMult(FIRERATE, partToCheck);
            if (gun.GetPartialMult(SPREADAIM, partToCheck) > highestMultPart.spread) highestMultPart.spread = gun.GetPartialMult(SPREADAIM, partToCheck);
            if (gun.GetPartialMult(RECOILAIM, partToCheck) > highestMultPart.recoil) highestMultPart.recoil = gun.GetPartialMult(RECOILAIM, partToCheck);
            if (gun.GetPartialMult(RELOAD, partToCheck) > highestMultPart.reloadSpeed) highestMultPart.reloadSpeed = gun.GetPartialMult(RELOAD, partToCheck);
            if (gun.GetPartialAdd(MOVEMENTSPEEDMODIFIER, partToCheck) > highestMultPart.movementSpeed) highestMultPart.movementSpeed = gun.GetPartialAdd(MOVEMENTSPEEDMODIFIER, partToCheck);
            if (gun.GetPartialAdd(HEALTH, partToCheck) > highestMultPart.health) highestMultPart.health = gun.GetPartialAdd(HEALTH, partToCheck);
            if (gun.GetPartialMult(PELLETS, partToCheck) > highestMultPart.pellets) highestMultPart.pellets = gun.GetPartialMult(PELLETS, partToCheck);
            if (gun.GetPartialMult(DETECTIONRADIUS, partToCheck) > highestMultPart.detectionRadius) highestMultPart.detectionRadius = gun.GetPartialMult(DETECTIONRADIUS, partToCheck);
            if (gun.GetPartialMult(DROPOFFSTUDS, partToCheck) > highestMultPart.range) highestMultPart.range = gun.GetPartialMult(DROPOFFSTUDS, partToCheck);
        }

        // Polymorphism is required due to the fact that sizeof(Magazine) is more than sizeof(Part) causing pointer arithmetic to be incorrect when iterating through the array.
        template <typename T>
        std::pair<Part, Part> FindHighestAndLowestMultInList(Gun& gun, T* partList, int size)
        {
            Part lowestMultPart(1), highestMultPart(1);
            for (int i = 0; i < size; i++)
                SetHighLowParts(gun, partList + i, lowestMultPart, highestMultPart);
            return std::pair<Part, Part>(lowestMultPart, highestMultPart);
        }

        void InitializeHighestAndLowestMultParts() // It may not look like it but I promise you this has a Time complexity of O(n) I'M TELLING YOU PLEASE BELIEVE ME
        {
            // Creation of dummyGuns to allow us to use (Gun obj).GetPartialMult and (Gun obj).GetPartialAdd for HighLow intialization
            Core coreCategories[CATEGORYCOUNT];
            Gun dummyGuns[CATEGORYCOUNT];
            for (int i = 0; i < CATEGORYCOUNT; i++)
            {
                coreCategories[i].category_fast = i;
                dummyGuns[i] = Gun(coreCategories + i);
            }

            for (int g = 0; g < CATEGORYCOUNT; g++)
            {
                Gun& dummyGun = dummyGuns[g];
                std::pair<Part, Part> magazineMultPair = FindHighestAndLowestMultInList(dummyGun, magazineList, magazineCount);
                std::pair<Part, Part> barrelMultPair = FindHighestAndLowestMultInList(dummyGun, barrelList, barrelCount);
                std::pair<Part, Part> gripMultPair = FindHighestAndLowestMultInList(dummyGun, gripList, gripCount);
                std::pair<Part, Part> stockMultPair = FindHighestAndLowestMultInList(dummyGun, stockList, stockCount);

                std::pair<Part, Part> *partPairs[4] = {&stockMultPair, &gripMultPair, &barrelMultPair, &magazineMultPair};
                for (int i = 0; i < 4; i++)
                {
                    for (int j = i; j < 4; j++)
                    {
                        // No *= because I said so
                        bestPossibleCombo[g][0][j] = bestPossibleCombo[g][0][j] * partPairs[i]->first;
                        bestPossibleCombo[g][1][j] = bestPossibleCombo[g][1][j] * partPairs[i]->second;

                        // Pellet damage compensation (ONLY WORKS FOR 1 PELLET)
                        bestPossibleCombo[g][0][j].damage *= powf(1.01, 1.25);
                        bestPossibleCombo[g][1][j].damage *= powf(1.01, 1.25);
                    }
                }
            }
            // for (int i = 0; i < 6; i++) for (int j = 0; j < 2; j++) for (int k = 0; k < 4; k++) std::cout << bestPossibleCombo[i][j][k];
        }
    }

    bool RangeFilterMult(float valueToCompare, float lowMult, float highMult, const fpair &range)
    {
        if (range.first == NILMIN && range.second == NILMAX) return true;
        if (range.first == NILMIN && valueToCompare * lowMult <= range.second) return true;
        if (range.second == NILMAX && valueToCompare * highMult >= range.first) return true;
        return valueToCompare * highMult >= range.first && valueToCompare * lowMult <= range.second;
    }

    bool RangeFilterAdd(float valueToCompare, float lowAdd, float highAdd, const fpair &range)
    {
        if (range.first == NILMIN && range.second == NILMAX) return true;
        if (range.first == NILMIN && valueToCompare + lowAdd <= range.second) return true;
        if (range.second == NILMAX && valueToCompare + highAdd >= range.first) return true;
        return valueToCompare + highAdd >= range.first && valueToCompare + lowAdd <= range.second;
    }

    bool IsValidCombination(const Gun& gun, const Part& lowPPC, const Part& highPPC) // PPC Stands for PossiblePartCombo
    {
        using namespace Fast;
        if (!RangeFilterMult(gun.damage.first, lowPPC.damage, highPPC.damage, Input::damageRange)) return false;
        if (!RangeFilterMult(gun.fireRate, lowPPC.fireRate, highPPC.fireRate, Input::fireRateRange)) return false;
        if (!RangeFilterMult(gun.hipfireSpread, lowPPC.spread, highPPC.spread, Input::spreadHipRange)) return false;
        if (!RangeFilterMult(gun.adsSpread, lowPPC.spread, highPPC.spread, Input::spreadAimRange)) return false;
        if (!RangeFilterMult(gun.recoilHipVertical.second, lowPPC.recoil, highPPC.recoil, Input::recoilHipRange)) return false;
        if (!RangeFilterMult(gun.recoilAimVertical.second, lowPPC.recoil, highPPC.recoil, Input::recoilAimRange)) return false;
        if (!RangeFilterMult(gun.reloadTime, lowPPC.reloadSpeed, highPPC.reloadSpeed, Input::reloadRange)) return false;
        if (!RangeFilterMult(gun.detectionRadius, lowPPC.detectionRadius, highPPC.detectionRadius, Input::detectionRadiusRange)) return false;
        if (!RangeFilterMult(gun.dropoffStuds.first, lowPPC.range, highPPC.range, Input::dropoffStudsRange)) return false;
        if (!RangeFilterMult(gun.pellets, lowPPC.pellets, highPPC.pellets, Input::pelletRange)) return false;
        if (!RangeFilterAdd(gun.movementSpeedModifier, lowPPC.movementSpeed, highPPC.movementSpeed, Input::movementSpeedRange)) return false;
        if (!RangeFilterAdd(gun.health, lowPPC.health, highPPC.health, Input::healthRange)) return false;
        return true;
    }

    void CustomPushback(std::vector<Gun>& vec, uint64_t& index, const Gun& gun)
    {
        if (vec.size() <= index) vec.resize(vec.size() * 2);
        vec[index] = gun;
        index++;
    }

    void Run(int threadId)
    {
        printf("Thread %d started\n", threadId);
        threadPQ[threadId] = PQ::Create();

        // This is why memory scales with O(n^4) (Hopefully not a big issue hahhah)
        printf("%d: Allocating %lu bytes for vectors\n", threadId, sizeof(Gun) * DEFAULTVECTORSIZE / Input::threadsToMake * 2);
        std::vector<Gun> validGuns1(DEFAULTVECTORSIZE / Input::threadsToMake);
        std::vector<Gun> validGuns2(DEFAULTVECTORSIZE / Input::threadsToMake);
        uint64_t validGunCount1 = 0;
        uint64_t validGunCount2 = 0;

        // start with validGuns1 (Parsing core)
        for (int c = 0; c < coreCount; c++)
        {
            if (c % Input::threadsToMake != threadId) continue;
            if (!Filter::CoreFilter(coreList + c)) continue;

            Gun currentGun = Gun(coreList + c);
            currentGun.CopyCoreValues(Filter::currentflags);
            bool validCombo = IsValidCombination(currentGun, HighLow::bestPossibleCombo[currentGun.core->category_fast][0][3], HighLow::bestPossibleCombo[currentGun.core->category_fast][1][3]);
            if (validCombo)
                CustomPushback(validGuns1, validGunCount1, currentGun);
        }
        printf("%d: Total valid cores: %lu / %u\n", threadId, validGunCount1, coreCount);

        // validGuns1 -> validGuns2 (Parsing core + magazine)
        for (int g = 0; g < validGunCount1; g++)
        {
            for (int m = 0; m < magazineCount; m++)
            {
                if (!Filter::MagazineFilter(magazineList + m)) continue;

                Gun currentGun = validGuns1[g]; // Copies over from the old vector
                currentGun.magazine = magazineList + m;
                currentGun.CopyMagazineValues(Filter::currentflags);
                currentGun.CalculatePartialGunStats(Filter::currentflags, currentGun.magazine);
                bool validCombo = IsValidCombination(currentGun, HighLow::bestPossibleCombo[currentGun.core->category_fast][0][2], HighLow::bestPossibleCombo[currentGun.core->category_fast][1][2]);
                if (validCombo)
                    CustomPushback(validGuns2, validGunCount2, currentGun);
            }
        }
        printf("%d: Total valid core + mag: %lu / %u\n", threadId, validGunCount2, coreCount * magazineCount);
        validGunCount1 = 0;

        // validGuns2 -> validGuns1 (Parsing core + magazine + barrel)
        for (int g = 0; g < validGunCount2; g++)
        {
            for (int b = 0; b < barrelCount; b++)
            {
                if (!Filter::BarrelFilter(barrelList + b)) continue;
                Gun currentGun = validGuns2[g]; // Copies over from the old vector
                currentGun.barrel = barrelList + b;
                currentGun.CalculatePartialGunStats(Filter::currentflags, currentGun.barrel);
                bool validCombo = IsValidCombination(currentGun, HighLow::bestPossibleCombo[currentGun.core->category_fast][0][1], HighLow::bestPossibleCombo[currentGun.core->category_fast][1][1]);
                if (validCombo)
                    CustomPushback(validGuns1, validGunCount1, currentGun);
            }
        }
        printf("%d: Total valid core + mag + barrel: %lu / %u\n", threadId, validGunCount1, coreCount * magazineCount * barrelCount);
        validGunCount2 = 0;

        // validGuns1 -> validGuns2 (Parsing core + magazine + barrel + grip)
        for (int g = 0; g < validGunCount1; g++)
        {
            for (int gr = 0; gr < gripCount; gr++)
            {
                if (!Filter::GripFilter(gripList + gr)) continue;
                Gun currentGun = validGuns1[g]; // Copies over from the old vector
                currentGun.grip = gripList + gr;
                currentGun.CalculatePartialGunStats(Filter::currentflags, currentGun.grip);
                bool validCombo = IsValidCombination(currentGun, HighLow::bestPossibleCombo[currentGun.core->category_fast][0][0], HighLow::bestPossibleCombo[currentGun.core->category_fast][1][0]);
                if (validCombo)
                    CustomPushback(validGuns2, validGunCount2, currentGun);
            }
        }
        printf("%d: Total valid core + mag + barrel + grip: %lu / %u\n", threadId, validGunCount2, coreCount * magazineCount * barrelCount * gripCount);
        validGunCount1 = 0;

        // validGuns2 * stocks -> Outputs to PQ
        for (int g = 0; g < validGunCount2; g++)
        {
            for (int s = 0; s < stockCount; s++)
            {
                if (!Filter::StockFilter(stockList + s)) continue;
                Gun currentGun = validGuns2[g]; // Copies over from the old vector
                currentGun.stock = stockList + s;
                currentGun.CalculatePartialGunStats(Filter::currentflags, currentGun.stock);
                bool validCombo = Filter::FilterGunStats(currentGun); // This is essentially what IsValidCombination is if you pass through Parts with identity values (1 for mult 0 for add)
                if (validCombo)
                {
                    validGunCount1++;
                    validGunInThread[threadId]++;
                    PQ::Push(threadPQ[threadId], currentGun);
                    if (PQ::Size(threadPQ[threadId]) > Input::howManyTopGunsToDisplay)
                        PQ::Pop(threadPQ[threadId]);
                }
            }
        }
        printf("%d: Total valid core + mag + barrel + grip + stock: %lu / %u\n", threadId, validGunCount1, coreCount * magazineCount * barrelCount * gripCount * stockCount);
    }
}

int main(int argc, char* argv[])
{
    CLI::App app{"A tool to bruteforce all possible stats inside of weird gun game."};
    argv = app.ensure_utf8(argv);

    app.add_option("-f, --file", Input::fileDir, "Path to the directory containing the json file (Default: Data)");
    app.add_option("-o, --output", Input::outpath, "Path to the output file (Default: Results.txt");
    app.add_option("-t, --threads", Input::threadsToMake, "Number of threads to use (MAX 16) (Default: 4)");
    app.add_option("-s, --sort", Input::sortType, "Sorting type (TTK, FIRERATE, SPREAD) (Default: TTK)");
    app.add_option("-n, --number", Input::howManyTopGunsToDisplay, "Number of top guns to display (Default: 10)");
    app.add_option("-m, --method", Input::method, "Method to use for calculation (BRUTEFORCE, PRUNE) (Default: PRUNE)");
    app.add_option("-i, --include", Input::includeCategories, "Categories to include in the calculation (AR, Sniper, LMG, SMG, Shotgun, Weird)")->required();

    app.add_option("--fb, --forceBarrel", Input::forceBarrel, "Force the calculator to use a specific barrel");
    app.add_option("--fm, --forceMagazine", Input::forceMagazine, "Force the calculator to use a specific magazine");
    app.add_option("--fg, --forceGrip", Input::forceGrip, "Force the calculator to use a specific grip");
    app.add_option("--fs, --forceStock", Input::forceStock, "Force the calculator to use a specific stock");
    app.add_option("--fc, --forceCore", Input::forceCore, "Force the calculator to use a specific core");

    app.add_option("--bb, --banBarrel", Input::banBarrel, "Ban the calculator from using a list of barrels");
    app.add_option("--bm, --banMagazine", Input::banMagazine, "Ban the calculator from using a list of magazines");
    app.add_option("--bg, --banGrip", Input::banGrip, "Ban the calculator from using a list of grips");
    app.add_option("--bs, --banStock", Input::banStock, "Ban the calculator from using a list of stocks");
    app.add_option("--bc, --banCore", Input::banCore, "Ban the calculator from using a list of cores");

    app.add_option("--damage", Input::damageRange, "Damage range to filter");
    app.add_option("--damageMin", Input::damageRange.first);
    app.add_option("--damageMax", Input::damageRange.second);
    app.add_option("--magazine", Input::magazineRange, "Size of magazine to filter");
    app.add_option("--magazineMin", Input::magazineRange.first);
    app.add_option("--magazineMax", Input::magazineRange.second);
    app.add_option("--spreadHip", Input::spreadHipRange, "Spread range to filter (HIP)");
    app.add_option("--spreadHipMin", Input::spreadHipRange.first);
    app.add_option("--spreadHipMax", Input::spreadHipRange.second);
    app.add_option("--spreadAim", Input::spreadAimRange, "Spread range to filter (AIM)");
    app.add_option("--spreadAimMin", Input::spreadAimRange.first);
    app.add_option("--spreadAimMax", Input::spreadAimRange.second);
    app.add_option("--recoilHip", Input::recoilHipRange, "Recoil range to filter (HIP)");
    app.add_option("--recoilHipMin", Input::recoilHipRange.first);
    app.add_option("--recoilHipMax", Input::recoilHipRange.second);
    app.add_option("--recoilAim", Input::recoilAimRange, "Recoil range to filter (AIM)");
    app.add_option("--recoilAimMin", Input::recoilAimRange.first);
    app.add_option("--recoilAimMax", Input::recoilAimRange.second);
    app.add_option("--speed", Input::movementSpeedRange, "Movement speed range to filter");
    app.add_option("--speedMin", Input::movementSpeedRange.first);
    app.add_option("--speedMax", Input::movementSpeedRange.second);
    app.add_option("--fireRate", Input::fireRateRange, "Fire rate range to filter");
    app.add_option("--fireRateMin", Input::fireRateRange.first);
    app.add_option("--fireRateMax", Input::fireRateRange.second);
    app.add_option("--health", Input::healthRange, "Health range to filter");
    app.add_option("--healthMin", Input::healthRange.first);
    app.add_option("--healthMax", Input::healthRange.second);
    app.add_option("--pellet", Input::pelletRange, "Pellet range to filter");
    app.add_option("--pelletMin", Input::pelletRange.first);
    app.add_option("--pelletMax", Input::pelletRange.second);
    app.add_option("--timeToAim", Input::timeToAimRange, "Time to aim range to filter");
    app.add_option("--timeToAimMin", Input::timeToAimRange.first);
    app.add_option("--timeToAimMax", Input::timeToAimRange.second);
    app.add_option("--reload", Input::reloadRange, "Reload time to filter");
    app.add_option("--reloadMin", Input::reloadRange.first);
    app.add_option("--reloadMax", Input::reloadRange.second);
    app.add_option("--detectionRadius", Input::detectionRadiusRange, "Detection radius range to filter (Why would you even use this?)");
    app.add_option("--detectionRadiusMin", Input::detectionRadiusRange.first);
    app.add_option("--detectionRadiusMax", Input::detectionRadiusRange.second);
    app.add_option("--range", Input::dropoffStudsRange, "Dropoff studs range to filter (Could have called it range range haha)");
    app.add_option("--rangeMin", Input::dropoffStudsRange.first);
    app.add_option("--rangeMax", Input::dropoffStudsRange.second);

    CLI11_PARSE(app, argc, argv);

    // The operator overloads go c++azy (Python actually has this with Pathlib but we don't talk about that)
    std::filesystem::path fullDataPath = Input::fileDir / "FullData.json";
    std::filesystem::path penaltiesPath = Input::fileDir / "Penalties.json";

    if (!std::filesystem::exists(fullDataPath) || !std::filesystem::exists(penaltiesPath))
    {
        printf("Files %s and %s are required\n", fullDataPath.c_str(), penaltiesPath.c_str());
        throw std::invalid_argument("Required files not found");
    }

    printf("Loading from json file %s and %s\n", fullDataPath.c_str(), penaltiesPath.c_str());

    std::ifstream FullData(fullDataPath);
    json data = json::parse(FullData);

    for (json &element : data["Barrels"])
    {
        barrelList[barrelCount] = Barrel(element);
        barrelCount++;
    }

    for (json &element : data["Magazines"])
    {
        magazineList[magazineCount] = Magazine(element);
        magazineCount++;
    }

    for (json &element : data["Grips"])
    {
        gripList[gripCount] = Grip(element);
        gripCount++;
    }

    for (json &element : data["Stocks"])
    {
        stockList[stockCount] = Stock(element);
        stockCount++;
    }

    for (json &element : data["Cores"])
    {
        coreList[coreCount] = Core(element);
        coreCount++;
    }

    std::ifstream Penalties(penaltiesPath);
    json penalties = json::parse(Penalties);

    for (int i = 0; i < CATEGORYCOUNT; i++)
        for (int j = 0; j < CATEGORYCOUNT; j++)
            Fast::penalties[i][j] = penalties[i][j];

    puts("Initializing required data");
    PQ::SetCurrentSortingType(Input::sortType);
    Fast::InitializeIncludeCategories();
    Fast::InitializeForceAndBanParts();
    BruteForce::Filter::InitializeMultFlag();
    Prune::Filter::InitializeMultFlag();
    Prune::HighLow::InitializeHighestAndLowestMultParts();

    totalCombinations = barrelCount * magazineCount * gripCount * stockCount * coreCount;
    printf("Barrels detected: %d, Magazines detected: %d, Grips detected: %d, Stocks detected: %d, Cores detected: %d\n", barrelCount, magazineCount, gripCount, stockCount, coreCount);
    printf("Total of %lu possibilities \n", totalCombinations);
    printf("Starting %s with %d threads\n", Input::method.c_str(), Input::threadsToMake);

    auto start = chrono::steady_clock::now();

    // Start selected method
    if (Input::method == "PRUNE")
    {
        for (int threadId = 0; threadId < Input::threadsToMake; threadId++)
            threads[threadId] = std::thread(Prune::Run, threadId);
    }
    else if (Input::method == "BRUTEFORCE")
    {
        for (int threadId = 0; threadId < Input::threadsToMake; threadId++)
            threads[threadId] = std::thread(BruteForce::Run, threadId);
    }
    else {
        throw std::invalid_argument("Invalid method");
    }

    // Wait for all threads to finish
    for (int threadId = 0; threadId < Input::threadsToMake; threadId++)
        threads[threadId].join();

    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

    uint64_t total_us = duration.count();
    uint64_t minutes = total_us / 60000000;
    uint64_t seconds = (total_us % 60000000) / 1000000;
    uint64_t milliseconds = (total_us % 1000000) / 1000;
    uint64_t microseconds = total_us % 1000;

    puts("Bruteforce completed");
    std::cout << "Elapsed time: "
         << minutes << " minute(s), "
         << seconds << " second(s), "
         << milliseconds << " millisecond(s), and "
         << microseconds << " microsecond(s)\n";

    unsigned int totalValidGuns = 0;
    for (int i = 0; i < Input::threadsToMake; i++)
        totalValidGuns += validGunInThread[i];
    printf("Total valid gun combinations based on filters: %u / %u\n", totalValidGuns, coreCount * magazineCount * barrelCount * gripCount * stockCount);

    PQ::topGuns = PQ::Create();

    // Combine all thread pqs into topguns
    for (int i = 0; i < Input::threadsToMake; i++)
        for (int j = 0; j < Input::howManyTopGunsToDisplay && PQ::HasMember(threadPQ[i]); j++)
            PQ::Push(PQ::topGuns, PQ::Pop(threadPQ[i]));


    // Dump all the pqs into a vector
    std::vector<Gun> outputGuns;
    int max = PQ::Size(PQ::topGuns);
    for (int i = 0; i < max; i++)
        outputGuns.push_back(PQ::Pop(PQ::topGuns));

    std::string fullCommand;
    for (int i = 0; i < argc; i++)
    {
        fullCommand += argv[i];
        fullCommand += " ";
    }

    // Write to the file
    std::ofstream file(Input::outpath);
    file << fullCommand << "\n";
    file << "Current top " << Input::howManyTopGunsToDisplay << " guns are: \n";
    int endSize = outputGuns.size()-1-Input::howManyTopGunsToDisplay;
    // Reverse out the answer
    for (int i = outputGuns.size()-1; i >=0; i--)
    {
        if (i == endSize) break;
        Gun g = outputGuns[i];
        // std::cout << g << '\n';
        g.CopyAllValues();
        g.CalculateAllGunStats();
        file << g << '\n';
    }
    file.close();
}

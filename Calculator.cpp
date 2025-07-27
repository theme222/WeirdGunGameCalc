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
// #include <iomanip>
#include "include/json.hpp" // "nlohmann/json.hpp"
#include "include/CLI11.hpp" // "CLIUtils"

#define chrono std::chrono
using json = nlohmann::json;
using fpair = std::pair<float, float>;

#define DEFAULTVECTORSIZE 100000lu
#define ALLMULTFLAG 4294967295 // 2^32 - 1
#define NILMIN -69420.5f
#define NILMAX 69420.5f
#define NILRANGE {NILMIN, NILMAX} // Hehehheheh who says programmers can't have fun
#define NILINT -1

namespace Input
{
    std::filesystem::path fileDir = "Data";
    std::string outpath = "Results.txt";
    std::string sortType = "TTK";
    std::string method = "PRUNE";
    bool detailed = false;

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

    int threadsToMake = std::thread::hardware_concurrency() == 0 ? 4 : std::thread::hardware_concurrency();
    int howManyTopGunsToDisplay = 10;

    fpair damageRange = NILRANGE;
    fpair damage2Range = NILRANGE;
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
    fpair dropoffStuds2Range = NILRANGE;
    fpair burstRange = NILRANGE;

    // _adjusted ranges contain the range that has removed the pellet modifier (pelletRange_adjusted is unused in the main program)
    fpair pelletRange_adjusted = NILRANGE;
    fpair damageRange_adjusted = NILRANGE;
    fpair damage2Range_adjusted = NILRANGE;
    fpair spreadHipRange_adjusted = NILRANGE;
    fpair spreadAimRange_adjusted = NILRANGE;
}


namespace Fast // Namespace to contain any indexing that uses the integer representation of categories and mults
{
    int categoryCount; // Gets set to fastifyCategory.size() in the main function

    // penalties[coreCategory][partCategory]
    std::vector<std::vector<float>> penalties;

    std::map<std::string, int> fastifyCategory;

    std::vector<bool> includeCategories_fast;
    int forceParts_fast[5] = {NILINT, NILINT, NILINT, NILINT, NILINT}; // Barrel Magazine Grip Stock Core
    std::vector<bool> banParts_fast[5];

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
        FALLOFFFACTOR = 1<<14,
        BURST = 1<<15,
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
        case DROPOFFSTUDS: // Range
        case FALLOFFFACTOR: // Range
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

    // 1, 50, 0.01, 0.25
    float reverseQuadraticDamage[51];
    // 1, 55, 0.005, 0.20
    float quadraticSpread[56];

    float ClampQuadratic(float pelletCount, MultFlags flag)
    {
        if (flag == DAMAGE)
        {
            pelletCount = std::clamp(ceilf(pelletCount), 1.0f, 50.0f);
            return reverseQuadraticDamage[(int)pelletCount];
        }
        else // SPREADAIM and SPREADHIP
        {
            pelletCount = std::clamp(ceilf(pelletCount), 1.0f, 55.0f);
            return quadraticSpread[(int)pelletCount];
        }
    }

    void InitializeClampQuadratic()
    {
        using namespace Input;
        // Precompute because pow and sqrt is scary
        for (int i = 1; i <= 50; i++)
            reverseQuadraticDamage[i] = (float)(0.25f + (sqrtf((50.0f - i) / (50.0f - 1.0f)) * (0.01f - 0.25f)));

        for (int i = 1; i <= 55; i++)
            quadraticSpread[i] = (float)(0.20f + (powf((55.0f - i) / (55.0f - 1.0f), 2.5f) * (0.005f - 0.20f)));

        // for (int i = 1; i <= 50; i++)
        //     std::cout << std::fixed << std::setprecision(3) << reverseQuadraticDamage[i] << ' ';
        // puts("");
        // for (int i = 1; i <= 55; i++)
        //     std::cout << std::fixed << std::setprecision(3) << quadraticSpread[i] << ' ';
        // puts("");

        pelletRange_adjusted = { damageRange.first != NILMIN ? damageRange.first : 1.0f, damageRange.second != NILMAX ? damageRange.second : 55.0f };

        // Adjusted values scale to (mult / pelletModMax^5) < trueMultRange < (mult / pelletModMin) | Yes I know it is a very wide and terrible range but i can't think of a smaller valid range
        damageRange_adjusted = damageRange;
        if (damageRange_adjusted.first != NILMIN) damageRange_adjusted.first /= powf(1 + ClampQuadratic(pelletRange_adjusted.second, DAMAGE), 5);
        if (damageRange_adjusted.second != NILMAX) damageRange_adjusted.second /= 1 + ClampQuadratic(pelletRange_adjusted.first, DAMAGE);

        damage2Range_adjusted = damage2Range;
        if (damage2Range_adjusted.first != NILMIN) damage2Range_adjusted.first /= powf(1 + ClampQuadratic(pelletRange_adjusted.second, DAMAGE), 5);
        if (damage2Range_adjusted.second != NILMAX) damage2Range_adjusted.second /= 1 + ClampQuadratic(pelletRange_adjusted.first, DAMAGE);

        spreadHipRange_adjusted = spreadHipRange;
        if (spreadHipRange_adjusted.first != NILMIN) spreadHipRange_adjusted.first /= powf(1 + ClampQuadratic(pelletRange_adjusted.second, SPREADHIP), 5);
        if (spreadHipRange_adjusted.second != NILMAX) spreadHipRange_adjusted.second /= 1 + ClampQuadratic(pelletRange_adjusted.first, SPREADHIP);

        spreadAimRange_adjusted = spreadAimRange;
        if (spreadAimRange_adjusted.first != NILMIN) spreadAimRange_adjusted.first /= powf(1 + ClampQuadratic(pelletRange_adjusted.second, SPREADAIM), 5);
        if (spreadAimRange_adjusted.second != NILMAX) spreadAimRange_adjusted.second /= 1 + ClampQuadratic(pelletRange_adjusted.first, SPREADAIM);
    }

    std::map<std::string, int> fastifyName = {};

    void PartExists(std::string partName)
    {
        if (!fastifyName.contains(partName))
            throw std::invalid_argument("Part " + partName + " doesn't exist");
    }

    void InitializeIncludeCategories()
    {
        includeCategories_fast.resize(categoryCount, false);
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

        // init banParts_fast to be a bool vector of size fastifyName.size()
        for (int i = 0; i < 5; i++)
            banParts_fast[i].resize(fastifyName.size(), false);

        for (auto barrel: banBarrel)
        {
            PartExists(barrel);
            banParts_fast[0][fastifyName[barrel]] = true;
        }
        for (auto magazine: banMagazine)
        {
            PartExists(magazine);
            banParts_fast[1][fastifyName[magazine]] = true;
        }
        for (auto grip: banGrip)
        {
            PartExists(grip);
            banParts_fast[2][fastifyName[grip]] = true;
        }
        for (auto stock: banStock)
        {
            PartExists(stock);
            banParts_fast[3][fastifyName[stock]] = true;
        }
        for (auto core: banCore)
        {
            PartExists(core);
            banParts_fast[4][fastifyName[core]] = true;
        }

    }

    bool RangeFilter(float value, fpair range)
    {
        if (range.first != NILMIN && range.second != NILMAX) return range.first <= value && value <= range.second;
        else if (range.first != NILMIN) return range.first <= value;
        else if (range.second != NILMAX) return value <= range.second;
        else return true;
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
    float rangeFalloff = 0; // Part of the range property but specifically calculated and stored for falloff calculation (Its calculated differently)

    // Init a default value part
    Part() {}

    // Init an identity value part (Can be used to directly apply the * operator onto a gun).
    static Part Identity() {
        // Health and Movement Speed are additive so the identity value is 0
        Part part;
        part.damage = 1;
        part.fireRate = 1;
        part.spread = 1;
        part.recoil = 1;
        part.reloadSpeed = 1;
        part.pellets = 1;
        part.detectionRadius = 1;
        part.range = 1;
        part.rangeFalloff = 1;
        return part;
    }

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
            case FALLOFFFACTOR:
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

    float damage = 0;
    float falloffFactor = 0;
    fpair dropoffStuds = fpair(0, 0);
    float fireRate = 0;
    float hipfireSpread = 0;
    float adsSpread = 0;
    float timeToAim = 0;
    float movementSpeedModifier = 0;
    float health = 0;
    float pellets = 1;
    float burst = 1;
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

        // Second damage stat will now be calculated using the falloff factor since range stat effects that
        if (!jsonObject.contains("Damage"))
            {}
        else if (jsonObject["Damage"].is_array())
            damage = jsonObject["Damage"][0];
        else
            damage = jsonObject["Damage"];

        if (jsonObject.contains("Dropoff_Studs"))
        {
            dropoffStuds.first = jsonObject["Dropoff_Studs"][0];
            dropoffStuds.second = jsonObject["Dropoff_Studs"][1];
        }

        if (jsonObject.contains("Falloff_Factor")) // Falloff_Factor is not a publicly available property so there only exists data from #update-log channel on discord
            falloffFactor = jsonObject["Falloff_Factor"];
        else if (jsonObject.contains("Damage") && jsonObject["Damage"].is_array())
            falloffFactor = ((float)jsonObject["Damage"][1] - (float)jsonObject["Damage"][0]) / (float)jsonObject["Damage"][0];

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
        if (jsonObject.contains("Burst"))
            burst = jsonObject["Burst"];
        if (jsonObject.contains("Detection_Radius"))
            detectionRadius = jsonObject["Detection_Radius"];
        if (jsonObject.contains("Health"))
            health = jsonObject["Health"];

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

    float damage;
    float damage2() const { return (damage * falloffFactor) + damage; }
    fpair dropoffStuds;
    float falloffFactor;
    float pellets;
    float burst;
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
    float TTK() const { return (ceilf(100 / damage) - 1) / fireRate * 60; }

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

    // Core and magazine copy functions are seperated for prune algorithm
    void CopyCoreValues(uint32_t flags)
    {
        using namespace Fast;
        if (flags & DAMAGE) damage = core->damage;
        if (flags & PELLETS) pellets = core->pellets;
        if (flags & BURST) burst = core->burst;
        if (flags & DROPOFFSTUDS) dropoffStuds = core->dropoffStuds;
        if (flags & FALLOFFFACTOR) falloffFactor = core->falloffFactor;
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

    // Used by Prune algorithm //
    float GetPartialFalloff(Fast::MultFlags propertyFlag, Part *part)
    {
        return 1 - (CalculatePenalty(propertyFlag, part) / 100);
    }

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
        // This doesn't include the pellet mod because prune requires all properties to be independent of each other.
        // Pellet mod gets applied on the last level of the prune in CalculateGunStats and the range is scaled appropriately.

        using namespace Fast;
        if (flags & PELLETS)
            pellets *= GetPartialMult(PELLETS, part);
        if (flags & DAMAGE)
        {
            float mult = GetPartialMult(DAMAGE, part);
            damage *= mult;
        }
        if (flags & FIRERATE) fireRate *= GetPartialMult(FIRERATE, part);
        if (flags & MOVEMENTSPEEDMODIFIER) movementSpeedModifier += GetPartialAdd(MOVEMENTSPEEDMODIFIER, part);
        if (flags & HEALTH) health += GetPartialAdd(HEALTH, part);
        if (flags & RELOAD) reloadTime *= GetPartialMult(RELOAD, part);
        if (flags & DETECTIONRADIUS) detectionRadius *= GetPartialMult(DETECTIONRADIUS, part);
        if (flags & SPREADAIM) adsSpread *= GetPartialMult(SPREADAIM, part);
        if (flags & SPREADHIP) hipfireSpread = GetPartialMult(SPREADHIP, part);
        if (flags & DROPOFFSTUDS)
        {
            dropoffStuds.first *= GetPartialMult(DROPOFFSTUDS, part);
            dropoffStuds.second *= GetPartialMult(DROPOFFSTUDS, part);
        }
        if (flags & FALLOFFFACTOR) falloffFactor *= GetPartialFalloff(FALLOFFFACTOR, part);
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
    float GetTotalFalloff(Fast::MultFlags propertyFlag)
    {
        float mult = 1;
        if (barrel != nullptr)
            mult *= 1 - (CalculatePenalty(propertyFlag, barrel) / 100);
        if (magazine != nullptr)
            mult *= 1 - (CalculatePenalty(propertyFlag, magazine) / 100);
        if (grip != nullptr)
            mult *= 1 - (CalculatePenalty(propertyFlag, grip) / 100);
        if (stock != nullptr)
            mult *= 1 - (CalculatePenalty(propertyFlag, stock) / 100);
        return mult;
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

    void CalculateGunStats(uint32_t flags)
    {
        float previousPelletCount[4] = {-1000, -1000, -1000, -1000}; // Magazine Barrel Grip Stock
        using namespace Fast;
        if (flags & PELLETS)
        {
            // Apply based on order of Magazine, Barrel, Stock, Sight, Grip
            // Magazine Before Barrel and Stock + Sight Before Grip. No idea whether stock or sight gets applied first.
            // (There are no sights or stocks that affect pellets so it is impossible to discern without asking a dev)

            pellets *= GetPartialMult(PELLETS, magazine);
            pellets = ceilf(pellets);
            previousPelletCount[0] = pellets;
            pellets *= GetPartialMult(PELLETS, barrel);
            pellets = ceilf(pellets);
            previousPelletCount[1] = pellets;
            pellets *= GetPartialMult(PELLETS, stock);
            pellets = ceilf(pellets);
            previousPelletCount[2] = pellets;
            pellets *= GetPartialMult(PELLETS, grip);
            pellets = ceilf(pellets);
            previousPelletCount[3] = pellets;
        }
        if (flags & DAMAGE)
        {
            float mult = 1;

            if (core->name_fast != magazine->name_fast)
                mult *= 1 + (CalculatePenalty(DAMAGE, magazine) / 100) + ClampQuadratic(previousPelletCount[0], DAMAGE);
            if (core->name_fast != barrel->name_fast)
                mult *= 1 + (CalculatePenalty(DAMAGE, barrel) / 100) + ClampQuadratic(previousPelletCount[1], DAMAGE);
            if (core->name_fast != stock->name_fast)
                mult *= 1 + (CalculatePenalty(DAMAGE, stock) / 100) + ClampQuadratic(previousPelletCount[2], DAMAGE);
            mult *= 1 + ClampQuadratic(previousPelletCount[2], DAMAGE); // Extra mult for scope
            if (core->name_fast != grip->name_fast)
                mult *= 1 + (CalculatePenalty(DAMAGE, grip) / 100) + ClampQuadratic(previousPelletCount[3], DAMAGE);

            damage *= mult;
        }
        if (flags & FIRERATE) fireRate *= GetTotalMult(FIRERATE);
        if (flags & MOVEMENTSPEEDMODIFIER) movementSpeedModifier += GetTotalAdd(MOVEMENTSPEEDMODIFIER);
        if (flags & HEALTH) health += GetTotalAdd(HEALTH);
        if (flags & RELOAD) reloadTime *= GetTotalMult(RELOAD);
        if (flags & DETECTIONRADIUS) detectionRadius *= GetTotalMult(DETECTIONRADIUS);
        if ((flags & SPREADAIM) || (flags & SPREADHIP))
        {
            // Mult is identical for both ADS and hipfire
            float mult = 1;

            if (core->name_fast != magazine->name_fast)
                mult *= 1 + (CalculatePenalty(SPREADHIP, magazine) / 100) + ClampQuadratic(previousPelletCount[0], SPREADHIP);
            if (core->name_fast != barrel->name_fast)
                mult *= 1 + (CalculatePenalty(SPREADHIP, barrel) / 100) + ClampQuadratic(previousPelletCount[1], SPREADHIP);
            if (core->name_fast != stock->name_fast)
                mult *= 1 + (CalculatePenalty(SPREADHIP, stock) / 100) + ClampQuadratic(previousPelletCount[2], SPREADHIP);
            mult *= 1 + ClampQuadratic(previousPelletCount[2], SPREADHIP); // Extra mult for scope
            if (core->name_fast != grip->name_fast)
                mult *= 1 + (CalculatePenalty(SPREADHIP, grip) / 100) + ClampQuadratic(previousPelletCount[3], SPREADHIP);

            adsSpread *= mult;
            hipfireSpread *= mult;
        }
        if (flags & DROPOFFSTUDS)
        {
            dropoffStuds.first *= GetTotalMult(DROPOFFSTUDS);
            dropoffStuds.second *= GetTotalMult(DROPOFFSTUDS);
        }
        if (flags & FALLOFFFACTOR) falloffFactor *= GetTotalFalloff(FALLOFFFACTOR);
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
};


std::ostream &operator<<(std::ostream &os, const fpair &fp)
{
    os << fp.first << " - " << fp.second;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Gun &gun)
{
    using Input::detailed;
    std::string barrelName = gun.barrel ? gun.barrel->name : "None";
    std::string magazineName = gun.magazine ? gun.magazine->name : "None";
    std::string gripName = gun.grip ? gun.grip->name : "None";
    std::string stockName = gun.stock ? gun.stock->name : "None";
    std::string coreName = gun.core ? gun.core->name : "None";

    os << "[ Barrel: " << barrelName << ", Magazine: " << magazineName << ", Grip: " << gripName << ", Stock: " << stockName << ", Core: " << coreName << " ]\n";
    os << "damage: " << gun.damage << " - " << gun.damage2() << "\n";
    os << "dropoffStuds: " << gun.dropoffStuds << "\n";
    if (Input::detailed) os << "falloffFactor: " << gun.falloffFactor << "\n";
    if (gun.pellets != 1 || Input::detailed) os << "pellets: " << gun.pellets << "\n";
    if (gun.burst != 1 || Input::detailed) os << "burst: " << gun.burst << "\n";
    if (Input::timeToAimRange != fpair(NILRANGE) || Input::detailed) os << "timeToAim: " << gun.timeToAim << "\n";
    os << "fireRate: " << gun.fireRate << "\n";
    os << "adsSpread: " << gun.adsSpread << "\n";
    os << "hipfireSpread: " << gun.hipfireSpread << "\n";
    os << "reloadTime: " << gun.reloadTime << "\n";
    os << "magazineSize: " << gun.magazineSize << "\n";
    if (gun.health != 0 || Input::detailed) os << "health: " << gun.health << "\n";
    os << "recoilHipVertical: " << gun.recoilHipVertical << "\n";
    os << "recoilAimVertical: " << gun.recoilAimVertical << "\n";
    if (gun.movementSpeedModifier != 0 || Input::detailed) os << "movementSpeed: " << gun.movementSpeedModifier << "\n";
    if (Input::detectionRadiusRange != fpair(NILRANGE) || Input::detailed) os << "detectionRadius: " << gun.detectionRadius << "\n";
    if (gun.TTK() != 0 || Input::detailed) os << "TTK: " << gun.TTK() << " Seconds\n";
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
       << "Detection Radius: " << part.detectionRadius << "\n"
       << "Falloff Factor: " << part.rangeFalloff << "\n";
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
    part1.rangeFalloff *= part2.rangeFalloff;
    return part1;
}


// This is now a uint64_t so I don't need to typecast it the moment I try to multiply these things together.
uint64_t barrelCount;
uint64_t magazineCount;
uint64_t gripCount;
uint64_t stockCount;
uint64_t coreCount;
uint64_t totalCombinations;

std::vector<Barrel> barrelList;
std::vector<Magazine> magazineList;
std::vector<Grip> gripList;
std::vector<Stock> stockList;
std::vector<Core> coreList;


namespace PQ // Also known as the stop using so many god damn macros bro holy shit namespace
{
    /*
    IMPORTANT: SORTING NEEDS TO BE THE INVERSE OF WHAT IS NORMALLY CONSIDERED GOOD
    THE CODE WILL BE POPPING THE !WORST! GUNS OUT FIRST ONLY KEEPING TOP 10
    SORTING NEEDS TO BE LIKE THIS :
        return gun1 < gun2 (Less is better)
        return gun1 > gun2 (More is better)
    */

    // The most devious lookin macro
    #define _PQSORT(name, expression) struct Sort##name { bool operator()(const Gun& gun1, const Gun& gun2) const {return expression;} };
    _PQSORT(TTK, gun1.TTK() < gun2.TTK())
    _PQSORT(FireRate, gun1.fireRate > gun2.fireRate)
    _PQSORT(ADSSpread, gun1.adsSpread < gun2.adsSpread)
    _PQSORT(HipfireSpread, gun1.hipfireSpread < gun2.hipfireSpread)
    _PQSORT(Recoil, gun1.recoilAimVertical < gun2.recoilAimVertical)
    _PQSORT(Health, gun1.health > gun2.health)
    _PQSORT(Speed, gun1.movementSpeedModifier > gun2.movementSpeedModifier)
    _PQSORT(Magazine, gun1.magazineSize > gun2.magazineSize)
    _PQSORT(Reload, gun1.reloadTime < gun2.reloadTime)

    #define _TYPEDEFPQ(sort, name) typedef std::priority_queue<Gun, std::vector<Gun>, sort> name;
    _TYPEDEFPQ(SortTTK, TTK_pq)
    _TYPEDEFPQ(SortFireRate, FireRate_pq)
    _TYPEDEFPQ(SortADSSpread, ADSSpread_pq)
    _TYPEDEFPQ(SortHipfireSpread, HipfireSpread_pq)
    _TYPEDEFPQ(SortRecoil, Recoil_pq)
    _TYPEDEFPQ(SortHealth, Health_pq)
    _TYPEDEFPQ(SortSpeed, Speed_pq)
    _TYPEDEFPQ(SortMagazine, Magazine_pq)
    _TYPEDEFPQ(SortReload, Reload_pq)

    typedef std::variant<
        TTK_pq,
        FireRate_pq,
        ADSSpread_pq,
        HipfireSpread_pq,
        Recoil_pq,
        Health_pq,
        Speed_pq,
        Magazine_pq,
        Reload_pq
    > Variant_pq;

    Variant_pq topGuns;

    enum SortingType {
        SORTBYTTK,
        SORTBYFIRERATE,
        SORTBYADSSPREAD,
        SORTBYHIPFIRESPREAD,
        SORTBYRECOIL,
        SORTBYHEALTH,
        SORTBYSPEED,
        SORTBYMAGAZINE,
        SORTBYRELOAD,
    };

    #define _CALLMACROPQANDTYPE(macro) \
        macro(SORTBYTTK, TTK_pq) \
        macro(SORTBYFIRERATE, FireRate_pq) \
        macro(SORTBYADSSPREAD, ADSSpread_pq) \
        macro(SORTBYHIPFIRESPREAD, HipfireSpread_pq) \
        macro(SORTBYRECOIL, Recoil_pq) \
        macro(SORTBYHEALTH, Health_pq) \
        macro(SORTBYSPEED, Speed_pq) \
        macro(SORTBYMAGAZINE, Magazine_pq) \
        macro(SORTBYRELOAD, Reload_pq)


    SortingType currentSortingType = SORTBYTTK;

    void SetCurrentSortingType(std::string type)
    {
        if (type == "TTK") currentSortingType = SORTBYTTK;
        else if (type == "FIRERATE") currentSortingType = SORTBYFIRERATE;
        else if (type == "ADSSPREAD") currentSortingType = SORTBYADSSPREAD;
        else if (type == "HIPFIRESPREAD") currentSortingType = SORTBYHIPFIRESPREAD;
        else if (type == "RECOIL") currentSortingType = SORTBYRECOIL;
        else if (type == "HEALTH") currentSortingType = SORTBYHEALTH;
        else if (type == "SPEED") currentSortingType = SORTBYSPEED;
        else if (type == "MAGAZINE") currentSortingType = SORTBYMAGAZINE;
        else if (type == "RELOAD") currentSortingType = SORTBYRELOAD;
        else throw std::runtime_error("Invalid sorting type");
    }

    bool HasMember(Variant_pq& pq)
    {
        #define __MACROHASMEMBER(sort, pqtype) case sort: return std::get<pqtype>(pq).size() > 0;
        switch (PQ::currentSortingType)
        {
            _CALLMACROPQANDTYPE(__MACROHASMEMBER)
            default:
                throw std::runtime_error("Invalid sorting type");
        }
    }


    Variant_pq Create()
    {
        #define __MACROCREATE(sort, pqtype) case sort: return pqtype();
        switch(currentSortingType)
        {
            _CALLMACROPQANDTYPE(__MACROCREATE)
            default:
                throw std::runtime_error("Invalid sorting type");
        }
    }

    Gun Pop(Variant_pq& pq)
    {
        #define __MACROPOP(sort, pqtype) case sort: {pqtype& _pq = std::get<pqtype>(pq); Gun gun = _pq.top(); _pq.pop(); return gun;}
        switch(currentSortingType)
        {
            _CALLMACROPQANDTYPE(__MACROPOP)
            default:
                throw std::runtime_error("Invalid sorting type");
        }
    }


    void Push(Variant_pq& pq, const Gun& gun)
    {
        #define __MACROPUSH(sort, pqtype) case sort: {pqtype& _pq = std::get<pqtype>(pq); _pq.push(gun); if (_pq.size() > Input::howManyTopGunsToDisplay) _pq.pop(); break;}
        switch(currentSortingType)
        {
            _CALLMACROPQANDTYPE(__MACROPUSH)
            default:
                throw std::runtime_error("Invalid sorting type");
        }
    }

    int Size(Variant_pq& pq)
    {
        #define __MACROSIZE(sort, pqtype) case sort: return std::get<pqtype>(pq).size();
        switch(currentSortingType)
        {
            _CALLMACROPQANDTYPE(__MACROSIZE)
            default:
                throw std::runtime_error("Invalid sorting type");
        }
    }
}

// Maybe make this a class or a struct? prob no tho
PQ::Variant_pq threadPQ[16];
std::thread threads[16];
uint64_t validGunInThread[16];

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
            b = barrelList.data() + barrelIndex;
            m = magazineList.data() + magazineIndex;
            g = gripList.data() + gripIndex;
            s = stockList.data() + stockIndex;
            c = coreList.data() + coreIndex;

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
            if (Input::damage2Range != nilrange) currentflags |= DAMAGE | PELLETS | FALLOFFFACTOR;
            if (Input::magazineRange != nilrange) currentflags |= MAGAZINESIZE;
            if (Input::movementSpeedRange != nilrange) currentflags |= MOVEMENTSPEEDMODIFIER;
            if (Input::spreadHipRange != nilrange) currentflags |= SPREADHIP | PELLETS;
            if (Input::spreadAimRange != nilrange) currentflags |= SPREADAIM | PELLETS;
            if (Input::recoilHipRange != nilrange) currentflags |= RECOILHIP;
            if (Input::recoilAimRange != nilrange) currentflags |= RECOILAIM;
            if (Input::fireRateRange != nilrange) currentflags |= FIRERATE;
            if (Input::healthRange != nilrange) currentflags |= HEALTH;
            if (Input::reloadRange != nilrange) currentflags |= RELOAD;
            if (Input::detectionRadiusRange != nilrange) currentflags |= DETECTIONRADIUS;
            if (Input::dropoffStudsRange != nilrange) currentflags |= DROPOFFSTUDS;
            if (Input::dropoffStuds2Range != nilrange) currentflags |= DROPOFFSTUDS;
            if (Input::burstRange != nilrange) currentflags |= BURST;
            switch (PQ::currentSortingType)
            {
                case PQ::SORTBYTTK:
                    currentflags |= DAMAGE | PELLETS;
                case PQ::SORTBYFIRERATE:
                    currentflags |= FIRERATE;
                    break;
                case PQ::SORTBYADSSPREAD:
                    currentflags |= SPREADAIM | PELLETS;
                    break;
                case PQ::SORTBYHIPFIRESPREAD:
                    currentflags |= SPREADHIP | PELLETS;
                    break;
                case PQ::SORTBYRECOIL:
                    currentflags |= RECOILAIM;
                    break;
                case PQ::SORTBYHEALTH:
                    currentflags |= HEALTH;
                    break;
                case PQ::SORTBYSPEED:
                    currentflags |= MOVEMENTSPEEDMODIFIER;
                    break;
                case PQ::SORTBYMAGAZINE:
                    currentflags |= MAGAZINESIZE;
                    break;
                case PQ::SORTBYRELOAD:
                    currentflags |= RELOAD;
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

            if (Fast::banParts_fast[0][b->name_fast]) return false;
            if (Fast::banParts_fast[1][m->name_fast]) return false;
            if (Fast::banParts_fast[2][g->name_fast]) return false;
            if (Fast::banParts_fast[3][s->name_fast]) return false;
            if (Fast::banParts_fast[4][c->name_fast]) return false;

            if (!Fast::includeCategories_fast[c->category_fast]) return false;
            if (!RangeFilter(c->burst, Input::burstRange)) return false;
            if (!RangeFilter(c->timeToAim, Input::timeToAimRange)) return false;
            if (!RangeFilter(m->magazineSize, Input::magazineRange)) return false;
            return true;
        }

        bool PostFilter(Gun& gun)
        {
            using Fast::RangeFilter;
            if (!RangeFilter(gun.damage, Input::damageRange)) return false;
            if (!RangeFilter(gun.damage2(), Input::damage2Range)) return false;
            if (!RangeFilter(gun.adsSpread, Input::spreadAimRange)) return false;
            if (!RangeFilter(gun.hipfireSpread, Input::spreadHipRange)) return false;
            if (!RangeFilter(gun.recoilAimVertical.second, Input::recoilAimRange)) return false;
            if (!RangeFilter(gun.recoilHipVertical.second, Input::recoilHipRange)) return false;
            if (!RangeFilter(gun.movementSpeedModifier, Input::movementSpeedRange)) return false;
            if (!RangeFilter(gun.fireRate, Input::fireRateRange)) return false;
            if (!RangeFilter(gun.health, Input::healthRange)) return false;
            if (!RangeFilter(ceilf(gun.pellets), Input::pelletRange)) return false;
            if (!RangeFilter(gun.reloadTime, Input::reloadRange)) return false;
            if (!RangeFilter(gun.detectionRadius, Input::detectionRadiusRange)) return false;
            if (!RangeFilter(gun.dropoffStuds.first, Input::dropoffStudsRange)) return false;
            if (!RangeFilter(gun.dropoffStuds.second, Input::dropoffStuds2Range)) return false;
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
            if (Input::damage2Range != nilrange) currentflags |= DAMAGE | PELLETS | FALLOFFFACTOR;
            if (Input::magazineRange != nilrange) currentflags |= MAGAZINESIZE;
            if (Input::movementSpeedRange != nilrange) currentflags |= MOVEMENTSPEEDMODIFIER;
            if (Input::spreadHipRange != nilrange) currentflags |= SPREADHIP | PELLETS;
            if (Input::spreadAimRange != nilrange) currentflags |= SPREADAIM | PELLETS;
            if (Input::recoilHipRange != nilrange) currentflags |= RECOILHIP;
            if (Input::recoilAimRange != nilrange) currentflags |= RECOILAIM;
            if (Input::fireRateRange != nilrange) currentflags |= FIRERATE;
            if (Input::healthRange != nilrange) currentflags |= HEALTH;
            if (Input::reloadRange != nilrange) currentflags |= RELOAD;
            if (Input::detectionRadiusRange != nilrange) currentflags |= DETECTIONRADIUS;
            if (Input::dropoffStudsRange != nilrange) currentflags |= DROPOFFSTUDS;
            if (Input::dropoffStuds2Range != nilrange) currentflags |= DROPOFFSTUDS;
            if (Input::burstRange != nilrange) currentflags |= BURST;
            switch (PQ::currentSortingType)
            {
                case PQ::SORTBYTTK:
                    currentflags |= DAMAGE | PELLETS;
                case PQ::SORTBYFIRERATE:
                    currentflags |= FIRERATE;
                    break;
                case PQ::SORTBYADSSPREAD:
                    currentflags |= SPREADAIM | PELLETS;
                    break;
                case PQ::SORTBYHIPFIRESPREAD:
                    currentflags |= SPREADHIP | PELLETS;
                    break;
                case PQ::SORTBYRECOIL:
                    currentflags |= RECOILAIM;
                    break;
                case PQ::SORTBYHEALTH:
                    currentflags |= HEALTH;
                    break;
                case PQ::SORTBYSPEED:
                    currentflags |= MOVEMENTSPEEDMODIFIER;
                    break;
                case PQ::SORTBYMAGAZINE:
                    currentflags |= MAGAZINESIZE;
                    break;
                case PQ::SORTBYRELOAD:
                    currentflags |= RELOAD;
                    break;
            }
        }

        bool CoreFilter(Core *core) // These filters require no gun calculation and can be immediately checked on the part itself
        {
            if (Fast::banParts_fast[4][core->name_fast]) return false;
            if (Fast::forceParts_fast[4] != NILINT && Fast::forceParts_fast[4] != core->name_fast) return false;
            if (!Fast::includeCategories_fast[core->category_fast]) return false;
            if (!Fast::RangeFilter(core->timeToAim, Input::timeToAimRange)) return false;
            if (!Fast::RangeFilter(core->burst, Input::burstRange)) return false;
            return true;
        }

        bool MagazineFilter(Magazine *magazine)
        {
            if (Fast::banParts_fast[1][magazine->name_fast]) return false;
            if (Fast::forceParts_fast[1] != NILINT && Fast::forceParts_fast[1] != magazine->name_fast) return false;
            if (!Fast::RangeFilter(magazine->magazineSize, Input::magazineRange)) return false;
            return true;
        }

        bool BarrelFilter(Barrel *barrel)
        {
            if (Fast::banParts_fast[0][barrel->name_fast]) return false;
            if (Fast::forceParts_fast[0] != NILINT && Fast::forceParts_fast[0] != barrel->name_fast) return false;
            return true;
        }

        bool GripFilter(Grip *grip)
        {
            if (Fast::banParts_fast[2][grip->name_fast]) return false;
            if (Fast::forceParts_fast[2] != NILINT && Fast::forceParts_fast[2] != grip->name_fast) return false;
            return true;
        }

        bool StockFilter(Stock *stock)
        {
            if (Fast::banParts_fast[3][stock->name_fast]) return false;
            if (Fast::forceParts_fast[3] != NILINT && Fast::forceParts_fast[3] != stock->name_fast) return false;
            return true;
        }

        bool FilterGunStats(const Gun& gun)
        {
            using Fast::RangeFilter;
            if (!Fast::includeCategories_fast[gun.core->category_fast]) return false;
            if (!RangeFilter(gun.timeToAim, Input::timeToAimRange)) return false;
            if (!RangeFilter(gun.magazineSize, Input::magazineRange)) return false;
            if (!RangeFilter(ceilf(gun.pellets), Input::pelletRange)) return false;
            if (!RangeFilter(gun.damage, Input::damageRange)) return false;
            if (!RangeFilter(gun.damage2(), Input::damage2Range)) return false;
            if (!RangeFilter(gun.adsSpread, Input::spreadAimRange)) return false;
            if (!RangeFilter(gun.hipfireSpread, Input::spreadHipRange)) return false;
            if (!RangeFilter(gun.recoilAimVertical.second, Input::recoilAimRange)) return false;
            if (!RangeFilter(gun.recoilHipVertical.second, Input::recoilHipRange)) return false;
            if (!RangeFilter(gun.movementSpeedModifier, Input::movementSpeedRange)) return false;
            if (!RangeFilter(gun.fireRate, Input::fireRateRange)) return false;
            if (!RangeFilter(gun.health, Input::healthRange)) return false;
            if (!RangeFilter(gun.reloadTime, Input::reloadRange)) return false;
            if (!RangeFilter(gun.detectionRadius, Input::detectionRadiusRange)) return false;
            if (!RangeFilter(gun.dropoffStuds.first, Input::dropoffStudsRange)) return false;
            if (!RangeFilter(gun.dropoffStuds.second, Input::dropoffStuds2Range)) return false;
            return true;
        }
    }

    namespace HighLow
    {
        /*
        This namespace revolves around the key data structure used in the Prune algorithm.
        It revolves around bestPossibleCombo which is a three-dimensional array containing
        Part objects that contain the best multiplier combination both low and high, for
        every core category and from a 1 part combo to a 4 part combo.
        */

        // bestPossibleCombo[coreCategoryFast][low / high][1 -> 4]
        std::vector<std::vector<std::vector<Part>>> bestPossibleCombo; // Chatgpt suggested me this omegalul

        void InitializeBestPossibleCombo() // resize based on category count
        {
            bestPossibleCombo.resize(Fast::categoryCount);
            for (int i = 0; i < Fast::categoryCount; i++)
            {
                bestPossibleCombo[i].resize(2);
                for (int j = 0; j < 2; j++)
                {
                    bestPossibleCombo[i][j].resize(4, Part::Identity());
                }
            }
        }

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
            if (gun.GetPartialFalloff(FALLOFFFACTOR, partToCheck) < lowestMultPart.rangeFalloff) lowestMultPart.rangeFalloff = gun.GetPartialFalloff(FALLOFFFACTOR, partToCheck);

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
            if (gun.GetPartialFalloff(FALLOFFFACTOR, partToCheck) > highestMultPart.rangeFalloff) highestMultPart.rangeFalloff = gun.GetPartialFalloff(FALLOFFFACTOR, partToCheck);
        }

        // Polymorphism is required due to the fact that sizeof(Magazine) is more than sizeof(Part) causing pointer arithmetic to be incorrect when iterating through the array.
        template <typename T>
        std::pair<Part, Part> FindHighestAndLowestMultInList(Gun& gun, T* partList, int size)
        {
            Part lowestMultPart = Part::Identity(), highestMultPart = Part::Identity();
            for (int i = 0; i < size; i++)
                SetHighLowParts(gun, partList + i, lowestMultPart, highestMultPart);
            return std::pair<Part, Part>(lowestMultPart, highestMultPart);
        }

        void InitializeHighestAndLowestMultParts() // It may not look like it but I promise you this has a Time complexity of O(n) I'M TELLING YOU PLEASE BELIEVE ME
        {
            // Creation of dummyGuns to allow us to use (Gun obj).GetPartialMult and (Gun obj).GetPartialAdd for HighLow intialization
            std::vector<Core> coreCategories(Fast::categoryCount);
            std::vector<Gun> dummyGuns(Fast::categoryCount);
            for (int i = 0; i < Fast::categoryCount; i++)
            {
                coreCategories[i].category_fast = i;
                dummyGuns[i] = Gun(&coreCategories[i]);
            }

            for (int g = 0; g < Fast::categoryCount; g++)
            {
                Gun& dummyGun = dummyGuns[g];
                std::pair<Part, Part> magazineMultPair = FindHighestAndLowestMultInList(dummyGun, magazineList.data(), magazineCount);
                std::pair<Part, Part> barrelMultPair = FindHighestAndLowestMultInList(dummyGun, barrelList.data(), barrelCount);
                std::pair<Part, Part> gripMultPair = FindHighestAndLowestMultInList(dummyGun, gripList.data(), gripCount);
                std::pair<Part, Part> stockMultPair = FindHighestAndLowestMultInList(dummyGun, stockList.data(), stockCount);

                std::pair<Part, Part> *partPairs[4] = {&stockMultPair, &gripMultPair, &barrelMultPair, &magazineMultPair};
                for (int i = 0; i < 4; i++)
                {
                    for (int j = i; j < 4; j++)
                    {
                        // No *= because I said so
                        bestPossibleCombo[g][0][j] = bestPossibleCombo[g][0][j] * partPairs[i]->first;
                        bestPossibleCombo[g][1][j] = bestPossibleCombo[g][1][j] * partPairs[i]->second;
                    }
                }
            }
            // for (int i = 0; i < 6; i++) for (int k = 0; k < 4; k++) for (int j = 0; j < 2; j++) std::cout << bestPossibleCombo[i][j][k];
        }
    }


    bool RangeFilterPellet(float valueToCompare, float lowMult, float highMult, const fpair &range) // Requires the use of ceilf
    {
        if (range.first == NILMIN && range.second == NILMAX) return true;
        if (range.first == NILMIN && ceilf(valueToCompare * lowMult) <= range.second) return true;
        if (range.second == NILMAX && ceilf(valueToCompare * highMult) >= range.first) return true;
        return ceilf(valueToCompare * highMult) >= range.first && ceilf(valueToCompare * lowMult) <= range.second;
    }

    bool RangeFilterDamage2(const Gun& gun, const Part& lowPPC, const Part& highPPC, const fpair &range)
    {
        if (range.first == NILMIN && range.second == NILMAX) return true;

        float lowDiff, highDiff;
        float lowDamage = gun.damage * lowPPC.damage;
        float highDamage = gun.damage * highPPC.damage;
        if (gun.falloffFactor > 0)
        {
            lowDiff =  lowDamage * (gun.falloffFactor * lowPPC.rangeFalloff);
            highDiff = highDamage * (gun.falloffFactor * highPPC.rangeFalloff);
        }
        else
        {
            lowDiff = lowDamage * (gun.falloffFactor * highPPC.rangeFalloff);
            highDiff = highDamage * (gun.falloffFactor * lowPPC.rangeFalloff);
        }
        if (range.first == NILMIN) return lowDamage + lowDiff <= range.second;
        if (range.second == NILMAX) return highDamage + highDiff >= range.first;
        return highDamage + highDiff >= range.first && lowDamage + lowDiff <= range.second;
    }

    bool RangeFilterMult(float valueToCompare, float lowMult, float highMult, const fpair &range)
    {
        if (range.first == NILMIN && range.second == NILMAX) return true;
        if (range.first == NILMIN) return valueToCompare * lowMult <= range.second;
        if (range.second == NILMAX) return valueToCompare * highMult >= range.first;
        return valueToCompare * highMult >= range.first && valueToCompare * lowMult <= range.second;
    }

    bool RangeFilterAdd(float valueToCompare, float lowAdd, float highAdd, const fpair &range)
    {
        if (range.first == NILMIN && range.second == NILMAX) return true;
        if (range.first == NILMIN) return valueToCompare + lowAdd <= range.second;
        if (range.second == NILMAX) return valueToCompare + highAdd >= range.first;
        return valueToCompare + highAdd >= range.first && valueToCompare + lowAdd <= range.second;
    }

    bool IsValidCombination(const Gun& gun, const Part& lowPPC, const Part& highPPC) // PPC Stands for PossiblePartCombo // This needs to be changed
    {
        using namespace Fast;
        if (!RangeFilterPellet(gun.pellets, lowPPC.pellets, highPPC.pellets, Input::pelletRange)) return false;
        if (!RangeFilterMult(gun.damage, lowPPC.damage, highPPC.damage, Input::damageRange_adjusted)) return false;
        if (!RangeFilterDamage2(gun, lowPPC, highPPC, Input::damage2Range_adjusted)) return false;
        if (!RangeFilterMult(gun.hipfireSpread, lowPPC.spread, highPPC.spread, Input::spreadHipRange_adjusted)) return false;
        if (!RangeFilterMult(gun.adsSpread, lowPPC.spread, highPPC.spread, Input::spreadAimRange_adjusted)) return false;
        if (!RangeFilterMult(gun.fireRate, lowPPC.fireRate, highPPC.fireRate, Input::fireRateRange)) return false;
        if (!RangeFilterMult(gun.recoilHipVertical.second, lowPPC.recoil, highPPC.recoil, Input::recoilHipRange)) return false;
        if (!RangeFilterMult(gun.recoilAimVertical.second, lowPPC.recoil, highPPC.recoil, Input::recoilAimRange)) return false;
        if (!RangeFilterMult(gun.reloadTime, lowPPC.reloadSpeed, highPPC.reloadSpeed, Input::reloadRange)) return false;
        if (!RangeFilterMult(gun.detectionRadius, lowPPC.detectionRadius, highPPC.detectionRadius, Input::detectionRadiusRange)) return false;
        if (!RangeFilterMult(gun.dropoffStuds.first, lowPPC.range, highPPC.range, Input::dropoffStudsRange)) return false;
        if (!RangeFilterMult(gun.dropoffStuds.second, lowPPC.range, highPPC.range, Input::dropoffStuds2Range)) return false;
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
            if (!Filter::CoreFilter(coreList.data() + c)) continue;

            Gun currentGun = Gun(coreList.data() + c);
            currentGun.CopyCoreValues(Filter::currentflags);
            bool validCombo = IsValidCombination(currentGun, HighLow::bestPossibleCombo[currentGun.core->category_fast][0][3], HighLow::bestPossibleCombo[currentGun.core->category_fast][1][3]);
            if (validCombo)
                CustomPushback(validGuns1, validGunCount1, currentGun);
        }
        printf("%d: Total valid cores: %lu / %lu\n", threadId, validGunCount1, coreCount);

        // validGuns1 -> validGuns2 (Parsing core + magazine)
        for (int m = 0; m < magazineCount; m++)
        {
            if (!Filter::MagazineFilter(magazineList.data() + m)) continue;
            for (int g = 0; g < validGunCount1; g++)
            {
                Gun currentGun = validGuns1[g]; // Copies over from the old vector
                currentGun.magazine = magazineList.data() + m;
                currentGun.CopyMagazineValues(Filter::currentflags);
                currentGun.CalculatePartialGunStats(Filter::currentflags, currentGun.magazine);
                bool validCombo = IsValidCombination(currentGun, HighLow::bestPossibleCombo[currentGun.core->category_fast][0][2], HighLow::bestPossibleCombo[currentGun.core->category_fast][1][2]);
                if (validCombo)
                    CustomPushback(validGuns2, validGunCount2, currentGun);
            }
        }
        printf("%d: Total valid core + mag: %lu / %lu\n", threadId, validGunCount2, coreCount * magazineCount);
        validGunCount1 = 0;

        // validGuns2 -> validGuns1 (Parsing core + magazine + barrel)
        for (int b = 0; b < barrelCount; b++)
        {
            if (!Filter::BarrelFilter(barrelList.data() + b)) continue;
            for (int g = 0; g < validGunCount2; g++)
            {
                Gun currentGun = validGuns2[g]; // Copies over from the old vector
                currentGun.barrel = barrelList.data() + b;
                currentGun.CalculatePartialGunStats(Filter::currentflags, currentGun.barrel);
                bool validCombo = IsValidCombination(currentGun, HighLow::bestPossibleCombo[currentGun.core->category_fast][0][1], HighLow::bestPossibleCombo[currentGun.core->category_fast][1][1]);
                if (validCombo)
                    CustomPushback(validGuns1, validGunCount1, currentGun);
            }
        }
        printf("%d: Total valid core + mag + barrel: %lu / %lu\n", threadId, validGunCount1, coreCount * magazineCount * barrelCount);
        validGunCount2 = 0;

        // validGuns1 -> validGuns2 (Parsing core + magazine + barrel + grip)
        for (int gr = 0; gr < gripCount; gr++)
        {
            if (!Filter::GripFilter(gripList.data() + gr)) continue;
            for (int g = 0; g < validGunCount1; g++)
            {
                Gun currentGun = validGuns1[g]; // Copies over from the old vector
                currentGun.grip = gripList.data() + gr;
                currentGun.CalculatePartialGunStats(Filter::currentflags, currentGun.grip);
                bool validCombo = IsValidCombination(currentGun, HighLow::bestPossibleCombo[currentGun.core->category_fast][0][0], HighLow::bestPossibleCombo[currentGun.core->category_fast][1][0]);
                if (validCombo)
                    CustomPushback(validGuns2, validGunCount2, currentGun);
            }
        }
        printf("%d: Total valid core + mag + barrel + grip: %lu / %lu\n", threadId, validGunCount2, coreCount * magazineCount * barrelCount * gripCount);
        validGunCount1 = 0;

        // validGuns2 * stocks -> Outputs to PQ
        for (int s = 0; s < stockCount; s++)
        {
            if (!Filter::StockFilter(stockList.data() + s)) continue;
            for (int g = 0; g < validGunCount2; g++)
            {
                Gun currentGun = validGuns2[g]; // Copies over from the old vector
                currentGun.stock = stockList.data() + s;

                currentGun.CalculatePartialGunStats(Filter::currentflags, currentGun.stock);
                // Recalculate Damage and Spread to contain the pellet modifier
                if (Filter::currentflags & Fast::PELLETS)
                    currentGun.pellets = currentGun.core->pellets;
                if (Filter::currentflags & Fast::DAMAGE)
                    currentGun.damage = currentGun.core->damage;
                if (Filter::currentflags & Fast::SPREADAIM)
                    currentGun.adsSpread = currentGun.core->adsSpread;
                if (Filter::currentflags & Fast::SPREADHIP)
                    currentGun.hipfireSpread = currentGun.core->hipfireSpread;

                currentGun.CalculateGunStats(Filter::currentflags & (Fast::PELLETS | Fast::DAMAGE | Fast::SPREADAIM | Fast::SPREADHIP));

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
        printf("%d: Total valid core + mag + barrel + grip + stock: %lu / %lu\n", threadId, validGunCount1, coreCount * magazineCount * barrelCount * gripCount * stockCount);
    }
}

int main(int argc, char* argv[])
{
    CLI::App app{"A tool to bruteforce all possible stats inside of weird gun game."};
    argv = app.ensure_utf8(argv);

    app.add_option("-f, --file", Input::fileDir, "Path to the directory containing the json file (Default: Data)");
    app.add_option("-o, --output", Input::outpath, "Path to the output file (Default: Results.txt)");
    app.add_option("-t, --threads", Input::threadsToMake, "Number of threads to use (MAX 16) (Default: AUTODETECT)")->check(CLI::Range(1, 16));
    app.add_option("-s, --sort", Input::sortType, "Sorting type (TTK, FIRERATE, ADSSPREAD, HIPFIRESPREAD, RECOIL, SPEED, HEALTH, MAGAZINE, RELOAD) (Default: TTK)");
    app.add_option("-n, --number", Input::howManyTopGunsToDisplay, "Number of top guns to display (Default: 10)");
    app.add_option("-m, --method", Input::method, "Method to use for calculation (BRUTEFORCE, PRUNE) (Default: PRUNE)");
    app.add_option("-i, --include", Input::includeCategories, "Categories to include in the calculation (AR, Sniper, LMG, SMG, Shotgun, Weird)")->required();
    app.add_flag  ("-d, --detailed", Input::detailed, "Display all stats of the gun including irrelevant ones");

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

    app.add_option("--damage, --damageStart", Input::damageRange, "Damage range to filter (START)");
    app.add_option("--damageMin, --damageStartMin", Input::damageRange.first);
    app.add_option("--damageMax, --damageStartMax", Input::damageRange.second);
    app.add_option("--damageEnd", Input::damage2Range, "Damage range to filter (END)");
    app.add_option("--damageEndMin", Input::damage2Range.first);
    app.add_option("--damageEndMax", Input::damage2Range.second);
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
    app.add_option("--detectionRadius", Input::detectionRadiusRange, "Detection radius range to filter");
    app.add_option("--detectionRadiusMin", Input::detectionRadiusRange.first);
    app.add_option("--detectionRadiusMax", Input::detectionRadiusRange.second);
    app.add_option("--range, --rangeStart", Input::dropoffStudsRange, "Dropoff studs range to filter (START)");
    app.add_option("--rangeMin, --rangeStartMin", Input::dropoffStudsRange.first);
    app.add_option("--rangeMax, --rangeStartEnd", Input::dropoffStudsRange.second);
    app.add_option("--rangeEnd", Input::dropoffStuds2Range, "Dropoff studs range to filter (END)");
    app.add_option("--rangeEndMin", Input::dropoffStuds2Range.first);
    app.add_option("--rangeEndMax", Input::dropoffStuds2Range.second);
    app.add_option("--burst", Input::burstRange, "Burst range to filter");
    app.add_option("--burstMin", Input::burstRange.first);
    app.add_option("--burstMax", Input::burstRange.second);

    CLI11_PARSE(app, argc, argv);

    // The operator overloads go c++azy (Python actually has this with Pathlib but we don't talk about that)
    std::filesystem::path fullDataPath = Input::fileDir / "FullData.json";
    std::filesystem::path penaltiesPath = Input::fileDir / "Penalties.json";
    std::filesystem::path categoriesPath = Input::fileDir / "Categories.json";

    if (!std::filesystem::exists(fullDataPath) || !std::filesystem::exists(penaltiesPath) || !std::filesystem::exists(categoriesPath))
    {
        std::cerr << "Files " << fullDataPath << ", " << penaltiesPath << ", and " << categoriesPath << " are required but don't exist (Did you install this in the correct folder?)\n";
        throw std::invalid_argument("Required files not found");
    }

    std::cout << "Loading from json file " << fullDataPath << ", " << penaltiesPath << ", and " << categoriesPath << "\n";

    std::ifstream Categories(categoriesPath);
    json categories = json::parse(Categories);

    for (auto &[key, value] : categories.items())
        Fast::fastifyCategory[key] = value;

    Fast::categoryCount = Fast::fastifyCategory.size();

    std::ifstream FullData(fullDataPath);
    json data = json::parse(FullData);

    barrelCount = data["Barrels"].size();
    barrelList.reserve(barrelCount);

    for (json &element : data["Barrels"])
        barrelList.push_back(element);

    magazineCount = data["Magazines"].size();
    magazineList.reserve(magazineCount);

    for (json &element : data["Magazines"])
        magazineList.push_back(element);

    gripCount = data["Grips"].size();
    gripList.reserve(gripCount);

    for (json &element : data["Grips"])
        gripList.push_back(element);

    stockCount = data["Stocks"].size();
    stockList.reserve(stockCount);

    for (json &element : data["Stocks"])
        stockList.push_back(element);

    coreCount = data["Cores"].size();
    coreList.reserve(coreCount);

    for (json &element : data["Cores"])
        coreList.push_back(element);

    std::ifstream Penalties(penaltiesPath);
    json penalties = json::parse(Penalties);

    Fast::penalties.resize(Fast::categoryCount);
    for (int i = 0; i < Fast::categoryCount; i++)
        Fast::penalties[i].resize(Fast::categoryCount);

    for (int i = 0; i < Fast::categoryCount; i++)
        for (int j = 0; j < Fast::categoryCount; j++)
            Fast::penalties[i][j] = penalties[i][j];

    puts("Initializing required data");

    PQ::SetCurrentSortingType(Input::sortType);
    Fast::InitializeIncludeCategories();
    Fast::InitializeForceAndBanParts();
    Fast::InitializeClampQuadratic();
    BruteForce::Filter::InitializeMultFlag();
    Prune::Filter::InitializeMultFlag();
    Prune::HighLow::InitializeBestPossibleCombo();
    Prune::HighLow::InitializeHighestAndLowestMultParts();

    totalCombinations = barrelCount * magazineCount * gripCount * stockCount * coreCount;
    printf("Barrels detected: %lu, Magazines detected: %lu, Grips detected: %lu, Stocks detected: %lu, Cores detected: %lu\n", barrelCount, magazineCount, gripCount, stockCount, coreCount);
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
        puts("This function is deprecated and I can't be bothered to test if it works 100%. PRUNE provides the same functionality with faster performance.");
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

    printf("%s completed\n", Input::method.c_str());
    std::cout << "Elapsed time: "
         << minutes << " minute(s), "
         << seconds << " second(s), "
         << milliseconds << " millisecond(s), and "
         << microseconds << " microsecond(s)\n";

    uint64_t totalValidGuns = 0;
    for (int i = 0; i < Input::threadsToMake; i++)
        totalValidGuns += validGunInThread[i];
    printf("Total valid gun combinations based on filters: %lu / %lu\n", totalValidGuns, coreCount * magazineCount * barrelCount * gripCount * stockCount);

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

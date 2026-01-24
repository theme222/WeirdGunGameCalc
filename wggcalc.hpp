// This is the header file for wggcalc. Whole lotta inlines
#include <iostream>
#include <stdexcept>
#include <string>
#include <map>
#include <filesystem>
#include <algorithm>
#include <queue>
#include <sys/types.h>
#include <vector>
#include <cmath>
#include <cstdint>
#include <thread>
#include <atomic>
#include "include/json.hpp"
// #include <variant>
// #include <iomanip>

#ifndef __WGGCALC_HPP__
#define __WGGCALC_HPP__
#define __WGGCALC_VERSION__ "1.7.1"

using fpair = std::pair<float, float>;
using json = nlohmann::json;
using std::size_t;

#define DEFAULTVECTORSIZE 100000lu
#define ALLMULTFLAG 4294967295 // 2^32 - 1
#define NILMIN -69420.5f
#define NILMAX 69420.5f
#define NILRANGE {NILMIN, NILMAX} // Hehehheheh who says programmers can't have fun
#define NILINT -1
typedef uint32_t multiFlags;

const fpair NILRANGE_P = NILRANGE;

namespace Input
{
    inline std::filesystem::path fileDir = "Data";
    inline std::string outpath = "Results.txt";
    inline std::string sortType = "TTK";
    inline std::string method = "DYNAMICPRUNE";
    inline std::string sortPriority = "AUTO"; // HIGHEST, LOWEST, AUTO

    // flags
    inline bool version = false;
    inline bool detailed = false;
    inline bool debug = false;
    inline bool testInstall = false;

    // These handle if the users force a specific part to be included
    inline std::vector<std::string> forceBarrel;
    inline bool forcingBarrel = false;
    inline std::vector<std::string> forceMagazine;
    inline bool forcingMagazine = false;
    inline std::vector<std::string> forceCore;
    inline bool forcingCore = false;
    inline std::vector<std::string> forceStock;
    inline bool forcingStock = false;
    inline std::vector<std::string> forceGrip;
    inline bool forcingGrip = false;

    inline std::vector<std::string> banBarrel;
    inline std::vector<std::string> banMagazine;
    inline std::vector<std::string> banCore;
    inline std::vector<std::string> banStock;
    inline std::vector<std::string> banGrip;

    inline std::vector<std::string> includeCategories;

    inline uint64_t threadsToMake = std::clamp(std::thread::hardware_concurrency(), 1u, 64u);
    inline uint64_t howManyTopGunsToDisplay = 10;
    inline int playerMaxHealth = 100;

    const int TOTALFILTERCOUNT = 19;
    inline fpair damageRange = NILRANGE;
    inline fpair damageEndRange = NILRANGE;
    inline fpair magazineRange = NILRANGE;
    inline fpair spreadHipRange = NILRANGE;
    inline fpair spreadAimRange = NILRANGE;
    inline fpair recoilHipRange = NILRANGE;
    inline fpair recoilAimRange = NILRANGE;
    inline fpair movementSpeedRange = NILRANGE;
    inline fpair fireRateRange = NILRANGE;
    inline fpair healthRange = NILRANGE;
    inline fpair pelletRange = NILRANGE;
    inline fpair timeToAimRange = NILRANGE;
    inline fpair reloadRange = NILRANGE;
    inline fpair detectionRadiusRange = NILRANGE;
    inline fpair dropoffStudsRange = NILRANGE;
    inline fpair dropoffStudsEndRange = NILRANGE;
    inline fpair burstRange = NILRANGE;
    inline fpair TTKRange = NILRANGE;
    inline fpair DPSRange = NILRANGE;

    inline void FormatInputs()
    {
        forcingBarrel = !forceBarrel.empty();
        forcingMagazine = !forceMagazine.empty();
        forcingCore = !forceCore.empty();
        forcingStock = !forceStock.empty();
        forcingGrip = !forceGrip.empty();
        detailed = detailed || debug;
        if (TTKRange.first != NILMIN) TTKRange.first /= 60.f;
        if (TTKRange.second != NILMAX) TTKRange.second /= 60.f;
        if (DPSRange.first != NILMIN) DPSRange.first *= 60.f;
        if (DPSRange.second != NILMAX) DPSRange.second *= 60.f;
    }

}


namespace Fast // Namespace to contain any indexing that uses the integer representation of categories and mults (I'm also just realizing that this is just giving an id to parts)
{
    inline int categoryCount; // Gets set to fastifyCategory.size() in the main function
    inline int primaryCategoryCount;
    inline int secondaryCategoryCount;

    inline bool includedPrimary = false;
    inline bool includedSecondary = false;

    // penalties[coreCategory][partCategory]
    inline std::vector<std::vector<float>> penalties;

    inline std::map<std::string, int> fastifyCategory;

    inline std::vector<bool> includeCategories_fast;
    // 0: core, 1: magazine, 2: barrel, 3: stock, 4: grip
    inline std::vector<bool> forceParts_fast[5];
    inline std::vector<bool> banParts_fast[5];

    const int TOTALMULTFLAGS = 20;
    const int TOTALPROPERTYFLAGS = 20;
    const int TOTALPARTFLAGS = 14;
    enum MultFlags {
        DAMAGE = 1<<0, // Part property
        DAMAGEEND = 1<<1, // Gun property exclusive
        DROPOFFSTUDS = 1<<2, // Part property (range)
        DROPOFFSTUDSEND = 1<<3, // Part property (range)
        RELOAD = 1<<4, // Part property
        MAGAZINESIZE = 1<<5, // Can't be modified // Magazine property
        FIRERATE = 1<<6, // Part property
        TIMETOAIM = 1<<7, // Can't be modified // Core property
        MOVEMENTSPEEDMODIFIER = 1<<8, // Part property
        SPREADAIM = 1<<9, // Part property (spread)
        SPREADHIP = 1<<10, // Part property (spread)
        RECOILAIM = 1<<11, // Part property (recoil)
        RECOILHIP = 1<<12, // Part property (recoil)
        PELLETS = 1<<13, // Part property
        HEALTH = 1<<14, // Part property
        DETECTIONRADIUS = 1<<15, // Part property
        FALLOFFFACTOR = 1<<16, // Can't be filtered // Part property (range)
        BURST = 1<<17, // Can't be modified // Core property
        TTK = 1<<18, // Gun property exclusive
        DPS = 1<<19, // Gun property exclusive
    };

    constexpr bool MoreIsBetter(MultFlags propertyFlag)
    {
        static_assert(TOTALMULTFLAGS == 20, "MoreIsBetter must be updated to contain all flags");
        switch (propertyFlag)
        {
            case DAMAGE:
            case DAMAGEEND:
            case DPS:
            case MOVEMENTSPEEDMODIFIER:
            case HEALTH:
            case FIRERATE:
            case PELLETS:
            case DROPOFFSTUDS: // Range
            case DROPOFFSTUDSEND: // Range
            case FALLOFFFACTOR: // Range
            case MAGAZINESIZE:
                return true;
            case TTK:
            case RELOAD:
            case RECOILAIM:
            case RECOILHIP:
            case SPREADAIM:
            case SPREADHIP:
            case DETECTIONRADIUS:
            case TIMETOAIM:
            case BURST: // Debatable but I gotta pick something. (Only applies during sorting not a property that gets modified)
                return false;
            default:
                throw std::invalid_argument("Invalid property flag");
        }
    }

    enum PropertyType {
        NONAPPLICABLE,
        MULTIPLIER,
        ADDER
    };

    constexpr PropertyType GetPropertyType(MultFlags flag)
    {
        static_assert(TOTALMULTFLAGS == 20, "PropertyType must be updated to contain all flags");
        switch (flag)
        {
            //// non trivial / non-applicable / not on part properties ////
            // Shouldn't get called //
            case TTK:
            case DPS:
            case DAMAGEEND:
            // Unchangeable //
            case MAGAZINESIZE:
            case TIMETOAIM:
            case BURST:
                return NONAPPLICABLE;
            //// multipliers ////
            case DAMAGE:
            case RELOAD:
            case FIRERATE:
            case PELLETS:
            case DROPOFFSTUDS: // Range
            case DROPOFFSTUDSEND: // Range
            case FALLOFFFACTOR: // Range
            case DETECTIONRADIUS:
            case SPREADAIM:
            case SPREADHIP:
            case RECOILAIM:
            case RECOILHIP:
                return MULTIPLIER;
            //// adders ////
            case HEALTH:
            case MOVEMENTSPEEDMODIFIER:
                return ADDER;
            default:
                throw std::invalid_argument("Invalid property flag");
        }
    }


    // 1, 50, 0.01, 0.25
    inline float reverseQuadraticDamage[51];
    // 1, 55, 0.005, 0.20
    inline float quadraticSpread[56];

    inline float ClampQuadratic(float pelletCount, MultFlags flag)
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

    inline void InitializeClampQuadratic()
    {
        using namespace Input;
        // Precompute because pow and sqrt is scary
        for (int i = 1; i <= 50; i++)
            reverseQuadraticDamage[i] = (float)(0.25f + (sqrtf((50.0f - i) / (50.0f - 1.0f)) * (0.01f - 0.25f)));

        for (int i = 1; i <= 55; i++)
            quadraticSpread[i] = (float)(0.20f + (powf((55.0f - i) / (55.0f - 1.0f), 2.5f) * (0.005f - 0.20f)));

        if (Input::debug)
        {
            std::cout << "reverseQuadraticDamage values:\n";
            for (int i = 1; i <= 50; i++)
                std::cout << std::fixed << std::setprecision(3) << reverseQuadraticDamage[i] << ' ';
            std::cout << "\nQuadraticSpread values:\n";
            for (int i = 1; i <= 55; i++)
                std::cout << std::fixed << std::setprecision(3) << quadraticSpread[i] << ' ';
            std::cout << '\n';
        }

        int maxPartCount = includedPrimary ? 5 : 4; // Secondaries don't have stocks

    }

    inline std::map<std::string, int> fastifyName = {};
    inline int fastifyNoneName() {return fastifyName.size() - 1;} // Returns the name_fast of the "None" part which is always the last part added.

    inline void PartExists(std::string partName)
    {
        if (!fastifyName.contains(partName))
            throw std::invalid_argument("Part " + partName + " doesn't exist");
    }

    inline void InitializeIncludeCategories()
    {
        includeCategories_fast.resize(categoryCount, false);
        for (auto& category : Input::includeCategories)
        {
            // if (category == "AR") category = "Assault Rifle";
            if (!fastifyCategory.contains(category))
                throw std::invalid_argument("Category " + category + " doesn't exist");
            includeCategories_fast[fastifyCategory[category]] = true;
        }

        for (int cat = 0; cat < Fast::categoryCount; cat++)
        {
            if (cat < Fast::primaryCategoryCount) includedPrimary = includedPrimary || includeCategories_fast[cat];
            else includedSecondary = includedSecondary || includeCategories_fast[cat];
        }
    }

    inline void InitializeForceAndBanParts()
    {
        using namespace Input;

        // init force and ban parts to be a bool vector of size fastifyName.size()
        for (int i = 0; i < 5; i++)
        {
            forceParts_fast[i].resize(fastifyName.size(), false);
            banParts_fast[i].resize(fastifyName.size(), false);
        }

        for (auto core: forceCore)
        {
            PartExists(core);
            forceParts_fast[0][fastifyName[core]] = true;
        }
        for (auto magazine: forceMagazine)
        {
            PartExists(magazine);
            forceParts_fast[1][fastifyName[magazine]] = true;
        }
        for (auto barrel: forceBarrel)
        {
            PartExists(barrel);
            forceParts_fast[2][fastifyName[barrel]] = true;
        }
        for (auto stock: forceStock)
        {
            PartExists(stock);
            forceParts_fast[3][fastifyName[stock]] = true;
        }
        for (auto grip: forceGrip)
        {
            PartExists(grip);
            forceParts_fast[4][fastifyName[grip]] = true;
        }

        // Set secondary class's forceStock to be None
        if (includedPrimary && includedSecondary)
            throw std::invalid_argument("Primary and Secondary categories cannot be included together. Please calculate them separately.");
        if (includedSecondary && forcingStock)
            puts("WARNING: Force stock is not allowed in secondary category, reverting to None");
        if (includedSecondary)
        {
            forcingStock = true;   
            forceParts_fast[3][fastifyName["None"]] = true;
        }

        for (auto core: banCore)
        {
            PartExists(core);
            banParts_fast[0][fastifyName[core]] = true;
        }
        for (auto magazine: banMagazine)
        {
            PartExists(magazine);
            banParts_fast[1][fastifyName[magazine]] = true;
        }
        for (auto barrel: banBarrel)
        {
            PartExists(barrel);
            banParts_fast[2][fastifyName[barrel]] = true;
        }
        for (auto stock: banStock)
        {
            PartExists(stock);
            banParts_fast[3][fastifyName[stock]] = true;
        }
        for (auto grip: banGrip)
        {
            PartExists(grip);
            banParts_fast[4][fastifyName[grip]] = true;
        }

    }

    inline bool RangeFilter(const float value, const fpair& range) // Checks if value is within range
    {
        if (range != NILRANGE_P) return range.first <= value && value <= range.second;
        else if (range.first != NILMIN) return range.first <= value;
        else if (range.second != NILMAX) return value <= range.second;
        else return true;
    }
}

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
    Core(const json &jsonObject);

};


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
    float rangeFalloff = 0; // Part of the range property but specifically calculated and stored for falloff calculation in highlow (combination parts). Normal parts don't have this property.

    // Init a default value part
    Part() {}

    template <typename T> // Makes that part that was passed through have no value and be set to None
    static T ToNone(T part)
    {
        if (!Fast::fastifyName.contains("None")) Fast::fastifyName["None"] = Fast::fastifyName.size();
        part.category = "Weird"; // Weird category applies zero penalty debuff.
        part.name = "None";

        part.category_fast = Fast::fastifyCategory["Weird"];
        part.name_fast = Fast::fastifyName["None"];
        return part;
    }

    // Init an identity value part (Can be used to directly apply the * operator onto a gun).
    static Part Identity()
    {
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

    Part(const json& jsonObject);

    void ApplyPenalty(Core *c) // this gets ran during the first loop (core) of DYNAMICPRUNE
    {
        // Applies penalty to stats as a form of precaculation.
        using namespace Fast;
        float penalty = penalties[c->category_fast][category_fast];
        bool sameName = false;
        if (c->name_fast == name_fast)
        {
            penalty = 0;
            sameName = true;
        }

        if ((damage > 0 && MoreIsBetter(DAMAGE)) || (damage < 0 && !MoreIsBetter(DAMAGE)) || sameName)
            damage *= penalty;
        if ((fireRate > 0 && MoreIsBetter(FIRERATE)) || (fireRate < 0 && !MoreIsBetter(FIRERATE)) || sameName)
            fireRate *= penalty;
        if ((spread > 0 && MoreIsBetter(SPREADAIM)) || (spread < 0 && !MoreIsBetter(SPREADAIM)) || sameName)
            spread *= penalty;
        if ((recoil > 0 && MoreIsBetter(RECOILAIM)) || (recoil < 0 && !MoreIsBetter(RECOILAIM)) || sameName)
            recoil *= penalty;
        if ((reloadSpeed > 0 && MoreIsBetter(RELOAD)) || (reloadSpeed < 0 && !MoreIsBetter(RELOAD)) || sameName)
            reloadSpeed *= penalty;
        if ((movementSpeed > 0 && MoreIsBetter(MOVEMENTSPEEDMODIFIER)) || (movementSpeed < 0 && !MoreIsBetter(MOVEMENTSPEEDMODIFIER)) || sameName)
            movementSpeed *= penalty;
        if ((health > 0 && MoreIsBetter(HEALTH)) || (health < 0 && !MoreIsBetter(HEALTH)) || sameName)
            health *= penalty;
        if ((pellets > 0 && MoreIsBetter(PELLETS)) || (pellets < 0 && !MoreIsBetter(PELLETS)) || sameName)
            pellets *= penalty;
        if ((detectionRadius > 0 && MoreIsBetter(DETECTIONRADIUS)) || (detectionRadius < 0 && !MoreIsBetter(DETECTIONRADIUS)) || sameName)
            detectionRadius *= penalty;
        if ((range > 0 && MoreIsBetter(DROPOFFSTUDS)) || (range < 0 && !MoreIsBetter(DROPOFFSTUDS)) || sameName)
            range *= penalty;
        // if ((rangeFalloff > 0 && MoreIsBetter(FALLOFFFACTOR)) || sameName) rangeFalloff *= penalty;
    }

    virtual float GetMult(Fast::MultFlags multFlag) const
    {
        static_assert(Fast::TOTALPARTFLAGS == 14, "GetMult must be updated to contain all part flags");
        using namespace Fast;
        switch(multFlag)
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
            case DROPOFFSTUDSEND:
                return range;
            default:
                std::cout << ("Property not found: " + std::to_string(multFlag)); // debug
                return 0;
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

    float GetMult(Fast::MultFlags multFlag) const override
    {
        if (multFlag == Fast::MAGAZINESIZE) throw std::runtime_error("Magazine size cannot get mult");
        return Part::GetMult(multFlag);
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


class Gun
{
public:
    Barrel *barrel = nullptr;
    Magazine *magazine = nullptr;
    Grip *grip = nullptr;
    Stock *stock = nullptr;
    Core *core = nullptr;

    float damage;
    inline float damageEnd() const { return (damage * falloffFactor) + damage; }
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
    inline float TTKM() const { return (ceilf(Input::playerMaxHealth / damage) - 1) / fireRate; }
    inline float TTKS() const { return TTKM() * 60; }
    inline float DPM() const { return damage * fireRate; } // DPS = DPM / 60
    inline float DPS() const { return DPM() / 60; }

    Gun() {}
    Gun(Core *core) : core(core) {}
    Gun(Barrel *barrel, Magazine *magazine, Grip *grip, Stock *stock, Core *core) : barrel(barrel), magazine(magazine), grip(grip), stock(stock), core(core) {}

    float GetProperty(Fast::MultFlags propertyFlag) const
    {
        static_assert(Fast::TOTALPROPERTYFLAGS == 20, "GetProperty must be updated to contain all property flags");

        switch (propertyFlag)
        {
            case Fast::DAMAGE:
                return damage;
            case Fast::DAMAGEEND:
                return damageEnd();
            case Fast::DROPOFFSTUDS:
                return dropoffStuds.first;
            case Fast::DROPOFFSTUDSEND:
                return dropoffStuds.second;
            case Fast::RELOAD:
                return reloadTime;
            case Fast::MAGAZINESIZE:
                return magazineSize;
            case Fast::FIRERATE:
                return fireRate;
            case Fast::TIMETOAIM:
                return timeToAim;
            case Fast::MOVEMENTSPEEDMODIFIER:
                return movementSpeedModifier;
            case Fast::SPREADAIM:
                return adsSpread;
            case Fast::SPREADHIP:
                return hipfireSpread;
            case Fast::RECOILAIM:
                return recoilAimVertical.second;
            case Fast::RECOILHIP:
                return recoilHipVertical.second;
            case Fast::PELLETS:
                return pellets;
            case Fast::HEALTH:
                return health;
            case Fast::DETECTIONRADIUS:
                return detectionRadius;
            case Fast::FALLOFFFACTOR:
                return falloffFactor;
            case Fast::BURST:
                return burst;
            case Fast::TTK:
                return TTKM();
            case Fast::DPS:
                return DPM(); // this looks so dumb xd
            default:
                throw std::invalid_argument("Invalid property flag");
        }
    }

    float CalculatePenalty(Fast::MultFlags propertyFlag, Part *part, bool direct)
    {
        float baseMult = part->GetMult(propertyFlag);
        if (direct)
            return baseMult;
        if (part->name_fast == core->name_fast) // This actually evaluates first weirdly enough (This also technically can never return due to iterator optimizations)
            return 0;
        if (!Fast::MoreIsBetter(propertyFlag) && baseMult > 0)
            return baseMult;
        if (Fast::MoreIsBetter(propertyFlag) && baseMult < 0)
            return baseMult;

        return baseMult * Fast::penalties[core->category_fast][part->category_fast];
    }

    // Core and magazine copy functions are seperated for prune algorithm
    void CopyCoreValues(multiFlags flags)
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

    void CopyMagazineValues(multiFlags flags)
    {
        using namespace Fast;
        if (flags & MAGAZINESIZE) magazineSize = magazine->magazineSize;
        if (flags & RELOAD) reloadTime = magazine->reloadTime;
    }

    void CopyValues(multiFlags flags)
    {
        if (core != nullptr) CopyCoreValues(flags);
        if (magazine != nullptr) CopyMagazineValues(flags);
    }

    // Used by Prune algorithm //
    float GetPartialFalloff(Fast::MultFlags propertyFlag, Part *part, bool direct)
    {
        return 1 - (CalculatePenalty(propertyFlag, part, direct) / 100);
    }

    float GetPartialMult(Fast::MultFlags propertyFlag, Part *part, bool direct)
    {
        return CalculatePenalty(propertyFlag, part, direct) / 100 + 1;
    }

    float GetPartialAdd(Fast::MultFlags propertyFlag, Part *part, bool direct)
    {
        return CalculatePenalty(propertyFlag, part, direct);
    }

    void CalculatePartialGunStats(multiFlags flags, Part *part, bool direct)
    {
        // Direct is used to determine if the stats should be pulled directly from the part
        // without calculating penalty. (used in dynamicprune)
        if (part == nullptr) throw std::runtime_error("Part is null");

        using namespace Fast;
        if (flags & PELLETS)
            pellets = ceilf(pellets * GetPartialMult(PELLETS, part, direct));
        if (flags & DAMAGE)
        {
            if (core->name_fast != part->name_fast && part->name_fast != Fast::fastifyNoneName())
                damage *= 1 + (CalculatePenalty(DAMAGE, part, direct) / 100) + ClampQuadratic(pellets, DAMAGE);
            if (part == this->stock) damage *= 1 + ClampQuadratic(pellets, DAMAGE); // Extra mult for scope
        }
        if (flags & FIRERATE) fireRate *= GetPartialMult(FIRERATE, part, direct);
        if (flags & MOVEMENTSPEEDMODIFIER) movementSpeedModifier += GetPartialAdd(MOVEMENTSPEEDMODIFIER, part, direct);
        if (flags & HEALTH) health += GetPartialAdd(HEALTH, part, direct);
        if (flags & RELOAD) reloadTime *= GetPartialMult(RELOAD, part, direct);
        if (flags & DETECTIONRADIUS) detectionRadius *= GetPartialMult(DETECTIONRADIUS, part, direct);

        if (flags & (SPREADAIM | SPREADHIP))
        {
            float mult = 1;
            if (core->name_fast != part->name_fast && part->name_fast != Fast::fastifyNoneName())
                mult *= 1 + (CalculatePenalty(SPREADHIP, part, direct) / 100) + ClampQuadratic(pellets, SPREADHIP);
            if (part == this->stock)
                mult *= 1 + ClampQuadratic(pellets, SPREADHIP); // Applies extra mult for scope on stock run

            adsSpread *= mult;
            hipfireSpread *= mult;
        }

        if (flags & DROPOFFSTUDS)
        {
            dropoffStuds.first *= GetPartialMult(DROPOFFSTUDS, part, direct);
            dropoffStuds.second *= GetPartialMult(DROPOFFSTUDS, part, direct);
        }
        if (flags & FALLOFFFACTOR) falloffFactor *= GetPartialFalloff(FALLOFFFACTOR, part, direct);
        if (flags & RECOILHIP)
        {
            float recoilMult = GetPartialMult(RECOILHIP, part, direct);
            recoilHipHorizontal.first *= recoilMult;
            recoilHipHorizontal.second *= recoilMult;
            recoilHipVertical.first *= recoilMult;
            recoilHipVertical.second *= recoilMult;
        }
        if (flags & RECOILAIM)
        {
            float recoilMult = GetPartialMult(RECOILAIM, part, direct);
            recoilAimHorizontal.first *= recoilMult;
            recoilAimHorizontal.second *= recoilMult;
            recoilAimVertical.first *= recoilMult;
            recoilAimVertical.second *= recoilMult;
        }
    }


    // Used by Prune algorithm //
    float GetTotalFalloff(Fast::MultFlags propertyFlag, bool direct)
    {
        float mult = 1;
        if (barrel != nullptr)
            mult *= 1 - (CalculatePenalty(propertyFlag, barrel, direct) / 100);
        if (magazine != nullptr)
            mult *= 1 - (CalculatePenalty(propertyFlag, magazine, direct) / 100);
        if (grip != nullptr)
            mult *= 1 - (CalculatePenalty(propertyFlag, grip, direct) / 100);
        if (stock != nullptr)
            mult *= 1 - (CalculatePenalty(propertyFlag, stock, direct) / 100);
        return mult;
    }

    float GetTotalMult(Fast::MultFlags propertyFlag, bool direct)
    {
        float mult = 1;
        if (barrel != nullptr)
            mult *= CalculatePenalty(propertyFlag, barrel, direct) / 100 + 1;
        if (magazine != nullptr)
            mult *= CalculatePenalty(propertyFlag, magazine, direct) / 100 + 1;
        if (grip != nullptr)
            mult *= CalculatePenalty(propertyFlag, grip, direct) / 100 + 1;
        if (stock != nullptr)
            mult *= CalculatePenalty(propertyFlag, stock, direct) / 100 + 1;
        return mult;
    }

    float GetTotalAdd(Fast::MultFlags propertyFlag, bool direct) // Speed is additive (bruh)
    {
        float add = 0;
        if (barrel != nullptr)
            add += CalculatePenalty(propertyFlag, barrel, direct);
        if (magazine != nullptr)
            add += CalculatePenalty(propertyFlag, magazine, direct);
        if (grip != nullptr)
            add += CalculatePenalty(propertyFlag, grip, direct);
        if (stock != nullptr)
            add += CalculatePenalty(propertyFlag, stock, direct);
        return add;
    }

    void CalculateGunStats(multiFlags flags, bool direct)
    {
        float previousPelletCount[4] = {-1000, -1000, -1000, -1000}; // Magazine Barrel Grip Stock
        using namespace Fast;
        if (flags & PELLETS)
        {
            // Apply based on order of Magazine, Barrel, Stock, Sight, Grip
            // Magazine Before Barrel and Stock + Sight Before Grip. No idea whether stock or sight gets applied first.
            // (There are no sights or stocks that affect pellets so it is impossible to discern without asking a dev)

            pellets *= GetPartialMult(PELLETS, magazine, direct);
            pellets = ceilf(pellets);
            previousPelletCount[0] = pellets;
            pellets *= GetPartialMult(PELLETS, barrel, direct);
            pellets = ceilf(pellets);
            previousPelletCount[1] = pellets;
            pellets *= GetPartialMult(PELLETS, stock, direct);
            pellets = ceilf(pellets);
            previousPelletCount[2] = pellets;
            pellets *= GetPartialMult(PELLETS, grip, direct);
            pellets = ceilf(pellets);
            previousPelletCount[3] = pellets;
        }
        if (flags & DAMAGE)
        {
            float mult = 1;

            // MAGAZINE
            if (core->name_fast != magazine->name_fast || magazine->name_fast == Fast::fastifyNoneName())
                mult *= 1 + (CalculatePenalty(DAMAGE, magazine, direct) / 100) + ClampQuadratic(previousPelletCount[0], DAMAGE);
            // BARREL
            if (core->name_fast != barrel->name_fast || barrel->name_fast == Fast::fastifyNoneName())
                mult *= 1 + (CalculatePenalty(DAMAGE, barrel, direct) / 100) + ClampQuadratic(previousPelletCount[1], DAMAGE);
            // STOCK
            if (core->name_fast != stock->name_fast || stock->name_fast == Fast::fastifyNoneName())
                mult *= 1 + (CalculatePenalty(DAMAGE, stock, direct) / 100) + ClampQuadratic(previousPelletCount[2], DAMAGE);
            // SCOPE
            mult *= 1 + ClampQuadratic(previousPelletCount[2], DAMAGE); // Extra mult for scope
            // GRIP
            if (core->name_fast != grip->name_fast || grip->name_fast == Fast::fastifyNoneName())
                mult *= 1 + (CalculatePenalty(DAMAGE, grip, direct) / 100) + ClampQuadratic(previousPelletCount[3], DAMAGE);

            damage *= mult;
        }
        if (flags & FIRERATE) fireRate *= GetTotalMult(FIRERATE, direct);
        if (flags & MOVEMENTSPEEDMODIFIER) movementSpeedModifier += GetTotalAdd(MOVEMENTSPEEDMODIFIER, direct);
        if (flags & HEALTH) health += GetTotalAdd(HEALTH, direct);
        if (flags & RELOAD) reloadTime *= GetTotalMult(RELOAD, direct);
        if (flags & DETECTIONRADIUS) detectionRadius *= GetTotalMult(DETECTIONRADIUS, direct);
        if ((flags & (SPREADAIM | SPREADHIP)))
        {
            // Mult is identical for both ADS and hipfire
            float mult = 1;

            if (core->name_fast != magazine->name_fast || magazine->name_fast == Fast::fastifyNoneName())
                mult *= 1 + (CalculatePenalty(SPREADHIP, magazine, direct) / 100) + ClampQuadratic(previousPelletCount[0], SPREADHIP);
            if (core->name_fast != barrel->name_fast || barrel->name_fast == Fast::fastifyNoneName())
                mult *= 1 + (CalculatePenalty(SPREADHIP, barrel, direct) / 100) + ClampQuadratic(previousPelletCount[1], SPREADHIP);
            if (core->name_fast != stock->name_fast || stock->name_fast == Fast::fastifyNoneName())
                mult *= 1 + (CalculatePenalty(SPREADHIP, stock, direct) / 100) + ClampQuadratic(previousPelletCount[2], SPREADHIP);
            mult *= 1 + ClampQuadratic(previousPelletCount[2], SPREADHIP); // Extra mult for scope
            if (core->name_fast != grip->name_fast || grip->name_fast == Fast::fastifyNoneName())
                mult *= 1 + (CalculatePenalty(SPREADHIP, grip, direct) / 100) + ClampQuadratic(previousPelletCount[3], SPREADHIP);

            adsSpread *= mult;
            hipfireSpread *= mult;
        }
        if (flags & DROPOFFSTUDS)
        {
            dropoffStuds.first *= GetTotalMult(DROPOFFSTUDS, direct);
            dropoffStuds.second *= GetTotalMult(DROPOFFSTUDS, direct);
        }
        if (flags & FALLOFFFACTOR) falloffFactor *= GetTotalFalloff(FALLOFFFACTOR, direct);
        if (flags & RECOILHIP)
        {
            float recoilMult = GetTotalMult(RECOILHIP, direct);
            recoilHipHorizontal.first *= recoilMult;
            recoilHipHorizontal.second *= recoilMult;
            recoilHipVertical.first *= recoilMult;
            recoilHipVertical.second *= recoilMult;
        }
        if (flags & RECOILAIM)
        {
            float recoilMult = GetTotalMult(RECOILAIM, direct);
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
        CalculateGunStats(ALLMULTFLAG, false);
    }
};


inline std::ostream &operator<<(std::ostream &os, const fpair &fp)
{
    os << fp.first << " - " << fp.second;
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const Gun &gun)
{
    using Input::detailed;
    std::string barrelName = gun.barrel ? gun.barrel->name : "None";
    std::string magazineName = gun.magazine ? gun.magazine->name : "None";
    std::string gripName = gun.grip ? gun.grip->name : "None";
    std::string stockName = gun.stock ? gun.stock->name : "None";
    std::string coreName = gun.core ? gun.core->name : "None";

    os << "[ Barrel: " << barrelName << ", Magazine: " << magazineName << ", Grip: " << gripName << ", Stock: " << stockName << ", Core: " << coreName << " ]\n";
    os << "damage: " << gun.damage << " - " << gun.damageEnd() << "\n";
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
    if (gun.TTKS() != 0 || Input::detailed) os << "TTK: " << gun.TTKS() << " Seconds\n";
    if (Input::sortType == "DPS" || Input::detailed) os << "DPS: " << gun.DPS() << "\n";
    return os;
}

inline std::ostream &operator<<(std::ostream &os, const Part &part)
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

inline std::ostream &operator<<(std::ostream &os, const std::pair<Part, Part> &pair)
{
    os << pair.first << "\n"
       << pair.second << "\n";
    return os;
}

inline Part operator*(Part part1, Part part2) // Used in highlow
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
inline uint64_t barrelCount;
inline uint64_t magazineCount;
inline uint64_t gripCount;
inline uint64_t stockCount;
inline uint64_t coreCount;
inline uint64_t totalCombinations;

inline std::vector<Barrel> barrelList;
inline std::vector<Magazine> magazineList;
inline std::vector<Grip> gripList;
inline std::vector<Stock> stockList;
inline std::vector<Core> coreList;


namespace PQ
{
    /*
    IMPORTANT: SORTING NEEDS TO BE THE INVERSE OF WHAT IS NORMALLY CONSIDERED GOOD
    THE CODE WILL BE POPPING THE !WORST! GUNS OUT FIRST ONLY KEEPING TOP 10
    SORTING NEEDS TO BE LIKE THIS :
        return gun1 < gun2 (Less is better)
        return gun1 > gun2 (More is better)
    */

    enum SortPriorityFlags // Highest returns true
    {
        LOWEST = false,
        HIGHEST = true,
    };

    inline SortPriorityFlags sortPriority; // AUTO priority gets changed in InitializeCurrentSortingType via a Fast::MoreIsBetter() check
    inline Fast::MultFlags currentSortingType;

    struct AllSortStruct {
        bool operator()(const Gun& gun1, const Gun& gun2) const {
            float gun1Property = gun1.GetProperty(currentSortingType);
            float gun2Property = gun2.GetProperty(currentSortingType);
            switch (sortPriority)
            {
                case HIGHEST:
                    return gun1Property > gun2Property;
                case LOWEST:
                    return gun1Property < gun2Property;
                default:
                    throw std::runtime_error("Invalid sorting priority");
            }
        }

        bool operator()(float val1, float val2) const {
            // Compare on value instead of guns (Used in DYNAMICPRUNE)
            // Returns true if gun1 is better than value
            switch (sortPriority)
            {
                case HIGHEST:
                    return val1 > val2;
                case LOWEST:
                    return val1 < val2;
                default:
                    throw std::runtime_error("Invalid sorting priority");
            }
        }
    };

    typedef std::priority_queue<Gun, std::vector<Gun>, AllSortStruct> AllSortPQ;

    inline AllSortPQ topGuns;

    inline void InitializeCurrentSortingType()
    {
        std::string type = Input::sortType;
        using namespace Fast;
        static_assert(Fast::TOTALPROPERTYFLAGS == 20, "InitializeCurrentSortingType must be updated to contain all property flags");
        if (type == "TTK") currentSortingType = TTK;
        else if (type == "DAMAGE") currentSortingType = DAMAGE;
        else if (type == "DAMAGEEND") currentSortingType = DAMAGEEND;
        else if (type == "FIRERATE") currentSortingType = FIRERATE;
        else if (type == "PELLETS") currentSortingType = PELLETS;
        else if (type == "SPREADAIM") currentSortingType = SPREADAIM;
        else if (type == "SPREADHIP") currentSortingType = SPREADHIP;
        else if (type == "RECOILAIM") currentSortingType = RECOILAIM;
        else if (type == "RECOILHIP") currentSortingType = RECOILHIP;
        else if (type == "HEALTH") currentSortingType = HEALTH;
        else if (type == "RANGE") currentSortingType = DROPOFFSTUDS;
        else if (type == "RANGEEND") currentSortingType = DROPOFFSTUDSEND;
        else if (type == "DETECTIONRADIUS") currentSortingType = DETECTIONRADIUS;
        else if (type == "TIMETOAIM") currentSortingType = TIMETOAIM;
        else if (type == "BURST") currentSortingType = BURST;
        else if (type == "SPEED") currentSortingType = MOVEMENTSPEEDMODIFIER;
        else if (type == "MAGAZINE") currentSortingType = MAGAZINESIZE;
        else if (type == "RELOAD") currentSortingType = RELOAD;
        else if (type == "DPS") currentSortingType = DPS;
        else throw std::invalid_argument("Invalid sorting type");

        // Initialize sort priority
        if (Input::sortPriority == "HIGHEST") sortPriority = HIGHEST;
        else if (Input::sortPriority == "LOWEST") sortPriority = LOWEST;
        else if (Input::sortPriority == "AUTO") sortPriority = Fast::MoreIsBetter(currentSortingType) ? HIGHEST: LOWEST;
        else throw std::invalid_argument("Invalid sort prioritization");
    }

}

namespace Filter
{
    using namespace Fast;

    inline multiFlags currentFlags = 0;

    inline void InitializeMultFlag()
    {
        if (Input::pelletRange != NILRANGE_P) currentFlags |= PELLETS;
        if (Input::damageRange != NILRANGE_P) currentFlags |= DAMAGE | PELLETS;
        if (Input::damageEndRange != NILRANGE_P) currentFlags |= DAMAGE | PELLETS | FALLOFFFACTOR;
        if (Input::magazineRange != NILRANGE_P) currentFlags |= MAGAZINESIZE;
        if (Input::movementSpeedRange != NILRANGE_P) currentFlags |= MOVEMENTSPEEDMODIFIER;
        if (Input::spreadHipRange != NILRANGE_P) currentFlags |= SPREADHIP | PELLETS;
        if (Input::spreadAimRange != NILRANGE_P) currentFlags |= SPREADAIM | PELLETS;
        if (Input::recoilHipRange != NILRANGE_P) currentFlags |= RECOILHIP;
        if (Input::recoilAimRange != NILRANGE_P) currentFlags |= RECOILAIM;
        if (Input::fireRateRange != NILRANGE_P) currentFlags |= FIRERATE;
        if (Input::healthRange != NILRANGE_P) currentFlags |= HEALTH;
        if (Input::reloadRange != NILRANGE_P) currentFlags |= RELOAD;
        if (Input::detectionRadiusRange != NILRANGE_P) currentFlags |= DETECTIONRADIUS;
        if (Input::dropoffStudsRange != NILRANGE_P) currentFlags |= DROPOFFSTUDS;
        if (Input::dropoffStudsEndRange != NILRANGE_P) currentFlags |= DROPOFFSTUDSEND;
        if (Input::burstRange != NILRANGE_P) currentFlags |= BURST;
        if (Input::TTKRange != NILRANGE_P) currentFlags |= DAMAGE | PELLETS | FIRERATE;
        if (Input::DPSRange != NILRANGE_P) currentFlags |= DAMAGE | PELLETS | FIRERATE;
        if (Input::timeToAimRange != NILRANGE_P) currentFlags |= TIMETOAIM;

        static_assert(Input::TOTALFILTERCOUNT == 19, "Update IntializeMultFlag");

        switch (PQ::currentSortingType)
        {
            case TTK:
            case DPS:
                currentFlags |= DAMAGE | PELLETS | FIRERATE;
                break;
            case DAMAGEEND:
                currentFlags |= DROPOFFSTUDS;
            case DAMAGE:
                currentFlags |= DAMAGE | PELLETS;
                break;
            case SPREADAIM:
                currentFlags |= SPREADAIM | PELLETS;
                break;
            case SPREADHIP:
                currentFlags |= SPREADHIP | PELLETS;
                break;
            default: // Trivial properties
                currentFlags |= PQ::currentSortingType;
                break;
        }

    }

    inline bool CoreFilter(Core *core) // These filters require no gun calculation and can be immediately checked on the part itself
    {
        if (Fast::banParts_fast[0][core->name_fast]) return false;
        if (Input::forcingCore && !Fast::forceParts_fast[0][core->name_fast]) return false;
        if (!Input::forcingCore && !Fast::includeCategories_fast[core->category_fast]) return false;
        if (!Fast::RangeFilter(core->timeToAim, Input::timeToAimRange)) return false;
        if (!Fast::RangeFilter(core->burst, Input::burstRange)) return false;
        return true;
    }

    inline bool MagazineFilter(Magazine *magazine)
    {
        if (Fast::banParts_fast[1][magazine->name_fast]) return false;
        if (Input::forcingMagazine && !Fast::forceParts_fast[1][magazine->name_fast]) return false;
        if (!Fast::forceParts_fast[1][Fast::fastifyNoneName()] && magazine->name_fast == Fast::fastifyNoneName()) return false;
        if (!Fast::RangeFilter(magazine->magazineSize, Input::magazineRange)) return false;
        return true;
    }

    inline bool BarrelFilter(Barrel *barrel)
    {
        if (Fast::banParts_fast[2][barrel->name_fast]) return false;
        if (Input::forcingBarrel && !Fast::forceParts_fast[2][barrel->name_fast]) return false;
        if (!Fast::forceParts_fast[2][Fast::fastifyNoneName()] && barrel->name_fast == Fast::fastifyNoneName()) return false;
        return true;
    }

    inline bool StockFilter(Stock *stock)
    {
        if (Fast::banParts_fast[3][stock->name_fast]) return false;
        if (Input::forcingStock && !Fast::forceParts_fast[3][stock->name_fast]) return false;
        if (!Fast::forceParts_fast[3][Fast::fastifyNoneName()] && stock->name_fast == Fast::fastifyNoneName()) return false;
        return true;
    }

    inline bool GripFilter(Grip *grip)
    {
        if (Fast::banParts_fast[4][grip->name_fast]) return false;
        if (Input::forcingGrip && !Fast::forceParts_fast[4][grip->name_fast]) return false;
        if (!Fast::forceParts_fast[4][Fast::fastifyNoneName()] && grip->name_fast == Fast::fastifyNoneName()) return false;
        return true;
    }

    inline bool FilterGunStatsOnRange(const Gun& gun)
    {
        using Fast::RangeFilter;
        static_assert(Input::TOTALFILTERCOUNT == 19, "FilterGunStatsOnRange needs to be updated");
        if (!RangeFilter(gun.timeToAim, Input::timeToAimRange)) return false;
        if (!RangeFilter(gun.magazineSize, Input::magazineRange)) return false;
        if (!RangeFilter(gun.pellets, Input::pelletRange)) return false;
        if (!RangeFilter(gun.damage, Input::damageRange)) return false;
        if (!RangeFilter(gun.damageEnd(), Input::damageEndRange)) return false;
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
        if (!RangeFilter(gun.dropoffStuds.second, Input::dropoffStudsEndRange)) return false;
        if (!RangeFilter(gun.burst, Input::burstRange)) return false;
        if (!RangeFilter(gun.TTKM(), Input::TTKRange)) return false;
        if (!RangeFilter(gun.DPM(), Input::DPSRange)) return false;
        return true;
    }

}

// Maybe make this a class or a struct? prob no tho
inline PQ::AllSortPQ threadPQ[64];
inline std::thread threads[64];
inline uint64_t validGunInThread[64];

// namespace BruteForce { }
// namespace Prune { }

namespace DynamicPrune
{
    // Implement prune with dynamic bounding and change to DFS instead of BFS.
    // Also pre calculate part penalties on the core loop for use in the inner loops
    // DFS is implemented via 5 nested for loops.
    // Also change the order of parts to Core -> Mag -> Barrel -> Stock -> Grip to match the pellet calculation (Slight decrease in speed but more accurate.)

    namespace HighLow
    {
        /*
        This namespace revolves around the key data structure used in the
        DYNAMICPRUNE algorithm. It revolves around bestPossibleCombo which is a
        three-dimensional array containing Part objects that contain the best
        multiplier combination both low and high, for every core category and
        from a 1 part combo to a 4 part combo.
        */

        // bestPossibleCombo[coreCategoryFast][low(0) / high(1)][1 -> 4]
        // 1: grip | index 0
        // 2: grip + stock | index 1
        // 3: grip + stock + barrel | index 2
        // 4: grip + stock + barrel + magazine | index 3

        inline std::vector<std::vector<std::vector<Part>>> bestPossibleCombo; // property values are already formatted (in fractional form for mults)

        // bestPossiblePart[coreCategoryFast][low(0) / high(1)][1 -> 4]
        // 1: grip | index 0
        // 2: stock | index 1
        // 3: barrel | index 2
        // 4: magazine | index 3
        inline std::vector<std::vector<std::vector<Part>>> bestPossiblePart; // property values are already formatted (in fractional form for mults)

        inline void InitializeBestPossible() // resize based on category count
        {
            bestPossibleCombo.resize(Fast::categoryCount);
            bestPossiblePart.resize(Fast::categoryCount);
            for (int i = 0; i < Fast::categoryCount; i++)
            {
                bestPossibleCombo[i].resize(2);
                bestPossiblePart[i].resize(2);
                for (int j = 0; j < 2; j++)
                {
                    bestPossibleCombo[i][j].resize(4, Part::Identity());
                    bestPossiblePart[i][j].resize(4, Part::Identity());
                }
            }
        }

        inline void SetHighLowParts(Gun& gun, Part* partToCheck, Part& lowestMultPart, Part& highestMultPart)
        {
            using namespace Fast;
            if (gun.GetPartialMult(DAMAGE, partToCheck, false) < lowestMultPart.damage) lowestMultPart.damage = gun.GetPartialMult(DAMAGE, partToCheck, false);
            if (gun.GetPartialMult(FIRERATE, partToCheck, false) < lowestMultPart.fireRate) lowestMultPart.fireRate = gun.GetPartialMult(FIRERATE, partToCheck, false);
            if (gun.GetPartialMult(SPREADAIM, partToCheck, false) < lowestMultPart.spread) lowestMultPart.spread = gun.GetPartialMult(SPREADAIM, partToCheck, false);
            if (gun.GetPartialMult(RECOILAIM, partToCheck, false) < lowestMultPart.recoil) lowestMultPart.recoil = gun.GetPartialMult(RECOILAIM, partToCheck, false);
            if (gun.GetPartialMult(RELOAD, partToCheck, false) < lowestMultPart.reloadSpeed) lowestMultPart.reloadSpeed = gun.GetPartialMult(RELOAD, partToCheck, false);
            if (gun.GetPartialAdd(MOVEMENTSPEEDMODIFIER, partToCheck, false) < lowestMultPart.movementSpeed) lowestMultPart.movementSpeed = gun.GetPartialAdd(MOVEMENTSPEEDMODIFIER, partToCheck, false);
            if (gun.GetPartialAdd(HEALTH, partToCheck, false) < lowestMultPart.health) lowestMultPart.health = gun.GetPartialAdd(HEALTH, partToCheck, false);
            if (gun.GetPartialMult(PELLETS, partToCheck, false) < lowestMultPart.pellets) lowestMultPart.pellets = gun.GetPartialMult(PELLETS, partToCheck, false);
            if (gun.GetPartialMult(DETECTIONRADIUS, partToCheck, false) < lowestMultPart.detectionRadius) lowestMultPart.detectionRadius = gun.GetPartialMult(DETECTIONRADIUS, partToCheck, false);
            if (gun.GetPartialMult(DROPOFFSTUDS, partToCheck, false) < lowestMultPart.range) lowestMultPart.range = gun.GetPartialMult(DROPOFFSTUDS, partToCheck, false);
            if (gun.GetPartialFalloff(FALLOFFFACTOR, partToCheck, false) < lowestMultPart.rangeFalloff) lowestMultPart.rangeFalloff = gun.GetPartialFalloff(FALLOFFFACTOR, partToCheck, false);

            if (gun.GetPartialMult(DAMAGE, partToCheck, false) > highestMultPart.damage) highestMultPart.damage = gun.GetPartialMult(DAMAGE, partToCheck, false);
            if (gun.GetPartialMult(FIRERATE, partToCheck, false) > highestMultPart.fireRate) highestMultPart.fireRate = gun.GetPartialMult(FIRERATE, partToCheck, false);
            if (gun.GetPartialMult(SPREADAIM, partToCheck, false) > highestMultPart.spread) highestMultPart.spread = gun.GetPartialMult(SPREADAIM, partToCheck, false);
            if (gun.GetPartialMult(RECOILAIM, partToCheck, false) > highestMultPart.recoil) highestMultPart.recoil = gun.GetPartialMult(RECOILAIM, partToCheck, false);
            if (gun.GetPartialMult(RELOAD, partToCheck, false) > highestMultPart.reloadSpeed) highestMultPart.reloadSpeed = gun.GetPartialMult(RELOAD, partToCheck, false);
            if (gun.GetPartialAdd(MOVEMENTSPEEDMODIFIER, partToCheck, false) > highestMultPart.movementSpeed) highestMultPart.movementSpeed = gun.GetPartialAdd(MOVEMENTSPEEDMODIFIER, partToCheck, false);
            if (gun.GetPartialAdd(HEALTH, partToCheck, false) > highestMultPart.health) highestMultPart.health = gun.GetPartialAdd(HEALTH, partToCheck, false);
            if (gun.GetPartialMult(PELLETS, partToCheck, false) > highestMultPart.pellets) highestMultPart.pellets = gun.GetPartialMult(PELLETS, partToCheck, false);
            if (gun.GetPartialMult(DETECTIONRADIUS, partToCheck, false) > highestMultPart.detectionRadius) highestMultPart.detectionRadius = gun.GetPartialMult(DETECTIONRADIUS, partToCheck, false);
            if (gun.GetPartialMult(DROPOFFSTUDS, partToCheck, false) > highestMultPart.range) highestMultPart.range = gun.GetPartialMult(DROPOFFSTUDS, partToCheck, false);
            if (gun.GetPartialFalloff(FALLOFFFACTOR, partToCheck, false) > highestMultPart.rangeFalloff) highestMultPart.rangeFalloff = gun.GetPartialFalloff(FALLOFFFACTOR, partToCheck, false);
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

        inline void InitializeHighestAndLowestMultParts() // It may not look like it but I promise you this has a Time complexity of O(n) I'M TELLING YOU PLEASE BELIEVE ME
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

                bestPossiblePart[g][0][0] = gripMultPair.first;
                bestPossiblePart[g][0][1] = stockMultPair.first;
                bestPossiblePart[g][0][2] = barrelMultPair.first;
                bestPossiblePart[g][0][3] = magazineMultPair.first;
                bestPossiblePart[g][1][0] = gripMultPair.second;
                bestPossiblePart[g][1][1] = stockMultPair.second;
                bestPossiblePart[g][1][2] = barrelMultPair.second;
                bestPossiblePart[g][1][3] = magazineMultPair.second;

                std::pair<Part, Part> *partPairs[4] = {&gripMultPair, &stockMultPair, &barrelMultPair, &magazineMultPair};
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
            if (Input::debug)
            {
                puts("----Best possible part-----");
                // for (int i = 0; i < 6; i++)
                    for (int k = 0; k < 4; k++) for (int j = 0; j < 2; j++) std::cout << bestPossiblePart[0][j][k];
                puts("----Best possible part-----");
                puts("----Best possible combo-----");
                // for (int i = 0; i < 6; i++)
                    for (int k = 0; k < 4; k++) for (int j = 0; j < 2; j++) std::cout << bestPossibleCombo[0][j][k];
                puts("----Best possible combo-----");
            }
        }
    }

    // Global variables shared across threads
    inline std::atomic<float> currentBestThreshold_a; // Keep track of the current best property value and dynamically adjust it as it gets ran.

    inline void InitializeThreshold()
    {
        currentBestThreshold_a.store(PQ::sortPriority == PQ::HIGHEST ? NILMIN : NILMAX);
    }

    // TODO: Consider caching this function.
    inline float ApplyPelletLadder(const Gun& gun, int coreCategory, int partComboStep, const std::vector<Part> &bestPossiblePartList, Fast::MultFlags multType)
    {
        // BestPossiblePartList is the vector of parts that contain the goal multiplier (normal or anti) for the current mult.
        float finalMult = 1;
        float currentPelletCount = gun.pellets;
        // Apply pellet mult ladder for damage
        for (int currentStep = partComboStep; currentStep >= 0; currentStep--)
        {
            currentPelletCount *= bestPossiblePartList[currentStep].pellets;
            currentPelletCount = ceilf(currentPelletCount);

            if (multType != Fast::PELLETS)
                finalMult *= bestPossiblePartList[currentStep].GetMult(multType) + ClampQuadratic(currentPelletCount, multType);
        }
        // Passing multType as pellets gets you the best possible pellet count.
        return multType == Fast::PELLETS ? currentPelletCount : finalMult;
    }


    inline bool ValueOffsetFilter(float valMin, float valMax, const fpair &range)
    {
        if (range == NILRANGE_P) return true; // Should already be checked before coming to this funcion
        if (range.first == NILMIN) return valMin <= range.second;
        if (range.second == NILMAX) return valMax >= range.first;
        return valMax >= range.first && valMin <= range.second;
    }

    inline bool ValueOffsetFilterMult(Fast::MultFlags flag, const fpair &range, const Gun& gun, const Part& lowPPC, const Part& highPPC)
    {
        if (range == NILRANGE_P) return true;
        float valueToCompare = gun.GetProperty(flag);
        float lowMult = lowPPC.GetMult(flag);
        float highMult = highPPC.GetMult(flag);

        return ValueOffsetFilter(valueToCompare * lowMult, valueToCompare * highMult, range);
    }

    inline bool ValueOffsetFilterAdd(Fast::MultFlags flag, const fpair &range, const Gun& gun, const Part& lowPPC, const Part& highPPC)
    {
        if (range == NILRANGE_P) return true;
        float valueToCompare = gun.GetProperty(flag);
        float lowAdd = lowPPC.GetMult(flag);
        float highAdd = highPPC.GetMult(flag);

        return ValueOffsetFilter(valueToCompare + lowAdd, valueToCompare + highAdd, range);
    }

    inline bool IsValidInRange(const Gun& gun, int coreCategory, int partComboStep) // PPC Stands for PossiblePartCombo
    {
        const Part& lowPPC = HighLow::bestPossibleCombo[coreCategory][0][partComboStep]; // lowPPC
        const Part& highPPC = HighLow::bestPossibleCombo[coreCategory][1][partComboStep]; // highPPC

        const std::vector<Part> &lowPP = HighLow::bestPossiblePart[coreCategory][0];
        const std::vector<Part> &highPP = HighLow::bestPossiblePart[coreCategory][1];

        using namespace Fast;

        float minDamageMult = NILMIN;
        float maxDamageMult = NILMAX;
        if (Input::damageRange != NILRANGE_P || Input::damageEndRange != NILRANGE_P || Input::DPSRange != NILRANGE_P || Input::TTKRange != NILRANGE_P)
        {
            minDamageMult = ApplyPelletLadder(gun, coreCategory, partComboStep, lowPP, DAMAGE);
            maxDamageMult = ApplyPelletLadder(gun, coreCategory, partComboStep, highPP, DAMAGE);
            // std::cout << "Damage Range: " << minDamageMult << " - " << maxDamageMult << '\n';
        }

        float minSpreadMult = NILMIN;
        float maxSpreadMult = NILMAX;
        if (Input::spreadAimRange != NILRANGE_P || Input::spreadHipRange != NILRANGE_P)
        {
            // Passing through spreadaim or spreadhip makes no difference (ref: ClampQuadratic())
            minSpreadMult = ApplyPelletLadder(gun, coreCategory, partComboStep, lowPP, SPREADAIM);
            maxSpreadMult = ApplyPelletLadder(gun, coreCategory, partComboStep, highPP, SPREADAIM);
            // std::cout << "Spread Range: " << minSpreadMult << " - " << maxSpreadMult << '\n';
        }

        // Pellets
        if (Input::pelletRange != NILRANGE_P)
        {
            float minPellets = ApplyPelletLadder(gun, coreCategory, partComboStep, lowPP, PELLETS);
            float maxPellets = ApplyPelletLadder(gun, coreCategory, partComboStep, highPP, PELLETS);
            // std::cout << "pellet Range: " << minPellets << " - " << maxPellets << '\n';
            if (!ValueOffsetFilter(minPellets, maxPellets, Input::pelletRange)) return false;
        }

        // Damage
        if (Input::damageRange != NILRANGE_P)
            if (!ValueOffsetFilter(gun.damage * minDamageMult, gun.damage * maxDamageMult, Input::damageRange)) return false;

        // DamageEnd
        if (Input::damageEndRange != NILRANGE_P)
        {
            float damage = gun.damage;

            float highFalloffFactor;
            float lowFalloffFactor;
            if (gun.falloffFactor > 0)
            {
                highFalloffFactor = gun.falloffFactor * highPPC.rangeFalloff;
                lowFalloffFactor = gun.falloffFactor * lowPPC.rangeFalloff;
            }
            else
            {
                lowFalloffFactor = gun.falloffFactor * highPPC.rangeFalloff;
                highFalloffFactor = gun.falloffFactor * lowPPC.rangeFalloff;
            }

            float highDamage = (damage * maxDamageMult * highFalloffFactor) + (damage * maxDamageMult);
            float lowDamage = (damage * minDamageMult * lowFalloffFactor) + (damage * minDamageMult);
            if (!ValueOffsetFilter(lowDamage, highDamage, Input::damageEndRange)) return false;
        }


        // hipspread
        if (Input::spreadHipRange != NILRANGE_P)
        {
            float hipfireSpread = gun.hipfireSpread;
            if (!ValueOffsetFilter(hipfireSpread * minSpreadMult, hipfireSpread * maxSpreadMult, Input::spreadHipRange)) return false;
        }

        // adsspread
        if (Input::spreadAimRange != NILRANGE_P)
        {
            float adsSpread = gun.adsSpread;
            if (!ValueOffsetFilter(adsSpread * minSpreadMult, adsSpread * maxSpreadMult, Input::spreadAimRange)) return false;
        }

        // TTK
        if (Input::TTKRange != NILRANGE_P)
        {
            float minDamage = gun.damage * minDamageMult;
            float maxDamage = gun.damage * maxDamageMult;

            float minFireRate = gun.fireRate * lowPPC.fireRate;
            float maxFireRate = gun.fireRate * highPPC.fireRate;

            // inline float TTKM() const { return (ceilf(Input::playerMaxHealth / damage) - 1) / fireRate; }
            float minTTK = (ceilf(Input::playerMaxHealth / maxDamage) - 1) / maxFireRate;
            float maxTTK = (ceilf(Input::playerMaxHealth / minDamage) - 1) / minFireRate;

            if (!ValueOffsetFilter(minTTK, maxTTK, Input::TTKRange)) return false;
        }

        // DPS
        if (Input::DPSRange != NILRANGE_P)
        {
            float minDamage = gun.damage * minDamageMult;
            float maxDamage = gun.damage * maxDamageMult;

            float minFireRate = gun.fireRate * lowPPC.fireRate;
            float maxFireRate = gun.fireRate * highPPC.fireRate;

            // inline float DPM() const { return damage * fireRate; }
            float minDPS = minDamage * minFireRate;
            float maxDPS = maxDamage * maxFireRate;

            if (!ValueOffsetFilter(minDPS, maxDPS, Input::DPSRange)) return false;
        }

        // Trivial Values
        if (!ValueOffsetFilterMult(FIRERATE, Input::fireRateRange, gun, lowPPC, highPPC)) return false;
        if (!ValueOffsetFilterMult(RECOILHIP, Input::recoilHipRange, gun, lowPPC, highPPC)) return false;
        if (!ValueOffsetFilterMult(RECOILAIM, Input::recoilAimRange, gun, lowPPC, highPPC)) return false;
        if (!ValueOffsetFilterMult(RELOAD, Input::reloadRange, gun, lowPPC, highPPC) && gun.magazine != nullptr) return false;
        if (!ValueOffsetFilterMult(DETECTIONRADIUS, Input::detectionRadiusRange, gun, lowPPC, highPPC)) return false;
        if (!ValueOffsetFilterMult(DROPOFFSTUDS, Input::dropoffStudsRange, gun, lowPPC, highPPC)) return false;
        if (!ValueOffsetFilterMult(DROPOFFSTUDSEND, Input::dropoffStudsEndRange, gun, lowPPC, highPPC)) return false;
        if (!ValueOffsetFilterAdd(MOVEMENTSPEEDMODIFIER, Input::movementSpeedRange, gun, lowPPC, highPPC)) return false;
        if (!ValueOffsetFilterAdd(HEALTH, Input::healthRange, gun, lowPPC, highPPC)) return false;
        return true;
    }

    inline bool IsValidInThreshold(const Gun& gun, int coreCategory, int partComboStep)
    {
        float gunValue;
        const Part& lowPPC = HighLow::bestPossibleCombo[coreCategory][0][partComboStep];
        const Part& highPPC = HighLow::bestPossibleCombo[coreCategory][1][partComboStep];

        const std::vector<Part> &lowPP = HighLow::bestPossiblePart[coreCategory][0];
        const std::vector<Part> &highPP = HighLow::bestPossiblePart[coreCategory][1];

        const Part& normalPPC = PQ::sortPriority ? highPPC : lowPPC;
        const Part& antiPPC = PQ::sortPriority ? lowPPC : highPPC;

        const std::vector<Part> &normalPP = PQ::sortPriority ? highPP : lowPP;
        const std::vector<Part> &antiPP = PQ::sortPriority ? lowPP : highPP;

        using namespace Fast;
        switch(PQ::currentSortingType)
        {
            // Apply pellet ladder if it is a non trivial property
            case TTK: // low TTK -> high damage\\ high firerate -> anti
            {
                float bestDamage = gun.damage;
                float bestFireRate = gun.fireRate * antiPPC.fireRate;

                float damageMult = ApplyPelletLadder(gun, coreCategory, partComboStep, antiPP, DAMAGE);
                bestDamage *= damageMult;
                gunValue = (ceilf(Input::playerMaxHealth / bestDamage) - 1) / bestFireRate;
                break;
            }

            case DPS: // high DPS -> high damage\\ high firerate -> normal
            {
                float bestDamage = gun.damage;
                float bestFireRate = gun.fireRate * antiPPC.fireRate;

                float damageMult = ApplyPelletLadder(gun, coreCategory, partComboStep, normalPP, DAMAGE);
                bestDamage *= damageMult;
                gunValue = bestDamage * bestFireRate;
                break;
            }

            case PELLETS: // high pellets -> high pellets\\ -> normal
                gunValue = ApplyPelletLadder(gun, coreCategory, partComboStep, normalPP, PELLETS);
                break;

            case DAMAGE: // high damage -> high damage\\ -> normal
                gunValue =  gun.damage * ApplyPelletLadder(gun, coreCategory, partComboStep, normalPP, DAMAGE);
                break;

            case DAMAGEEND: // high damageEnd -> high damage\\ (high or low) falloffFactor -> normal
            {
                float bestBaseDamage = gun.damage * ApplyPelletLadder(gun, coreCategory, partComboStep, normalPP, DAMAGE);
                float bestFalloffFactor;
                if (gun.falloffFactor > 0)
                    bestFalloffFactor = gun.falloffFactor * normalPPC.rangeFalloff;
                else
                    bestFalloffFactor = gun.falloffFactor * antiPPC.rangeFalloff;
                gunValue = (bestBaseDamage * bestFalloffFactor) + bestBaseDamage;
            }

            case SPREADAIM: // high spreadAim -> high spread\\ -> normal
                gunValue = gun.adsSpread * ApplyPelletLadder(gun, coreCategory, partComboStep, normalPP, SPREADAIM);
                break;
            case SPREADHIP: // high spreadHip -> high spread\\ -> normal
                gunValue = gun.hipfireSpread * ApplyPelletLadder(gun, coreCategory, partComboStep, normalPP, SPREADAIM);
                break;

            default:
            {
                switch (GetPropertyType(PQ::currentSortingType))
                {
                    case Fast::MULTIPLIER:
                        gunValue = gun.GetProperty(PQ::currentSortingType) * normalPPC.GetMult(PQ::currentSortingType);
                        break;
                    case Fast::ADDER:
                        gunValue = gun.GetProperty(PQ::currentSortingType) + normalPPC.GetMult(PQ::currentSortingType);
                        break;
                    case Fast::NONAPPLICABLE:
                        gunValue = gun.GetProperty(PQ::currentSortingType);
                        break;
                    default:
                        throw std::runtime_error("Invalid property type");
                }
            }
        }

        float threshold = currentBestThreshold_a.load();
        if (PQ::sortPriority == PQ::HIGHEST)
            return gunValue >= threshold;
        else if (PQ::sortPriority == PQ::LOWEST)
            return gunValue <= threshold;
        else
            throw std::runtime_error("Invalid sort priority");
    }

    inline void ReplaceNewestThreshold(const Gun& gun) // https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange.html
    {
        float current = gun.GetProperty(PQ::currentSortingType);
        PQ::AllSortStruct comp;
        float expected;
        int x = 0;
        for (; x < 100; x++)
        {
            expected = currentBestThreshold_a.load();

            if (!comp(current, expected))
                break;

            if (currentBestThreshold_a.compare_exchange_weak(expected, current))
                break;
        }

        if (Input::debug)
            printf("Threshold change %f -> %f\n", expected, current);
        if (x == 100)
            throw std::runtime_error("Failed to replace threshold");
    }

    typedef struct PassThroughArgs // Arguments that stay constant after the core loop
    {
        int threadId;
        int currentCoreCat;
        std::vector<Magazine>& magazineList_penalized;
        std::vector<Barrel>& barrelList_penalized;
        std::vector<Stock>& stockList_penalized;
        std::vector<Grip>& gripList_penalized;
    } PassThroughArgs; // Genuinely looks like reinventing React over here

    // Declerations so I can define the functions in order instead of reverse
    uint64_t CoreLoop(int threadId);
    uint64_t MagazineLoop(const Gun& prevGun, const PassThroughArgs& args);
    uint64_t BarrelLoop(const Gun& prevGun, const PassThroughArgs& args);
    uint64_t StockLoop(const Gun& prevGun, const PassThroughArgs& args);
    void GripLoop(const Gun& prevGun, const PassThroughArgs& args);

    inline uint64_t CoreLoop(int threadId)
    {
        uint64_t prunedEndpoints = 0; // Total endpoints pruned before the final loop
        for (int c = 0; c < coreCount; c++)
        {
            Core *corePtr = coreList.data() + c;
            int currentCoreCat = corePtr->category_fast;

            if (c % Input::threadsToMake != threadId) continue;
            if (!Filter::CoreFilter(corePtr)) continue;
            Gun currentGun = Gun(corePtr);
            currentGun.CopyCoreValues(Filter::currentFlags);
            // std::cout << currentGun << '\n';

            // std::cout << IsValidInRange(currentGun, currentCoreCat, 3) << ' ' << IsValidInThreshold(currentGun, currentCoreCat, 3) << '\n';
            if (
                !IsValidInThreshold(currentGun, currentCoreCat, 3)||
                !IsValidInRange(currentGun, currentCoreCat, 3)
            )
            {
                prunedEndpoints++;
                continue;
            }

            // Generate penalized versions as a pre calculation. (O(n^2) time & memory complexity)
            std::vector<Magazine> magazineList_penalized = magazineList;
            for (auto& a : magazineList_penalized) a.ApplyPenalty(corePtr);

            std::vector<Barrel> barrelList_penalized = barrelList;
            for (auto& a : barrelList_penalized) a.ApplyPenalty(corePtr);

            std::vector<Stock> stockList_penalized = stockList;
            for (auto& a : stockList_penalized) a.ApplyPenalty(corePtr);

            std::vector<Grip> gripList_penalized = gripList;
            for (auto& a : gripList_penalized) a.ApplyPenalty(corePtr);

            PassThroughArgs args = {
                threadId,
                currentCoreCat,
                magazineList_penalized,
                barrelList_penalized,
                stockList_penalized,
                gripList_penalized,
            };

            prunedEndpoints += MagazineLoop(currentGun, args);
        }
        return prunedEndpoints;
    }

    inline uint64_t MagazineLoop(const Gun& prevGun, const PassThroughArgs& args)
    {
        // printf("%d: Processing magazine loop\n", args.threadId);
        uint64_t prunedEndpoints = 0;
        for (int m = 0; m < magazineCount + 1; m++)
        {
            Magazine *magazinePtr = args.magazineList_penalized.data() + m;
            if (!Filter::MagazineFilter(magazinePtr)) continue;

            Gun currentGun = prevGun;
            currentGun.magazine = magazinePtr;
            currentGun.CopyMagazineValues(Filter::currentFlags);
            currentGun.CalculatePartialGunStats(Filter::currentFlags, magazinePtr, true);
            // std::cout << currentGun << '\n';

            if (
                !IsValidInThreshold(currentGun, args.currentCoreCat, 2) ||
                !IsValidInRange(currentGun, args.currentCoreCat, 2)
            )
            {
                prunedEndpoints++;
                continue;
            }

            prunedEndpoints += BarrelLoop(currentGun, args);
        }
        return prunedEndpoints;
    }

    inline uint64_t BarrelLoop(const Gun& prevGun, const PassThroughArgs& args)
    {
        // printf("%d: Processing barrel loop\n", args.threadId);
        uint64_t prunedEndpoints = 0;
        for (int b = 0; b < barrelCount + 1; b++)
        {
            Barrel *barrelPtr = args.barrelList_penalized.data() + b;
            if (!Filter::BarrelFilter(barrelPtr)) continue;

            Gun currentGun = prevGun;
            currentGun.barrel = barrelPtr;
            currentGun.CalculatePartialGunStats(Filter::currentFlags, barrelPtr, true);
            // std::cout << currentGun << '\n';

            if (
                !IsValidInThreshold(currentGun, args.currentCoreCat, 1) ||
                !IsValidInRange(currentGun, args.currentCoreCat, 1)
            )
            {
                prunedEndpoints++;
                continue;
            }

            prunedEndpoints += StockLoop(currentGun, args);
        }
        return prunedEndpoints;
    }

    inline uint64_t StockLoop(const Gun& prevGun, const PassThroughArgs& args)
    {
        // printf("%d: Processing stock loop\n", args.threadId);
        uint64_t prunedEndpoints = 0;
        for (int s = 0; s < stockCount + 1; s++)
        {
            Stock *stockPtr = args.stockList_penalized.data() + s;
            if (!Filter::StockFilter(stockPtr)) continue;

            Gun currentGun = prevGun;
            currentGun.stock = stockPtr;
            currentGun.CalculatePartialGunStats(Filter::currentFlags, stockPtr, true);
            // std::cout << currentGun << '\n';

            if (
                !IsValidInThreshold(currentGun, args.currentCoreCat, 0) ||
                !IsValidInRange(currentGun, args.currentCoreCat, 0)
            )
            {
                prunedEndpoints++;
                continue;
            }

            GripLoop(currentGun, args);
        }
        return prunedEndpoints;
    }

    inline void GripLoop(const Gun& prevGun, const PassThroughArgs& args)
    {
        // printf("%d: Processing grip loop\n", args.threadId);
        for (int g = 0; g < gripCount + 1; g++)
        {
            Grip *gripPtr = args.gripList_penalized.data() + g;
            if (!Filter::GripFilter(gripPtr)) continue;

            Gun currentGun = prevGun;
            currentGun.grip = gripPtr;
            currentGun.CalculatePartialGunStats(Filter::currentFlags, gripPtr, true);
            // std::cout << currentGun << '\n';

            // std::cout << "Filter result: " << Filter::FilterGunStatsOnRange(currentGun)  << " Gun Stats: " << currentGun;
            if (!Filter::FilterGunStatsOnRange(currentGun)) continue;

            validGunInThread[args.threadId]++;

            // In theory the gun should be fully calculated here.
            float expectedThreshold = currentBestThreshold_a.load();
            if (Input::debug)
                printf("%d: Expected Threshold: %f\n", args.threadId, expectedThreshold);
            if (PQ::AllSortStruct()(currentGun.GetProperty(PQ::currentSortingType), expectedThreshold))
            {
                if (threadPQ[args.threadId].size() == Input::howManyTopGunsToDisplay && !PQ::AllSortStruct()(currentGun, threadPQ[args.threadId].top()))
                {
                    // This should theoretically never run. (Threshold in the first if statement should have contained the best value of the top of all priority queues. If this runs then somewhere it fucked up)
                    puts("WARNING: Unexpected atomicity reorder. Please look into this.");
                }

                // Here the current gun contains pointers to components that will go out of scope.
                // We need to return the pointers to the original components.

                // Return pointers to original (non penalized) components
                currentGun.magazine = magazineList.data() + (currentGun.magazine - args.magazineList_penalized.data());
                currentGun.barrel = barrelList.data() + (currentGun.barrel - args.barrelList_penalized.data());
                currentGun.stock = stockList.data() + (currentGun.stock - args.stockList_penalized.data());
                currentGun.grip = gripList.data() + (currentGun.grip - args.gripList_penalized.data());

                threadPQ[args.threadId].push(currentGun);

                if (threadPQ[args.threadId].size() > Input::howManyTopGunsToDisplay)
                    threadPQ[args.threadId].pop();

                // here threadPQ <= Input::howManyTopGunsToDisplay
                assert(threadPQ[args.threadId].size() <= Input::howManyTopGunsToDisplay && "Oh shit");
                if (threadPQ[args.threadId].size() == Input::howManyTopGunsToDisplay)
                    ReplaceNewestThreshold(threadPQ[args.threadId].top());
            }
        }
    }

    inline void Run(int threadId)
    {
        printf("Thread %d started\n", threadId);
        threadPQ[threadId] = PQ::AllSortPQ();
        uint64_t prunedEndpoints = CoreLoop(threadId);
        printf("%d: Pruned endpoints: %lu\n", threadId, prunedEndpoints);
    }
}
#endif

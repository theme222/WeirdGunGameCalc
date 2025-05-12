// Why did I decide to write this in c++? well... I wanted to challenge myself because python would just be wayy too boring
// It's also because I wanted the speed of c but I decided writing structs was a bad idea
// who am I kidding it's because I hate myself
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <queue>
#include <sys/types.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <thread>
#include "include/json.hpp" // "nlohmann/json.hpp"
#include "include/CLI11.hpp" // "CLIUtils"

#define chrono std::chrono
using json = nlohmann::json;
typedef std::pair<float, float> fpair;

#define NILMIN -69420.5f
#define NILMAX 69420.5f
#define NILRANGE {NILMIN, NILMAX} // Hehehheheh

namespace Input
{
    std::string filepath = "Data/FullData.json";
    std::string outpath = "Results.txt";
    std::string sortType = "TTK";

    std::vector<std::string> includeCategories;

    int threadsToMake = 4;
    int howManyTopGunsToDisplay = 10;

    fpair damageRange = NILRANGE;
    fpair magazineRange = NILRANGE;
    fpair spreadRange = NILRANGE;
    fpair recoilRange = NILRANGE;
    fpair movementSpeedRange = NILRANGE;
    fpair fireRateRange = NILRANGE;
    fpair healthRange = NILRANGE;
}


namespace Fast // Namespace to contain any indexing that uses the integer representation of categories and mults
{
    // penalties[coreCategory][partCategory]
    float penalties[6][6] = {
        {1, 0.7, 0.8, 0.75, 1, 0.7},
        {0.7, 1, 0.6, 0.8, 1, 0.6},
        {0.8, 0.6, 1, 0.65, 1, 0.65},
        {0.75, 0.8, 0.65, 1, 1, 0.75},
        {1, 1, 1, 1, 1, 1},
        {0.7, 0.6, 0.65, 0.75, 1, 1},
    };

    std::map<std::string, int> fastifyCategory = {
        {"Assault Rifle", 0},
        {"Sniper", 1},
        {"SMG", 2},
        {"LMG", 3},
        {"Weird", 4},
        {"Shotgun", 5},
    };

    bool includeCategories_fast[5] = {false, false, false, false, false};

    enum MultFlags {
        DAMAGE = 1<<0,
        DROPOFFSTUDS = 1<<1,
        RELOAD = 1<<2,
        MAGAZINESIZE = 1<<3,
        FIRERATE = 1<<4,
        TIMETOAIM = 1<<5,
        MOVEMENTSPEEDMODIFIER = 1<<6,
        SPREAD = 1<<7,
        RECOILAIM = 1<<8,
        RECOILHIP = 1<<9,
        PELLETS = 1<<10,
        HEALTH = 1<<11,
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
            return true;
        case RELOAD:
        case RECOILAIM:
        case RECOILHIP:
        case SPREAD:
            return false;
        default:
            throw std::invalid_argument("Invalid property flag");
        }
    }

    std::map<std::string, int> fastifyName = {};
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

    Part() {}
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
        if (jsonObject.contains("Movement_Speed"))
            movementSpeed = jsonObject["Movement_Speed"];
        if (jsonObject.contains("Reload_Speed"))
            reloadSpeed = jsonObject["Reload_Speed"];
        if (jsonObject.contains("Health"))
            health = jsonObject["Health"];
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
            case SPREAD:
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


uint32_t ALLMULTFLAG = 4294967295; // 2^32 - 1

class Gun
{
public:
    Barrel *barrel;
    Magazine *magazine;
    Grip *grip;
    Stock *stock;
    Core *core;

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
    fpair recoilHipHorizontal;
    fpair recoilHipVertical;
    fpair recoilAimHorizontal;
    fpair recoilAimVertical;

    Gun() {}
    Gun(Barrel *barrel, Magazine *magazine, Grip *grip, Stock *stock, Core *core) : barrel(barrel), magazine(magazine), grip(grip), stock(stock), core(core) {}

    void ReGun(Barrel *barrel, Magazine *magazine, Grip *grip, Stock *stock, Core *core) { // Used for optimization
        this->barrel = barrel;
        this->magazine = magazine;
        this->grip = grip;
        this->stock = stock;
        this->core = core;
    }


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
        mult *= CalculatePenalty(propertyFlag, barrel) / 100 + 1;
        mult *= CalculatePenalty(propertyFlag, magazine) / 100 + 1;
        mult *= CalculatePenalty(propertyFlag, grip) / 100 + 1;
        mult *= CalculatePenalty(propertyFlag, stock) / 100 + 1;
        return mult;
    }

    float GetTotalAdd(Fast::MultFlags propertyFlag) // Speed is additive (bruh)
    {
        float add = 0;
        add += CalculatePenalty(propertyFlag, barrel);
        add += CalculatePenalty(propertyFlag, magazine);
        add += CalculatePenalty(propertyFlag, grip);
        add += CalculatePenalty(propertyFlag, stock);
        return add;
    }

    void CopyValues(uint32_t flags)
    {
        using namespace Fast;
        if (flags & DAMAGE) damage = core->damage;
        if (flags & PELLETS) pellets = core->pellets;
        if (flags & DROPOFFSTUDS) dropoffStuds = core->dropoffStuds;
        if (flags & FIRERATE) fireRate = core->fireRate;
        if (flags & TIMETOAIM) timeToAim = core->timeToAim;
        if (flags & MOVEMENTSPEEDMODIFIER) movementSpeedModifier = core->movementSpeedModifier;
        if (flags & MAGAZINESIZE) magazineSize = magazine->magazineSize;
        if (flags & HEALTH) health = core->health;
        if (flags & RELOAD) reloadTime = magazine->reloadTime;
        if (flags & SPREAD)
        {
            hipfireSpread = core->hipfireSpread;
            adsSpread = core->adsSpread;
        }
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
        if (flags & SPREAD)
        {
            hipfireSpread *= GetTotalMult(SPREAD);
            adsSpread *= GetTotalMult(SPREAD);
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
    os << "Gun contains"
       << "[Barrel: " << gun.barrel->name << ", Magazine: " << gun.magazine->name << ", Grip: " << gun.grip->name << ", Stock: " << gun.stock->name << ", Core: " << gun.core->name << "]\n"
       << "damage: " << gun.damage << "\n"
       << "fireRate: " << gun.fireRate << "\n"
       << "adsSpread: " << gun.adsSpread << "\n"
       << "reloadTime: " << gun.reloadTime << "\n"
       << "magazineSize: " << gun.magazineSize << "\n"
       << "health: " << gun.health << "\n"
       << "recoilAimVertical: " << gun.recoilAimVertical << "\n"
       << "movementSpeed: " << gun.movementSpeedModifier << "\n"
       << "TTK " << CalcTTK(gun) << " Seconds\n";
    return os;
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

Barrel barrelList[64];
Magazine magazineList[64];
Grip gripList[64];
Stock stockList[64];
Core coreList[64];



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
        {
            // if (barrelCount == 0 || magazineCount == 0 || gripCount == 0 || stockCount == 0 || coreCount == 0)
            //     throw std::runtime_error("1 Or more types of parts are missing");
        }
        Iterator(int threadId): threadId(threadId)
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
            do {
                coreIndex++;
            } while (coreIndex % Input::threadsToMake != threadId);

            printf("Processed core index: %d\n", coreIndex);
            return coreIndex < coreCount;
        }
    };

    namespace Filter
    {

        void InitializeIncludeCategories()
        {
            for (auto& category : Input::includeCategories)
            {
                if (category == "AR") category = "Assault Rifle";
                Fast::includeCategories_fast[Fast::fastifyCategory[category]] = true;
            }
        }

        uint32_t currentflags = 0;
        void InitializeMultFlag()
        {
            fpair nilrange = NILRANGE;
            if (Input::damageRange != nilrange) currentflags |= DAMAGE | PELLETS;
            if (Input::magazineRange != nilrange) currentflags |= MAGAZINESIZE;
            if (Input::movementSpeedRange != nilrange) currentflags |= MOVEMENTSPEEDMODIFIER;
            if (Input::spreadRange != nilrange) currentflags |= SPREAD;
            if (Input::recoilRange != nilrange) currentflags |= RECOILAIM;
            if (Input::fireRateRange != nilrange) currentflags |= FIRERATE;
            if (Input::healthRange != nilrange) currentflags |= HEALTH;
            switch (PQ::currentSortingType)
            {
                case PQ::SORTBYTTK:
                    currentflags |= DAMAGE | PELLETS;
                case PQ::SORTBYFIRERATE:
                    currentflags |= FIRERATE;
                    break;
                case PQ::SORTBYSPREAD:
                    currentflags |= SPREAD;
                    break;
            }
        }

        bool RangeFilter(float value, fpair range)
        {
            if (range.first != NILMIN && range.second != NILMAX) return range.first <= value && value <= range.second;
            else if (range.first != NILMIN) return range.first <= value;
            else if (range.second != NILMAX) return value <= range.second;
            else return true;
        }

        bool PreFilter(Barrel* b, Magazine* m, Grip* g, Stock* s, Core* c)
        {
            if (!Fast::includeCategories_fast[c->category_fast]) return false;
            return RangeFilter(m->magazineSize, Input::magazineRange);
        }

        bool PostFilter(Gun& gun)
        {
            if (!RangeFilter(gun.damage.first, Input::damageRange)) return false;
            if (!RangeFilter(gun.adsSpread, Input::spreadRange)) return false;
            if (!RangeFilter(gun.recoilAimVertical.second, Input::recoilRange)) return false;
            if (!RangeFilter(gun.movementSpeedModifier, Input::movementSpeedRange)) return false;
            if (!RangeFilter(gun.fireRate, Input::fireRateRange)) return false;
            if (!RangeFilter(gun.health, Input::healthRange)) return false;
            return true;
        }

    }


    void Run(int threadId)
    {
        Barrel* b; Magazine* m; Grip* g; Stock* s; Core* c;

        printf("Starting thread: %d\n", threadId);

        threadPQ[threadId] = PQ::Create();
        Iterator iter(threadId);
        uint32_t flag = ALLMULTFLAG;

        while (iter.Step(b, m, g, s, c))
        {
            if (!Filter::PreFilter(b, m, g, s, c)) continue;

            Gun currentGun = Gun(b, m, g, s, c);
            currentGun.CopyValues(Filter::currentflags);
            currentGun.CalculateGunStats(Filter::currentflags);

            if (!Filter::PostFilter(currentGun)) continue;

            PQ::Push(threadPQ[threadId], currentGun);
        }
    }
};


int main(int argc, char* argv[])
{
    CLI::App app{"A tool to bruteforce all possible stats inside of weird gun game."};
    argv = app.ensure_utf8(argv);

    app.add_option("-f,--file", Input::filepath, "Path to the json file containing the parts data (Default: Data/FullData.json)");
    app.add_option("-o, --output", Input::outpath, "Path to the output file (Default: Results.txt");
    app.add_option("-t,--threads", Input::threadsToMake, "Number of threads to use (MAX 16) (Default: 4)");
    app.add_option("-s,--sort", Input::sortType, "Sorting type (TTK, FIRERATE, SPREAD)");
    app.add_option("-n,--number", Input::howManyTopGunsToDisplay, "Number of top guns to display (Default: 10)");
    app.add_option("-i, --include", Input::includeCategories, "Categories to include in the calculation");

    app.add_option("--damage", Input::damageRange, "Damage range to filter");
    app.add_option("--damageMin", Input::damageRange.first);
    app.add_option("--damageMax", Input::damageRange.second);
    app.add_option("--magazine", Input::magazineRange, "Size of magazine to filter");
    app.add_option("--magazineMin", Input::magazineRange.first);
    app.add_option("--magazineMax", Input::magazineRange.second);
    app.add_option("--spread", Input::spreadRange, "Spread range to filter");
    app.add_option("--spreadMin", Input::spreadRange.first);
    app.add_option("--spreadMax", Input::spreadRange.second);
    app.add_option("--recoil", Input::recoilRange, "Recoil range to filter");
    app.add_option("--recoilMin", Input::recoilRange.first);
    app.add_option("--recoilMax", Input::recoilRange.second);
    app.add_option("--speed", Input::movementSpeedRange, "Movement speed range to filter");
    app.add_option("--speedMin", Input::movementSpeedRange.first);
    app.add_option("--speedMax", Input::movementSpeedRange.second);
    app.add_option("--fireRate", Input::fireRateRange, "Fire rate range to filter");
    app.add_option("--fireRateMin", Input::fireRateRange.first);
    app.add_option("--fireRateMax", Input::fireRateRange.second);
    app.add_option("--health", Input::healthRange, "Health range to filter");
    app.add_option("--healthMin", Input::healthRange.first);
    app.add_option("--healthMax", Input::healthRange.second);

    CLI11_PARSE(app, argc, argv);

    PQ::SetCurrentSortingType(Input::sortType);
    BruteForce::Filter::InitializeIncludeCategories();
    BruteForce::Filter::InitializeMultFlag();

    std::ifstream f(Input::filepath);
    puts("Reading json file");
    json data = json::parse(f);

    puts("Loading parts into array");

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

    printf("Barrels detected: %d, Magazines detected: %d, Grips detected: %d, Stocks detected: %d, Cores detected: %d\n", barrelCount, magazineCount, gripCount, stockCount, coreCount);
    printf("Total of %u possibilities \n", barrelCount*magazineCount*gripCount*stockCount*coreCount);
    printf("Starting bruteforce with %d threads\n", Input::threadsToMake);

    auto start = chrono::steady_clock::now();

    // Start all threads
    for (int threadId = 0; threadId < Input::threadsToMake; threadId++)
        threads[threadId] = std::thread(BruteForce::Run, threadId);

    // Wait for all threads to finish
    for (int threadId = 0; threadId < Input::threadsToMake; threadId++)
        threads[threadId].join();

    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(end - start);

    puts("Bruteforce completed");
    std::cout << "Elapsed time: " << duration.count() / 60 << " minute(s) and " << duration.count() % 60 << " second(s)\n";

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
        g.CopyAllValues();
        g.CalculateAllGunStats();
        file << g << '\n';
    }
    file.close();
}

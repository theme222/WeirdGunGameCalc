// Why did I decide to write this in c++? well... I wanted to challenge myself because python would just be wayy too boring
// It's also because I wanted the speed of c but I decided writing structs was a bad idea
// who am I kidding it's because I hate myself
#include <iostream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <memory>
#include <fstream>
#include <queue>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstdint>
#include "include/json.hpp" // "nlohmann/json.hpp"
#define chrono std::chrono

using json = nlohmann::json;

json emptyJSON = json::parse(R"({})");

typedef std::pair<float, float> fpair;

std::string filepath = "Data/FullData.json";

// Penalty[coreCategory][partCategory]
float penalties[6][6] = {
    {1, 0.7, 0.8, 0.75, 1, 0.7},
    {0.7, 1, 0.6, 0.8, 1, 0.6},
    {0.8, 0.6, 1, 0.65, 1, 0.65},
    {0.75, 0.8, 0.65, 1, 1, 0.75},
    {1, 1, 1, 1, 1, 1},
    {0.7, 0.6, 0.65, 0.75, 1, 1},
};

std::map<std::string, int> fastCategory = {
    {"Assault Rifle", 0},
    {"Sniper", 1},
    {"SMG", 2},
    {"LMG", 3},
    {"Weird", 4},
    {"Shotgun", 5},
};

std::map<std::string, float> ARPenalties = {
    {"Assault Rifle", 1},
    {"Sniper", 0.7},
    {"SMG", 0.8},
    {"LMG", 0.75},
    {"Weird", 1},
    {"Shotgun", 0.7},
};

std::map<std::string, float> SniperPenalties = {
    {"Assault Rifle", 0.7},
    {"Sniper", 1},
    {"SMG", 0.6},
    {"LMG", 0.8},
    {"Weird", 1},
    {"Shotgun", 0.6},
};

std::map<std::string, float> SMGPenalties = {
    {"Assault Rifle", 0.8},
    {"Sniper", 0.6},
    {"SMG", 1},
    {"LMG", 0.65},
    {"Weird", 1},
    {"Shotgun", 0.65},
};

std::map<std::string, float> LMGPenalties = {
    {"Assault Rifle", 0.75},
    {"Sniper", 0.8},
    {"SMG", 0.65},
    {"LMG", 1},
    {"Weird", 1},
    {"Shotgun", 0.75},
};

std::map<std::string, float> ShotgunPenalties = {
    {"Assault Rifle", 0.7},
    {"Sniper", 0.6},
    {"SMG", 0.65},
    {"LMG", 0.75},
    {"Weird", 1},
    {"Shotgun", 1},
};


std::map<std::string, bool> moreIsBetter = {
    // Based on what Part's multipliers have
    {"damage", true},
    {"movementSpeed", true},
    {"health", true},
    {"fireRate", true},
    {"reloadSpeed", false},
    {"recoil", false},
    {"spread", false},
};

enum MultFlags {
    DAMAGE = 1<<0,
    DROPOFFSTUDS = 1<<1,
    RELOADTIME = 1<<2,
    MAGAZINESIZE = 1<<3,
    FIRERATE = 1<<4,
    TIMETOAIM = 1<<5,
    MOVEMENTSPEEDMODIFIER = 1<<6,
    SPREAD = 1<<7,
    RECOILAIM = 1<<8,
    RECOILHIP = 1<<9,
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

    Part() {}
    Part(const json &jsonObject): category(jsonObject["Category"]), name(jsonObject["Name"])
    {
        category_fast = fastCategory[category];

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

    float GetMult(const std::string &propertyName)
    {
        // Only the multipliers
        if (propertyName == "damage")
            return damage;
        if (propertyName == "fireRate")
            return fireRate;
        if (propertyName == "spread")
            return spread;
        if (propertyName == "recoil")
            return recoil;
        if (propertyName == "reloadSpeed")
            return reloadSpeed;
        if (propertyName == "movementSpeed")
            return movementSpeed;
        if (propertyName == "health")
            return health;

        throw std::invalid_argument("Property not found: " + propertyName);
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

    int category_fast = 0;
    int name_fast = 0;

    fpair damage = fpair(0, 0);
    fpair dropoffStuds = fpair(0, 0);
    float fireRate = 0;
    float hipfireSpread = 0;
    float adsSpread = 0;
    float timeToAim = 0;
    float movementSpeedModifier = 0;
    fpair recoilHipHorizontal = fpair(0, 0);
    fpair recoilHipVertical = fpair(0, 0);
    fpair recoilAimHorizontal = fpair(0, 0);
    fpair recoilAimVertical = fpair(0, 0);

    Core() {}
    Core(const json &jsonObject) : category(jsonObject["Category"]), name(jsonObject["Name"])
    {
        category_fast = fastCategory[category];

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
    float reloadTime;
    float magazineSize;
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


    float CalculatePenalty(const std::string &propertyName, Part *part)
    {
        float baseMult = part->GetMult(propertyName);
        if (part->name == core->name) // This actually evaluates first weirdly enough
            return 0;
        if (!moreIsBetter[propertyName] && baseMult > 0)
            return baseMult;
        if (moreIsBetter[propertyName] && baseMult < 0)
            return baseMult;

        return baseMult * penalties[core->category_fast][part->category_fast];

        // if (core->category == "Weird")
        //     return baseMult;
        // if (core->category == "Assault Rifle")
        //     return baseMult * ARPenalties[part->category];
        // if (core->category == "Sniper")
        //     return baseMult * SniperPenalties[part->category];
        // if (core->category == "SMG")
        //     return baseMult * SMGPenalties[part->category];
        // if (core->category == "LMG")
        //     return baseMult * LMGPenalties[part->category];
        // if (core->category == "Shotgun")
        //     return baseMult * ShotgunPenalties[part->category];

        throw std::invalid_argument("Category doesn't exist " + core->category + "\n");
    }

    float GetTotalMult(const std::string &propertyName)
    {
        float mult = 1;
        mult *= CalculatePenalty(propertyName, barrel) / 100 + 1;
        mult *= CalculatePenalty(propertyName, magazine) / 100 + 1;
        mult *= CalculatePenalty(propertyName, grip) / 100 + 1;
        mult *= CalculatePenalty(propertyName, stock) / 100 + 1;
        return mult;
    }

    float GetTotalAdd(const std::string &propertyName) // Speed is additive (bruh)
    {
        float add = 0;
        add += CalculatePenalty(propertyName, barrel);
        add += CalculatePenalty(propertyName, magazine);
        add += CalculatePenalty(propertyName, grip);
        add += CalculatePenalty(propertyName, stock);
        return add;
    }

    void CopyValues(uint32_t flags)
    {
        if (flags & DAMAGE) damage = core->damage;
        if (flags & DROPOFFSTUDS) dropoffStuds = core->dropoffStuds;
        if (flags & FIRERATE) fireRate = core->fireRate;
        if (flags & TIMETOAIM) timeToAim = core->timeToAim;
        if (flags & MOVEMENTSPEEDMODIFIER) movementSpeedModifier = core->movementSpeedModifier;
        if (flags & MAGAZINESIZE) magazineSize = magazine->magazineSize;
        if (flags & RELOADTIME) reloadTime = magazine->reloadTime;
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
        if (flags & DAMAGE)
        {
            // This is due to a random glitch giving any part connected to core a 1% boost in damage (1.01)^5 = 1.051
            damage.first *= GetTotalMult("damage") * 1.051;
            damage.second *= GetTotalMult("damage") * 1.051;
        }
        if (flags & FIRERATE) fireRate *= GetTotalMult("fireRate");
        if (flags & MOVEMENTSPEEDMODIFIER) movementSpeedModifier += GetTotalAdd("movementSpeed"); // Bruhhhhhhhhh
        if (flags & RELOADTIME) reloadTime *= GetTotalMult("reloadSpeed");
        if (flags & SPREAD)
        {
            hipfireSpread *= GetTotalMult("spread");
            adsSpread *= GetTotalMult("spread");
        }
        if (flags & RECOILHIP)
        {
            float recoilMult = GetTotalMult("recoil");
            recoilHipHorizontal.first *= recoilMult;
            recoilHipHorizontal.second *= recoilMult;
            recoilHipVertical.first *= recoilMult;
            recoilHipVertical.second *= recoilMult;
        }
        if (flags & RECOILAIM)
        {
            float recoilMult = GetTotalMult("recoil");
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

int howManyTopGunsToDisplay = 10;

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

std::priority_queue<Gun, std::vector<Gun>,
    SortByTTK
    // SortByFireRate
    // SortByADSSpread
    > topGuns;

namespace BruteForce
{
    class Iterator
    {
    public:

        int barrelIndex = 0;
        int magazineIndex = 0;
        int gripIndex = 0;
        int stockIndex = 0;
        int coreIndex = 0;

        Iterator()
        {
            // if (barrelCount == 0 || magazineCount == 0 || gripCount == 0 || stockCount == 0 || coreCount == 0)
            //     throw std::runtime_error("1 Or more types of parts are missing");
        }

        bool Step(Barrel*& b, Magazine*& m, Grip*& g, Stock*& s, Core*& c) // Returns true if another combination still exists
        {
            b = barrelList + barrelIndex;
            m = magazineList + magazineIndex;
            g = gripList + gripIndex;
            s = stockList + stockIndex;
            c = coreList + coreIndex;

            std::string coreName = coreList[coreIndex].name;

            do {
                barrelIndex++;
            } while (barrelIndex < barrelCount && barrelList[barrelIndex].name == coreName);

            if (barrelIndex < barrelCount) return true;

            barrelIndex = 0;
            do {
                magazineIndex++;
            } while (magazineIndex < magazineCount && magazineList[magazineIndex].name == coreName);

            if (magazineIndex < magazineCount) return true;

            magazineIndex = 0;
            do {
                gripIndex++;
            } while (gripIndex < gripCount && gripList[gripIndex].name == coreName);

            if (gripIndex < gripCount) return true;

            gripIndex = 0;
            do {
                stockIndex++;
            } while (stockIndex < stockCount && stockList[stockIndex].name == coreName);

            if (stockIndex < stockCount) return true;

            stockIndex = 0;
            coreIndex++;

            return coreIndex < coreCount;
        }
    };

    void Wrapper(const std::function<void()> &func) // Thanks chatgpt
    {
        puts("Starting bruteforce");
        auto start = chrono::steady_clock::now();

        func();

        auto end = chrono::steady_clock::now();
        auto duration = chrono::duration_cast<chrono::seconds>(end - start);

        int minutes = duration.count() / 60;
        int seconds = duration.count() % 60;

        puts("Bruteforce completed");
        std::cout << "Elapsed time: " << minutes << " minute(s) and " << seconds << " second(s)\n";
    }

    Iterator iter;
    Barrel* b;
    Magazine* m;
    Grip* g;
    Stock* s;
    Core* c;

    void LowestTTK()
    {
        puts("Running lowest TTK config");
        uint32_t flag = DAMAGE | FIRERATE | SPREAD | RECOILAIM | MOVEMENTSPEEDMODIFIER;

        while (iter.Step(b, m, g, s, c))
        {
            if (c->category == "Sniper") continue;
            if (b->name == "Honk") continue;
            if (m->magazineSize < 30) continue;
            if (s->name == "Anvil") continue;
            if (m->category == "Shotgun") continue;

            Gun currentGun = Gun(b, m, g, s, c);
            currentGun.CopyValues(flag);
            currentGun.CalculateGunStats(flag);

            if (currentGun.movementSpeedModifier < 0) continue;
            if (currentGun.damage.first >= 100) continue;
            // if (currentGun.damage.first < 20) continue;
            if (currentGun.adsSpread >= 1.05) continue;
            if (currentGun.recoilAimVertical.second > 35) continue;

            topGuns.push(currentGun);
            if (topGuns.size() > howManyTopGunsToDisplay) topGuns.pop();
        }
    }

    void LowestTTKNonSMG()
    {
        uint32_t flag = DAMAGE | FIRERATE | SPREAD | RECOILAIM | MOVEMENTSPEEDMODIFIER;

        while (iter.Step(b, m, g, s, c))
        {
            if (c->category != "Assault Rifle" && c->category != "LMG") continue;
            if (m->magazineSize < 30) continue;
            if (b->name == "Honk") continue;
            if (s->name == "Anvil") continue;

            Gun currentGun = Gun(b, m, g, s, c);
            currentGun.CopyValues(flag);
            currentGun.CalculateGunStats(flag);

            if (currentGun.movementSpeedModifier < 0) continue;
            if (currentGun.damage.first >= 100) continue;
            if (currentGun.damage.first < 22.3) continue;
            if (currentGun.adsSpread >= 0.75) continue;
            if (currentGun.recoilAimVertical.second > 35) continue;

            topGuns.push(currentGun);
            if (topGuns.size() > howManyTopGunsToDisplay) topGuns.pop();
        }
    }


    void Fastest1TapSniper()
    {
        uint32_t flag = DAMAGE | FIRERATE | SPREAD;

        while (iter.Step(b, m, g, s, c))
        {
            if (c->category != "Sniper") continue;
            if (s->name == "Anvil") continue;

            Gun currentGun = Gun(b, m, g, s, c);
            currentGun.CopyValues(flag);
            currentGun.CalculateGunStats(flag);

            if (currentGun.damage.first < 99.95) continue;
            if (currentGun.adsSpread >= 0.15) continue;

            topGuns.push(currentGun);
            if (topGuns.size() > howManyTopGunsToDisplay) topGuns.pop();
        }
    }

    void FastestHeadshotSniper()
    {
        uint32_t flag = DAMAGE | FIRERATE | SPREAD | MOVEMENTSPEEDMODIFIER;

        while (iter.Step(b, m, g, s, c))
        {
            if (c->category != "Sniper") continue;
            if (s->name == "Anvil") continue;

            Gun currentGun = Gun(b, m, g, s, c);
            currentGun.CopyValues(flag);
            currentGun.CalculateGunStats(flag);

            if (currentGun.damage.first < 49.95) continue;
            if (currentGun.movementSpeedModifier < 0) continue;
            if (currentGun.adsSpread >= 0.15) continue;

            topGuns.push(currentGun);
            if (topGuns.size() > howManyTopGunsToDisplay) topGuns.pop();
        }
    }

    void LowestHonkSpread()
    {
        uint32_t flag = DAMAGE | SPREAD | FIRERATE;

        while (iter.Step(b, m, g, s, c))
        {
            if (b->name != "Honk") continue;
            Gun currentGun = Gun(b, m, g, s, c);
            currentGun.CopyValues(flag);
            currentGun.CalculateGunStats(flag);

            if (currentGun.adsSpread > 1) continue;
            if (currentGun.damage.first < 49.95) continue;

            topGuns.push(currentGun);
            if (topGuns.size() > howManyTopGunsToDisplay) topGuns.pop();
        }
    }
};


int main()
{
    std::ifstream f(filepath);
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

    BruteForce::Wrapper(
        /* Use sort by TTK */
        BruteForce::LowestTTK
        // BruteForce::LowestTTKNonSMG
        // BruteForce::LowestHonkSpread
        /* Use sort by firerate */
        // BruteForce::Fastest1TapSniper
        // BruteForce::FastestHeadshotSniper
    );


    // Dump the pq into a vector
    std::vector<Gun> outputGuns;
    int max = topGuns.size();
    for (int i = 0; i < max; i++)
    {
        outputGuns.push_back(topGuns.top());
        topGuns.pop();
    }

    puts("Current top guns are: \n");
    // Reverse print out the answer
    for (int i = outputGuns.size()-1; i >=0; i--)
    {
        Gun g = outputGuns[i];
        g.CopyAllValues();
        g.CalculateAllGunStats();
        std::cout << g << '\n';
    }
}

// TODO: IF NECESSARY IMPLEMENT THREADING

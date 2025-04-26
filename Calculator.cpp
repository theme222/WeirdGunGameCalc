// Why did I decide to write this in c++? well... I wanted to challenge myself because python would just be wayy too boring
// It's also because I wanted the speed of c but I decided writing structs was a bad idea
// who am I kidding it's because I hate myself
#include <iostream>
#include <string>
#include <algorithm>
#include <memory>
#include <fstream>
#include <queue>
#include <vector>
#include <cmath>
#include "include/json.hpp" // "nlohmann/json.hpp"

using json = nlohmann::json;

json emptyJSON = json::parse(R"({})");

typedef std::pair<float, float> fpair;
typedef unsigned long long uint_64_t; 
typedef unsigned long uint_32_t; 


std::string filepath = "Data/FullData.json";

std::map<std::string, float> ARPenalties = {
    {"Assault Rifle", 1},
    {"Sniper", 0.7},
    {"SMG", 0.8},
    {"LMG", 0.75},
    {"Weird", 1},
};

std::map<std::string, float> SniperPenalties = {
    {"Sniper", 1},
    {"Assault Rifle", 0.7},
    {"SMG", 0.6},
    {"LMG", 0.8},
    {"Weird", 1},
};

std::map<std::string, float> SMGPenalties = {
    {"Assault Rifle", 0.8},
    {"Sniper", 0.6},
    {"SMG", 1},
    {"LMG", 0.65},
    {"Weird", 1},
};

std::map<std::string, float> LMGPenalties = {
    {"Assault Rifle", 0.75},
    {"Sniper", 0.8},
    {"SMG", 0.65},
    {"LMG", 1},
    {"Weird", 1},
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


class Part
{
public:
    std::string category;
    std::string name;

    float damage = 0;
    float fireRate = 0;
    float spread = 0;
    float recoil = 0;
    float reloadSpeed = 0;
    float movementSpeed = 0;
    float health = 0;

    Part() {}
    Part(const json &jsonObject)
    {
        if (jsonObject.contains("Category"))
            category = jsonObject["Category"];
        if (jsonObject.contains("Name"))
            name = jsonObject["Name"];
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
        if (jsonObject["Damage"].is_array())
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

Barrel emptyBarrel;
Magazine emptyMagazine;
Grip emptyGrip;
Stock emptyStock;
Core emptyCore;

enum MultFlags {
    DAMAGE = 1<<0,
    DROPOFFSTUDS = 1<<1,
    RELOADTIME = 1<<2,
    MAGAZINESIZE = 1<<3,
    FIRERATE = 1<<4,
    TIMETOAIM = 1<<5,
    MOVEMENTSPEEDMODIFIER = 1<<6,
    SPREAD = 1<<7,
    RECOIL = 1<<8
};

uint_32_t ALLMULTFLAG = 4294967295;

class Gun
{
public:
    Barrel *barrel;
    Magazine *magazine;
    Grip *grip;
    Stock *stock;
    Core *core;

    fpair damage;
    fpair dropoffStuds = fpair(0, 0);
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

    float CalculatePenalty(const std::string &propertyName, Part *part)
    {
        float baseMult = part->GetMult(propertyName);
        if (part->name == core->name) // This actually evaluates first weirdly enough
            return 0;
        if (!moreIsBetter[propertyName] && baseMult > 0)
            return baseMult;
        if (moreIsBetter[propertyName] && baseMult < 0)
            return baseMult;
        if (core->category == "Weird")
            return baseMult;
        if (core->category == "Assault Rifle")
            return baseMult * ARPenalties[part->category];
        if (core->category == "Sniper")
            return baseMult * SniperPenalties[part->category];
        if (core->category == "SMG")
            return baseMult * SMGPenalties[part->category];
        if (core->category == "LMG")
            return baseMult * LMGPenalties[part->category];
        std::cerr << "Category doesn't exist " << core->category << std::endl;
        exit(1);
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
        if (flags & RECOIL)
        {
            recoilHipHorizontal = core->recoilHipHorizontal;
            recoilHipVertical = core->recoilHipVertical;
            recoilAimHorizontal = core->recoilAimHorizontal;
            recoilAimVertical = core->recoilAimVertical;
        }
    }

    void CalculateGunStats(uint32_t flags) 
    {
        if (flags & DAMAGE) 
        {
            damage.first *= GetTotalMult("damage");
            damage.second *= GetTotalMult("damage");
        }
        if (flags & FIRERATE) fireRate *= GetTotalMult("fireRate");
        if (flags & MOVEMENTSPEEDMODIFIER) movementSpeedModifier += GetTotalAdd("movementSpeed"); // Bruhhhhhhhhh
        if (flags & RELOADTIME) reloadTime *= GetTotalMult("reloadSpeed");
        if (flags & SPREAD)
        {
            hipfireSpread *= GetTotalMult("spread");
            adsSpread *= GetTotalMult("spread");
        }
        if (flags & RECOIL)
        {
            float recoilMult = GetTotalMult("recoil");
            recoilHipHorizontal.first *= recoilMult;
            recoilHipHorizontal.second *= recoilMult;
            recoilHipVertical.first *= recoilMult;
            recoilHipVertical.second *= recoilMult;
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

struct SortByTTK{
    bool operator()(const Gun& gun1, const Gun& gun2) const {
        return CalcTTK(gun1) > CalcTTK(gun2);
    }
};

struct SortByFireRate{
    bool operator()(const Gun& gun1, const Gun& gun2) const {
        return gun1.fireRate < gun2.fireRate;
    }
};

int barrelCount;
int magazineCount;
int gripCount;
int stockCount;
int coreCount;

Barrel barrelList[50];
Magazine magazineList[50];
Grip gripList[50];
Stock stockList[50];
Core coreList[50];

std::priority_queue<Gun, std::vector<Gun>, SortByTTK> topGuns;
//std::priority_queue<Gun, std::vector<Gun>, SortByFireRate> topGuns;

namespace BruteForce 
{
    void LowestTTK()
    {
        uint32_t flag = DAMAGE | FIRERATE | SPREAD | RECOIL | MOVEMENTSPEEDMODIFIER;
        for (int c = 0; c < coreCount; c++)
        {
            if (coreList[c].category == "Sniper") continue;
            for (int b = 0; b < barrelCount; b++)
            {
                if (barrelList[b].name == "Honk") continue;
                for (int m = 0; m < magazineCount; m++)
                {
                    if (magazineList[m].magazineSize < 30) continue;
                    for (int g = 0; g < gripCount; g++)
                    {
                        for (int s = 0; s < stockCount; s++)
                        {
                            if (stockList[s].name == "Anvil") continue;
                            Gun currentGun = Gun(barrelList+b, magazineList+m, gripList+g, stockList+s, coreList+c);
                            currentGun.CopyValues(flag);
                            currentGun.CalculateGunStats(flag);
                            if (currentGun.movementSpeedModifier < 0) continue;
                            if (currentGun.damage.first >= 100) continue;
                            if (currentGun.adsSpread > 1.04) continue;
                            //if (currentGun.recoilAimVertical.second > 30) continue;

                            topGuns.push(currentGun);
                        }
                    }
                }
            }
        }
    }

    void Fastest1TapSniper()
    {
        uint32_t flag = DAMAGE | FIRERATE | SPREAD;
        for (int c = 0; c < coreCount; c++)
        {
            if (coreList[c].category != "Sniper") continue;
            for (int b = 0; b < barrelCount; b++)
            {
                for (int m = 0; m < magazineCount; m++)
                {
                    for (int g = 0; g < gripCount; g++)
                    {
                        for (int s = 0; s < stockCount; s++)
                        {
                            if (stockList[s].name == "Anvil") continue;
                            Gun currentGun = Gun(barrelList+b, magazineList+m, gripList+g, stockList+s, coreList+c);
                            currentGun.CopyValues(flag);
                            currentGun.CalculateGunStats(flag);
                            if (currentGun.damage.first < 99.95) continue;
                            if (currentGun.adsSpread >= 0.15) continue;

                            topGuns.push(currentGun);
                        }
                    }
                }
            }
        }
    }

    void FastestHeadshotSniper()
    {
        uint32_t flag = DAMAGE | FIRERATE | SPREAD | MOVEMENTSPEEDMODIFIER;
        for (int c = 0; c < coreCount; c++)
        {
            if (coreList[c].category != "Sniper") continue;
            for (int b = 0; b < barrelCount; b++)
            {
                for (int m = 0; m < magazineCount; m++)
                {
                    for (int g = 0; g < gripCount; g++)
                    {
                        for (int s = 0; s < stockCount; s++)
                        {
                            if (stockList[s].name == "Anvil") continue;
                            Gun currentGun = Gun(barrelList+b, magazineList+m, gripList+g, stockList+s, coreList+c);
                            currentGun.CopyValues(flag);
                            currentGun.CalculateGunStats(flag);
                            if (currentGun.damage.first < 49.95) continue;
                            if (currentGun.movementSpeedModifier < 0) continue;
                            if (currentGun.adsSpread >= 0.15) continue;

                            topGuns.push(currentGun);
                        }
                    }
                }
            }
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

    puts("Starting bruteforce");

    BruteForce::LowestTTK();
    //BruteForce::Fastest1TapSniper();
    //BruteForce::FastestHeadshotSniper();

    puts("Bruteforce completed");
    puts("Current top guns are: ");
    for (int i = 0; i < topGuns.size(); i++)
    {
        if (i == 10) break;
        Gun g = topGuns.top(); 
        g.CopyAllValues();
        g.CalculateAllGunStats();
        std::cout << g << '\n';
        topGuns.pop();
    }
}

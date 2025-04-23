// Why did I decide to write this in c++? well... I wanted to challenge myself because python would just be wayy too boring
// It's also because I wanted the speed of c but I decided writing structs was a bad idea
// who am I kidding it's because I hate myself
#include <iostream>
#include <string>
#include <algorithm>
#include <memory>
#include <fstream>
#include "include/json.hpp" // "nlohmann/json.hpp"

using json = nlohmann::json;

json emptyJSON = json::parse(R"({})");

typedef std::pair<float, float> fpair;

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
        if (!moreIsBetter[propertyName])
            return baseMult;
        if (part->name == core->name)
            return 0;
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

    void CopyInitialValues()
    {
        // Copy values from core
        damage = core->damage;
        dropoffStuds = core->dropoffStuds;
        fireRate = core->fireRate;
        timeToAim = core->timeToAim;
        movementSpeedModifier = core->movementSpeedModifier;
        hipfireSpread = core->hipfireSpread;
        adsSpread = core->adsSpread;
        recoilHipHorizontal = core->recoilHipHorizontal;
        recoilHipVertical = core->recoilHipVertical;
        recoilAimHorizontal = core->recoilAimHorizontal;
        recoilAimVertical = core->recoilAimVertical;

        // Copy values from magazine
        magazineSize = magazine->magazineSize;
        reloadTime = magazine->reloadTime;
    }

    void CalculateGunStats()
    {
        damage.first *= GetTotalMult("damage");
        damage.second *= GetTotalMult("damage");
        fireRate *= GetTotalMult("fireRate");
        movementSpeedModifier *= GetTotalMult("movementSpeed");
        hipfireSpread *= GetTotalMult("spread");
        adsSpread *= GetTotalMult("spread");
        recoilHipHorizontal.first *= GetTotalMult("recoil");
        recoilHipHorizontal.second *= GetTotalMult("recoil");
        recoilHipVertical.first *= GetTotalMult("recoil");
        recoilHipVertical.second *= GetTotalMult("recoil");
        recoilAimHorizontal.first *= GetTotalMult("recoil");
        recoilAimHorizontal.second *= GetTotalMult("recoil");
        recoilAimVertical.first *= GetTotalMult("recoil");
        recoilAimVertical.second *= GetTotalMult("recoil");
        reloadTime *= GetTotalMult("reloadSpeed");
    }
};

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
       << "recoilAimVertical: " << gun.recoilAimVertical << "\n";
    return os;
}

int barrelCount;
int magazineCount;
int gripCount;
int stockCount;
int coreCount;

int main()
{
    std::ifstream f(filepath);
    puts("Reading json file");
    json data = json::parse(f);

    puts("Loading parts into array");
    Barrel barrelList[50];
    Magazine magazineList[50];
    Grip gripList[50];
    Stock stockList[50];
    Core coreList[50];

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
    puts("Starting bruteforce");

    Gun bestGun;
    
    for (int c = 0; c < coreCount; c++)
    {
        if (coreList[c].category != "Sniper") continue;
        for (int b = 0; b < barrelCount; b++)
            for (int m = 0; m < magazineCount; m++)
                for (int g = 0; g < gripCount; g++)
                    for (int s = 0; s < stockCount; s++)
                        {
                            Gun currentGun = Gun(barrelList + b, magazineList + m, gripList + g, stockList + s, coreList + c);
                            currentGun.CopyInitialValues();
                            currentGun.CalculateGunStats();
                            if (currentGun.damage.first > 100.0 && currentGun.fireRate > bestGun.fireRate && currentGun.adsSpread < 0.2)
                                bestGun = currentGun;
                        }

    }

    std::cout << bestGun << std::endl;
}

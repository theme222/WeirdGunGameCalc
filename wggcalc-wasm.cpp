// Webbing it up like spiderman
#include "wggcalc.hpp"
#include "include/json.hpp" // nlhoman::json

#include <cinttypes>
#include <cassert>

#define chrono std::chrono
using json = nlohmann::json;

namespace Clear { // Define functions that will be used to clear filters and other temporary values that exist per run.
    
    void ClearInput() {
        // Clear input namespace
        using namespace Input;
        
        sortType = "TTK";
        method = "DYNAMICPRUNE";
        sortPriority = "AUTO";
        
        version = false;
        detailed = false;
        debug = false;
        testInstall = false;
    
        // These handle if the users force a specific part to be included
        forceBarrel.clear();
        forcingBarrel = false;
        forceMagazine.clear();
        forcingMagazine = false;
        forceCore.clear();
        forcingCore = false;
        forceStock.clear();
        forcingStock = false;
        forceGrip.clear();
        forcingGrip = false;
    
        banBarrel.clear();
        banMagazine.clear();
        banCore.clear();
        banStock.clear();
        banGrip.clear();
        banPriceType.clear(); // Can either be COIN, WC, ROBUX, or SPECIAL
    
        includeCategories.clear();
        threadsToMake = 1;
        
        howManyTopGunsToDisplay = 10;
        playerMaxHealth = 100;
    
        static_assert(TOTALFILTERCOUNT == 19, "Update ClearInput");
        damageRange = NILRANGE_P;
        damageEndRange = NILRANGE_P;
        magazineRange = NILRANGE_P;
        spreadHipRange = NILRANGE_P;
        spreadAimRange = NILRANGE_P;
        recoilHipRange = NILRANGE_P;
        recoilAimRange = NILRANGE_P;
        movementSpeedRange = NILRANGE_P;
        fireRateRange = NILRANGE_P;
        healthRange = NILRANGE_P;
        pelletRange = NILRANGE_P;
        timeToAimRange = NILRANGE_P;
        reloadRange = NILRANGE_P;
        detectionRadiusRange = NILRANGE_P;
        dropoffStudsRange = NILRANGE_P;
        dropoffStudsEndRange = NILRANGE_P;
        burstRange = NILRANGE_P;
        TTKRange = NILRANGE_P;
        DPSRange = NILRANGE_P;
    }
    
    void ClearFast() 
    {
        // Clear fast namespace
        using namespace Fast;
        includeCategories_fast.clear();
        
        for (int i = 0; i < 5; i++) 
        {
            forceParts_fast[i].clear();
            banParts_fast[i].clear();
        }
        
        banPriceType_fast = 0;
    }
    
    void ClearFilter()
    {
        // Clear filter namespace
        using namespace Filter;
        currentFlags = 0;
    }
}

int main() // Initialize dataset and other non changing info
{
    Clear::ClearInput();
    Clear::ClearFast();
    std::cout << "TEST 123\n";
    // json fullDataJSON = json::parse(FullData);
    
    // json &categoriesJSON = fullDataJSON["Categories"];
    // json &dataJSON = fullDataJSON["Data"];
    // json &penaltiesJSON = fullDataJSON["Penalties"];

    // for (auto &[key, value] : categoriesJSON["Primary"].items())
    //     Fast::fastifyCategory[key] = value;

    // for (auto &[key, value] : categoriesJSON["Secondary"].items())
    //     Fast::fastifyCategory[key] = value;

    // Fast::categoryCount = Fast::fastifyCategory.size();
    // Fast::primaryCategoryCount = categoriesJSON["Primary"].size();
    // Fast::secondaryCategoryCount = categoriesJSON["Secondary"].size();

    // Data::barrelCount = dataJSON["Barrels"].size();
    // Data::barrelList.reserve(Data::barrelCount + 1);

    // for (json &element : dataJSON["Barrels"])
    //     Data::barrelList.push_back(element);

    // Data::magazineCount = dataJSON["Magazines"].size();
    // Data::magazineList.reserve(Data::magazineCount + 1);

    // for (json &element : dataJSON["Magazines"])
    //     Data::magazineList.push_back(element);

    // Data::gripCount = dataJSON["Grips"].size();
    // Data::gripList.reserve(Data::gripCount + 1);

    // for (json &element : dataJSON["Grips"])
    //     Data::gripList.push_back(element);

    // Data::stockCount = dataJSON["Stocks"].size();
    // Data::stockList.reserve(Data::stockCount + 1);

    // for (json &element : dataJSON["Stocks"])
    //     Data::stockList.push_back(element);

    // Data::coreCount = dataJSON["Cores"].size();
    // Data::coreList.reserve(Data::coreCount + 1);

    // for (json &element : dataJSON["Cores"])
    //     Data::coreList.push_back(element);

    // // Add the "None" part for help with secondaries. The +1 to reserve is for "None" part
    // // Although it is not a part of the count because it isn't an actual part and I don't want it to be used in PRUNE or BRUTEFORCE.
    // // This was added to help with secondary weapons because they don't have a stock.
    // Data::barrelList.push_back(Part::ToNone(Barrel()));
    // Data::magazineList.push_back(Part::ToNone(Magazine()));
    // Data::gripList.push_back(Part::ToNone(Grip()));
    // Data::stockList.push_back(Part::ToNone(Stock()));

    // Fast::penalties.resize(Fast::categoryCount);
    // for (int i = 0; i < Fast::categoryCount; i++)
    //     Fast::penalties[i].resize(Fast::categoryCount);

    // for (int i = 0; i < Fast::categoryCount; i++)
    //     for (int j = 0; j < Fast::categoryCount; j++)
    //         Fast::penalties[i][j] = penaltiesJSON[i][j];


    // puts("Initializing required data");

    // // Input::FormatInputs();
    // // PQ::InitializeCurrentSortingType();
    // // Fast::InitializeCategoriesFBParts();
    // Fast::InitializeClampQuadratic();
    // // Filter::InitializeMultFlag();
    // DynamicPrune::HighLow::InitializeBestPossible();
    // DynamicPrune::HighLow::InitializeHighestAndLowestMultParts();
    // DynamicPrune::InitializeThreshold();

    return 0;
}
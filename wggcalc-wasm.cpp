// Webbing it up like spiderman
#include "wggcalc.hpp"
#include "include/json.hpp" // nlhoman::json

#include <emscripten/bind.h>
#include <cassert>
#include <stdexcept>

#define DEBUG false
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
        debug = DEBUG;
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
    
        static_assert(TOTALFILTERCOUNT == 20, "Update ClearInput");
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
        equipTimeRange = NILRANGE_P;
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
        for (int i = 0; i < 64; i++) validGunInThread[i] = 0;
    }
    
    void Clear() 
    {
        ClearInput();
        ClearFast();
        ClearFilter();
    }
}

void InitDataset(std::string JSONString) // Initialize dataset and other non changing info
{
    Input::debug = DEBUG;
    json fullDataJSON = json::parse(JSONString);
    
    json &categoriesJSON = fullDataJSON["Categories"];
    json &dataJSON = fullDataJSON["Data"];
    json &penaltiesJSON = fullDataJSON["Penalties"];

    for (auto &[key, value] : categoriesJSON["Primary"].items())
        Fast::fastifyCategory[key] = value;

    for (auto &[key, value] : categoriesJSON["Secondary"].items())
        Fast::fastifyCategory[key] = value;

    Fast::categoryCount = Fast::fastifyCategory.size();
    Fast::primaryCategoryCount = categoriesJSON["Primary"].size();
    Fast::secondaryCategoryCount = categoriesJSON["Secondary"].size();

    Data::barrelCount = dataJSON["Barrels"].size();
    Data::barrelList.reserve(Data::barrelCount + 1);

    for (json &element : dataJSON["Barrels"])
        Data::barrelList.push_back(element);

    Data::magazineCount = dataJSON["Magazines"].size();
    Data::magazineList.reserve(Data::magazineCount + 1);

    for (json &element : dataJSON["Magazines"])
        Data::magazineList.push_back(element);

    Data::gripCount = dataJSON["Grips"].size();
    Data::gripList.reserve(Data::gripCount + 1);

    for (json &element : dataJSON["Grips"])
        Data::gripList.push_back(element);

    Data::stockCount = dataJSON["Stocks"].size();
    Data::stockList.reserve(Data::stockCount + 1);

    for (json &element : dataJSON["Stocks"])
        Data::stockList.push_back(element);

    Data::coreCount = dataJSON["Cores"].size();
    Data::coreList.reserve(Data::coreCount + 1);

    for (json &element : dataJSON["Cores"])
        Data::coreList.push_back(element);

    // Add the "None" part for help with secondaries. The +1 to reserve is for "None" part
    // Although it is not a part of the count because it isn't an actual part and I don't want it to be used in PRUNE or BRUTEFORCE.
    // This was added to help with secondary weapons because they don't have a stock.
    Data::barrelList.push_back(Part::ToNone(Barrel()));
    Data::magazineList.push_back(Part::ToNone(Magazine()));
    Data::gripList.push_back(Part::ToNone(Grip()));
    Data::stockList.push_back(Part::ToNone(Stock()));

    Fast::penalties.resize(Fast::categoryCount);
    for (int i = 0; i < Fast::categoryCount; i++)
        Fast::penalties[i].resize(Fast::categoryCount);

    for (int i = 0; i < Fast::categoryCount; i++)
        for (int j = 0; j < Fast::categoryCount; j++)
            Fast::penalties[i][j] = penaltiesJSON[i][j];

    puts("Initializing required data");

    Fast::InitializeClampQuadratic();
    DynamicPrune::HighLow::InitializeBestPossible();
    DynamicPrune::HighLow::InitializeHighestAndLowestMultParts();
}

void RunCalc() 
{
    Input::FormatInputs();
    PQ::InitializeCurrentSortingType();
    Fast::InitializeCategoriesFBParts();
    Filter::InitializeMultFlag();
    DynamicPrune::InitializeThreshold();
    
    puts("Running calculator!");
    std::cout << "Sorting " << Input::sortType << " by: " << (PQ::sortPriority ? "HIGHEST": "LOWEST") << '\n';

    totalCombinations = Data::barrelCount * Data::magazineCount * Data::gripCount * Data::stockCount * Data::coreCount;
    printf("Barrels detected: %" PRId64 ", Magazines detected: %" PRId64 ", Grips detected: %" PRId64 ", Stocks detected: %" PRId64 ", Cores detected: %" PRId64 "\n", Data::barrelCount, Data::magazineCount, Data::gripCount, Data::stockCount, Data::coreCount);
    printf("Total of %" PRId64 " possibilities \n", totalCombinations);

    DynamicPrune::Run(0);

    uint64_t totalValidGuns = 0;
    for (int i = 0; i < Input::threadsToMake; i++)
        totalValidGuns += validGunInThread[i];
    printf("Total valid gun combinations based on filters: %" PRId64 " / %" PRId64 "\n", totalValidGuns, Data::coreCount * Data::magazineCount * Data::barrelCount * Data::gripCount * Data::stockCount);

    PQ::topGuns = PQ::AllSortPQ();

    // Combine all thread pqs into topguns
    for (int i = 0; i < Input::threadsToMake; i++)
    {
        for (int j = 0; j < Input::howManyTopGunsToDisplay && !threadPQ[i].empty(); j++)
        {
            PQ::topGuns.push(threadPQ[i].top());
            threadPQ[i].pop();
        }
    }

    // Dump all the pqs into a vector
    std::vector<Gun> outputGuns;
    int max = PQ::topGuns.size();
    for (int i = 0; i < max; i++)
    {
        outputGuns.push_back(PQ::topGuns.top());
        PQ::topGuns.pop();
    }

    std::cout << "Current top " << Input::howManyTopGunsToDisplay << " guns are: \n";
    int endSize = outputGuns.size()-1-Input::howManyTopGunsToDisplay;
    // Reverse out the answer
    for (int i = outputGuns.size()-1; i >= 0; i--)
    {
        if (i == endSize) break;
        Gun g = outputGuns[i];
        // Recalculate all values just in case
        g.CopyAllValues();
        g.CalculateAllGunStats();
        std::cout << g << '\n';
    }
    
}

std::string GetVersion()
{
    return __WGGCALC_VERSION__;
}


namespace AddFilter 
{
    void Required(int total, std::string sortType, std::string priority, std::vector<std::string> categories) 
    {
        Input::howManyTopGunsToDisplay = total;
        Input::sortType = sortType;
        Input::sortPriority = priority;
        Input::includeCategories = categories;
    }
    
    void ForceBan(std::string title, std::vector<std::string> partList) 
    {
        if (title == "Force Core") Input::forceCore = partList;
        else if (title == "Force Magazine") Input::forceMagazine = partList;
        else if (title == "Force Barrel") Input::forceBarrel = partList;
        else if (title == "Force Stock") Input::forceStock = partList;
        else if (title == "Force Grip") Input::forceGrip = partList;
        else if (title == "Ban Core") Input::banCore = partList;
        else if (title == "Ban Magazine") Input::banMagazine = partList;
        else if (title == "Ban Barrel") Input::banBarrel = partList;
        else if (title == "Ban Stock") Input::banStock = partList;
        else if (title == "Ban Grip") Input::banGrip = partList;
        else throw std::invalid_argument("Invalid filter: " + title);
    }
    
    void Range(std::string title, std::string type, float num1, float num2)
    {
        fpair pair = std::make_pair(num1, num2);
        
        // Invalidate the other number if its only min or max
        if (type == "min") pair.second = NILMAX;
        else if (type == "max") pair.first = NILMIN;
        
        // This is what peak c++ looks like
        if (title == "TTK") Input::TTKRange = pair;
        else if (title == "DAMAGE") Input::damageRange = pair;
        else if (title == "DAMAGEEND") Input::damageEndRange = pair;
        else if (title == "FIRERATE") Input::fireRateRange = pair;
        else if (title == "PELLETS") Input::pelletRange = pair;
        else if (title == "SPREADAIM") Input::spreadAimRange = pair;
        else if (title == "SPREADHIP") Input::spreadHipRange = pair;
        else if (title == "RECOILAIM") Input::recoilAimRange = pair;
        else if (title == "RECOILHIP") Input::recoilHipRange = pair;
        else if (title == "HEALTH") Input::healthRange = pair;
        else if (title == "RANGE") Input::dropoffStudsRange = pair;
        else if (title == "RANGEEND") Input::dropoffStudsEndRange = pair;
        else if (title == "DETECTIONRADIUS") Input::detectionRadiusRange = pair;
        else if (title == "TIMETOAIM") Input::timeToAimRange = pair;
        else if (title == "EQUIPTIME") Input::equipTimeRange = pair;
        else if (title == "BURST") Input::burstRange = pair;
        else if (title == "SPEED") Input::movementSpeedRange = pair;
        else if (title == "MAGAZINESIZE") Input::magazineRange = pair;
        else if (title == "RELOAD") Input::reloadRange = pair;
        else if (title == "DPS") Input::DPSRange = pair;
        else throw std::invalid_argument("Invalid filter: " + title);
    }
}

EMSCRIPTEN_BINDINGS(types) 
{
    emscripten::register_vector<std::string>("VectorString");
}

EMSCRIPTEN_BINDINGS(util) 
{
    emscripten::function("Clear", &Clear::Clear);
    emscripten::function("InitDataset", &InitDataset);
    emscripten::function("RunCalc", &RunCalc);
    emscripten::function("GetVersion", &GetVersion);
}

EMSCRIPTEN_BINDINGS(setup) 
{
    emscripten::function("AddFilterRequired", &AddFilter::Required);
    emscripten::function("AddFilterForceBan", &AddFilter::ForceBan);
    emscripten::function("AddFilterRange", &AddFilter::Range);
}



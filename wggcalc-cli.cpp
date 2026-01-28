// Why did I decide to write this in c++? well... I wanted to challenge myself because python would just be wayy too boring
// It's also because I wanted the speed of c but I decided writing structs was a bad idea
// who am I kidding it's because I hate myself
#include "include/json.hpp" // "nlohmann/json.hpp"
#include "include/CLI11.hpp" // "CLIUtils"
#include "wggcalc.hpp"

#include <fstream>
#include <chrono>

#define chrono std::chrono
using json = nlohmann::json;


int main(int argc, char* argv[])
{
    CLI::App app{"A tool to bruteforce all possible guns inside of the roblox game Weird Gun Game."};
    argv = app.ensure_utf8(argv);
    
    // Formatting
    app.get_formatter()->column_width(40);
    app.get_formatter()->long_option_alignment_ratio(0.1);
    
    // option groups
    const std::string GROUP_IO = "INPUT / OUTPUT";
    const std::string GROUP_GENERAL = "GENERAL";
    const std::string GROUP_TEST = "TESTING";
    const std::string GROUP_FB = "FORCE / BAN PARTS";
    const std::string GROUP_FILTERS = "FILTERS";

    app.add_flag  ("-v, --version", Input::version, "Display the current version and exit.");
    
    // app.add_option_group(GROUP_IO);
    app.add_option("-d, --dir", Input::fileDir, "Path to the directory containing the json file (Default: Data)")->check(CLI::ExistingDirectory)->group(GROUP_IO);
    app.add_option("-o, --output", Input::outpath, "Path to the output file (Default: Results.txt)")->group(GROUP_IO);
    
    // app.add_option_group(GROUP_GENERAL);
    app.add_option("-t, --threads", Input::threadsToMake, "Number of threads to use (Default: AUTODETECT)")->check(CLI::Range(1u, 64u))->group(GROUP_GENERAL);
    app.add_option("-s, --sort", Input::sortType, "Sorting type (TTK DAMAGE DAMAGEEND FIRERATE PELLETS SPREADAIM SPREADHIP RECOILAIM RECOILHIP HEALTH RANGE RANGEEND DETECTIONRADIUS TIMETOAIM BURST SPEED MAGAZINE RELOAD DPS) (Default: TTK)")->group(GROUP_GENERAL);
    app.add_option("-n, --number", Input::howManyTopGunsToDisplay, "Number of top guns to display (Default: 10)")->group(GROUP_GENERAL);
    app.add_option("-m, --method", Input::method, "Method to use for calculation (BRUTEFORCE, PRUNE, DYNAMICPRUNE) (Default: DYNAMICPRUNE)")->group(GROUP_GENERAL);
    app.add_option("-i, --include", Input::includeCategories, "Categories to include in the calculation (AR, Sniper, LMG, SMG, Shotgun, Weird)")->group(GROUP_GENERAL);
    app.add_option("-p, --priority", Input::sortPriority, "Sort direction priority (HIGHEST, LOWEST) (Default: AUTO)")->group(GROUP_GENERAL);
    app.add_option("--mh, --defaultMaxHealth", Input::playerMaxHealth, "Set the player max health for TTK calculation (Default: 100)")->group(GROUP_GENERAL);
    
    // app.add_option_group(GROUP_TEST);
    app.add_flag  ("--detailed", Input::detailed, "Display all stats of the gun including irrelevant ones")->group(GROUP_TEST);
    app.add_flag  ("--debug", Input::debug, "Enter debug mode (also does --detailed)")->group(GROUP_TEST);
    app.add_flag  ("--testInstall", Input::testInstall, "Test the current installation and attempt to read from data files.")->group(GROUP_TEST);

    // app.add_option_group(GROUP_FB);
    app.add_option("--fb, --forceBarrel", Input::forceBarrel, "Force the calculator to use a list of barrels")->group(GROUP_FB);
    app.add_option("--fm, --forceMagazine", Input::forceMagazine, "Force the calculator to use a list of magazines")->group(GROUP_FB);
    app.add_option("--fg, --forceGrip", Input::forceGrip, "Force the calculator to use a list of grips")->group(GROUP_FB);
    app.add_option("--fs, --forceStock", Input::forceStock, "Force the calculator to use a list of stocks")->group(GROUP_FB);
    app.add_option("--fc, --forceCore", Input::forceCore, "Force the calculator to use a list of cores")->group(GROUP_FB);

    app.add_option("--bb, --banBarrel", Input::banBarrel, "Ban the calculator from using a list of barrels")->group(GROUP_FB);
    app.add_option("--bm, --banMagazine", Input::banMagazine, "Ban the calculator from using a list of magazines")->group(GROUP_FB);
    app.add_option("--bg, --banGrip", Input::banGrip, "Ban the calculator from using a list of grips")->group(GROUP_FB);
    app.add_option("--bs, --banStock", Input::banStock, "Ban the calculator from using a list of stocks")->group(GROUP_FB);
    app.add_option("--bc, --banCore", Input::banCore, "Ban the calculator from using a list of cores")->group(GROUP_FB);
    
    app.add_option("--banPriceType", Input::banPriceType, "Ban the calculator from using a part / core with a cost type (COIN, WC, ROBUX, LIMITED, SPECIAL)")->group(GROUP_FB); // SPECIAL will ban parts that are obtained in abnormal ways (wheel spin, verification, following, etc.)

    // app.add_option_group(GROUP_FILTERS);
    app.add_option("--damage, --damageStart", Input::damageRange, "Damage range to filter (START)")->group(GROUP_FILTERS);
    app.add_option("--damageMin, --damageStartMin", Input::damageRange.first)->group(GROUP_FILTERS);
    app.add_option("--damageMax, --damageStartMax", Input::damageRange.second)->group(GROUP_FILTERS);
    app.add_option("--damageEnd", Input::damageEndRange, "Damage range to filter (END)")->group(GROUP_FILTERS);
    app.add_option("--damageEndMin", Input::damageEndRange.first)->group(GROUP_FILTERS);
    app.add_option("--damageEndMax", Input::damageEndRange.second)->group(GROUP_FILTERS);
    app.add_option("--magazine", Input::magazineRange, "Size of magazine to filter")->group(GROUP_FILTERS);
    app.add_option("--magazineMin", Input::magazineRange.first)->group(GROUP_FILTERS);
    app.add_option("--magazineMax", Input::magazineRange.second)->group(GROUP_FILTERS);
    app.add_option("--spreadHip", Input::spreadHipRange, "Spread range to filter (HIP)")->group(GROUP_FILTERS);
    app.add_option("--spreadHipMin", Input::spreadHipRange.first)->group(GROUP_FILTERS);
    app.add_option("--spreadHipMax", Input::spreadHipRange.second)->group(GROUP_FILTERS);
    app.add_option("--spreadAim", Input::spreadAimRange, "Spread range to filter (AIM)")->group(GROUP_FILTERS);
    app.add_option("--spreadAimMin", Input::spreadAimRange.first)->group(GROUP_FILTERS);
    app.add_option("--spreadAimMax", Input::spreadAimRange.second)->group(GROUP_FILTERS);
    app.add_option("--recoilHip", Input::recoilHipRange, "Recoil range to filter (HIP)")->group(GROUP_FILTERS);
    app.add_option("--recoilHipMin", Input::recoilHipRange.first)->group(GROUP_FILTERS);
    app.add_option("--recoilHipMax", Input::recoilHipRange.second)->group(GROUP_FILTERS);
    app.add_option("--recoilAim", Input::recoilAimRange, "Recoil range to filter (AIM)")->group(GROUP_FILTERS);
    app.add_option("--recoilAimMin", Input::recoilAimRange.first)->group(GROUP_FILTERS);
    app.add_option("--recoilAimMax", Input::recoilAimRange.second)->group(GROUP_FILTERS);
    app.add_option("--speed", Input::movementSpeedRange, "Movement speed range to filter")->group(GROUP_FILTERS);
    app.add_option("--speedMin", Input::movementSpeedRange.first)->group(GROUP_FILTERS);
    app.add_option("--speedMax", Input::movementSpeedRange.second)->group(GROUP_FILTERS);
    app.add_option("--fireRate", Input::fireRateRange, "Fire rate range to filter")->group(GROUP_FILTERS);
    app.add_option("--fireRateMin", Input::fireRateRange.first)->group(GROUP_FILTERS);
    app.add_option("--fireRateMax", Input::fireRateRange.second)->group(GROUP_FILTERS);
    app.add_option("--health", Input::healthRange, "Health range to filter")->group(GROUP_FILTERS);
    app.add_option("--healthMin", Input::healthRange.first)->group(GROUP_FILTERS);
    app.add_option("--healthMax", Input::healthRange.second)->group(GROUP_FILTERS);
    app.add_option("--pellet", Input::pelletRange, "Pellet range to filter")->group(GROUP_FILTERS);
    app.add_option("--pelletMin", Input::pelletRange.first)->group(GROUP_FILTERS);
    app.add_option("--pelletMax", Input::pelletRange.second)->group(GROUP_FILTERS);
    app.add_option("--timeToAim", Input::timeToAimRange, "Time to aim range to filter")->group(GROUP_FILTERS);
    app.add_option("--timeToAimMin", Input::timeToAimRange.first)->group(GROUP_FILTERS);
    app.add_option("--timeToAimMax", Input::timeToAimRange.second)->group(GROUP_FILTERS);
    app.add_option("--reload", Input::reloadRange, "Reload time to filter")->group(GROUP_FILTERS);
    app.add_option("--reloadMin", Input::reloadRange.first)->group(GROUP_FILTERS);
    app.add_option("--reloadMax", Input::reloadRange.second)->group(GROUP_FILTERS);
    app.add_option("--detectionRadius", Input::detectionRadiusRange, "Detection radius range to filter")->group(GROUP_FILTERS);
    app.add_option("--detectionRadiusMin", Input::detectionRadiusRange.first)->group(GROUP_FILTERS);
    app.add_option("--detectionRadiusMax", Input::detectionRadiusRange.second)->group(GROUP_FILTERS);
    app.add_option("--range, --rangeStart", Input::dropoffStudsRange, "Dropoff studs range to filter (START)")->group(GROUP_FILTERS);
    app.add_option("--rangeMin, --rangeStartMin", Input::dropoffStudsRange.first)->group(GROUP_FILTERS);
    app.add_option("--rangeMax, --rangeStartEnd", Input::dropoffStudsRange.second)->group(GROUP_FILTERS);
    app.add_option("--rangeEnd", Input::dropoffStudsEndRange, "Dropoff studs range to filter (END)")->group(GROUP_FILTERS);
    app.add_option("--rangeEndMin", Input::dropoffStudsEndRange.first)->group(GROUP_FILTERS);
    app.add_option("--rangeEndMax", Input::dropoffStudsEndRange.second)->group(GROUP_FILTERS);
    app.add_option("--burst", Input::burstRange, "Burst range to filter")->group(GROUP_FILTERS);
    app.add_option("--burstMin", Input::burstRange.first)->group(GROUP_FILTERS);
    app.add_option("--burstMax", Input::burstRange.second)->group(GROUP_FILTERS);
    app.add_option("--TTK", Input::TTKRange, "TTK range to filter")->group(GROUP_FILTERS); // Input is TTKS but gets turned into TTKM internally
    app.add_option("--TTKMin", Input::TTKRange.first)->group(GROUP_FILTERS);
    app.add_option("--TTKMax", Input::TTKRange.second)->group(GROUP_FILTERS);
    app.add_option("--DPS", Input::DPSRange, "DPS range to filter")->group(GROUP_FILTERS); // Input is DPS but gets turned into DPM internally
    app.add_option("--DPSMin", Input::DPSRange.first)->group(GROUP_FILTERS);
    app.add_option("--DPSMax", Input::DPSRange.second)->group(GROUP_FILTERS);

    CLI11_PARSE(app, argc, argv);

    if (Input::version)
    {
        std::cout << "WeirdGunGameCalc version " << __WGGCALC_VERSION__ << "\n";
        return 0;
    }

    // The operator overloads go c++azy (Python actually has this with Pathlib but we don't talk about that)
    std::filesystem::path fullDataPath = Input::fileDir / "FullData.json";

    if (!std::filesystem::exists(fullDataPath))
    {
        std::cerr << "Files " << fullDataPath << " is required but don't exist (Did you install this in the correct folder?)\n";
        throw std::invalid_argument("Required files not found");
    }

    if (Input::testInstall)
        std::cout << "Attempting to load database...\n";

    std::cout << "Loading from json file " << fullDataPath << "\n";

    std::ifstream FullData(fullDataPath);
    json fullDataJSON = json::parse(FullData);
    
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

    if (Input::testInstall)
    {
        std::cout << "No errors detected. Exiting...\n";
        return 0;
    }

    puts("Initializing required data");

    Input::FormatInputs();
    PQ::InitializeCurrentSortingType();
    Fast::InitializeCategoriesFBParts();
    Fast::InitializeClampQuadratic();
    Filter::InitializeMultFlag();
    DynamicPrune::HighLow::InitializeBestPossible();
    DynamicPrune::HighLow::InitializeHighestAndLowestMultParts();
    DynamicPrune::InitializeThreshold();
    // BruteForce::Filter::InitializeMultFlag();
    // Prune::Filter::InitializeMultFlag();
    // Prune::HighLow::InitializeBestPossibleCombo();
    // Prune::HighLow::InitializeHighestAndLowestMultParts();

    std::cout << "Sorting " << Input::sortType << " by: " << (PQ::sortPriority ? "HIGHEST": "LOWEST") << '\n';

    totalCombinations = Data::barrelCount * Data::magazineCount * Data::gripCount * Data::stockCount * Data::coreCount;
    printf("Barrels detected: %lu, Magazines detected: %lu, Grips detected: %lu, Stocks detected: %lu, Cores detected: %lu\n", Data::barrelCount, Data::magazineCount, Data::gripCount, Data::stockCount, Data::coreCount);
    printf("Total of %lu possibilities \n", totalCombinations);
    printf("Starting %s with %lu threads\n", Input::method.c_str(), Input::threadsToMake);

    auto start = chrono::steady_clock::now();

    // Start selected method
    if (Input::method == "DYNAMICPRUNE")
    {
        for (int threadId = 0; threadId < Input::threadsToMake; threadId++)
            threads[threadId] = std::thread(DynamicPrune::Run, threadId);
    }
    else if (Input::method == "PRUNE")
    {
        puts("This function is no longer supported. Please use switch to another method.");
        throw std::invalid_argument("PRUNE is no longer supported");
    }
    else if (Input::method == "BRUTEFORCE")
    {
        puts("This function is no longer supported. Please use switch to another method.");
        throw std::invalid_argument("BRUTEFORCE is no longer supported");
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
    printf("Total valid gun combinations based on filters: %lu / %lu\n", totalValidGuns, Data::coreCount * Data::magazineCount * Data::barrelCount * Data::gripCount * Data::stockCount);

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
    for (int i = outputGuns.size()-1; i >= 0; i--)
    {
        if (i == endSize) break;
        Gun g = outputGuns[i];
        if (Input::debug)
            std::cout << "Partial calculation of gun " << i << ": \n" << g << '\n';
        // Recalculate all values just in case
        g.CopyAllValues();
        g.CalculateAllGunStats();
        file << g << '\n';
    }
    file.close();
}

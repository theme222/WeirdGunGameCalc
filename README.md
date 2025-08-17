# WeirdGunGameCalc
> A tool to bruteforce all combinations of guns in the roblox game "Weird Gun Game"

## About
`Updated to Sidearm Update`<br/>

**Updates To The Calculator (Not The Dataset) Will Be Put On Hold For The Time Being (or when I feel like it)**

Made using c++ this tool reads data from Data/FullData.json and calculates all possible combinations of guns in [Weird Gun Game](https://www.roblox.com/games/94590879393563/Weird-Gun-Game-UPDATE). Please note that I am not affiliated with the [Redscape Interactive Projects](https://www.roblox.com/communities/35232296/Redscape-Interactive-Projects#!/about)
group and don't have *direct* access to the source code of the game. This is
purely a personal project in my attempt to learn c++. Any interesting builds
will be updated in the `InterestingBuilds` folder. If you find any guns that
have stats that are different from the game please notify me on discord
@theme222 or open an issue here on github.

## Data
The data it reads is the output of `FileFormatter.py` that formats the .txt
files in RawData scraped using a `DataScreenshotter.py` originally provided by
@zyadak. Existing data is also verified by reading roblox memory using
@emma.5829's script (Not here cause it technically is against roblox tos haha).

## Usage

If you are unfamilliar with programmer slang and want the easy step by step guide go [here](https://scribehow.com/shared/How_to_Download_and_Run_Weird_Gun_Game_Calculator__5Ys-2XhCR5-tHUvSz9-y_g)

Download (or clone) this repository and get the [latest
release](https://github.com/theme222/WeirdGunGameCalc/releases) that corresponds
to your operating system (Currently only supported on Linux and Windows) and
place it in the directory of the repository. Run the binary / executable on
the command line by running `./Calculator` or `Calculator.exe` depending on your
operating system. Provide filters as flags by adding `--<filter_name> <value>`
to the command. To test if you installed it correctly, run it with `--help`.

Filters can contain both a minimum amount and a maximum amount. They can be added in 2 ways:
1. Using the `--<filter_name> <min value> <max value>` flag.
2. Using the `--<filter_name><min or max> <value>` flag.

*The qoutes are required for parts that contain spaces or special characters.*
You may also force a specific part type using this syntax `--force<part type> "<part name (CASE SENSITIVE)>"`
You may ban parts by using this syntax `--ban<part type> "<partname (CASE SENSITIVE)>" "<part2>" "<part3>" ...`

You can also look for some example commands that I use to fill the `InterestingBuilds` directory in the `Commands.sh` file.

This is the full list of flags supported:
```sh
# Result of Calculator --help (Slightly modified to look better because it's kinda ugly. Lookin at you CLI11)
-h,   --help                Print this help message and exit
-f,   --file TEXT           Path to the directory containing the json file (Default: Data)
-o,   --output TEXT         Path to the output file (Default: Results.txt)
-t,   --threads INT         Number of threads to use [1, 64] (Default: AUTODETECT)
-s,   --sort TEXT           Sorting type (TTK, FIRERATE, ADSSPREAD, HIPFIRESPREAD, RECOIL, SPEED, HEALTH, MAGAZINE, RELOAD) (Default: TTK)
-n,   --number INT          Number of top guns to display (Default: 10)
-m,   --method TEXT         Method to use for calculation (BRUTEFORCE, PRUNE) (Default: PRUNE)
-i,   --include TEXT        Categories to include in the calculation (AR, Sniper, LMG, SMG, Shotgun, Weird) (REQUIRED)
-d,   --detailed            Display all stats of the gun including irrelevant ones

--fb, --forceBarrel TEXT    Force the calculator to use a specific barrel
--fm, --forceMagazine TEXT  Force the calculator to use a specific magazine
--fg, --forceGrip TEXT      Force the calculator to use a specific grip
--fs, --forceStock TEXT     Force the calculator to use a specific stock
--fc, --forceCore TEXT      Force the calculator to use a specific core
--bb, --banBarrel TEXT      Ban the calculator from using a list of barrels
--bm, --banMagazine TEXT    Ban the calculator from using a list of magazines
--bg, --banGrip TEXT        Ban the calculator from using a list of grips
--bs, --banStock TEXT       Ban the calculator from using a list of stocks
--bc, --banCore TEXT        Ban the calculator from using a list of cores

--damage, --damageStart [FLOAT,FLOAT] Damage range to filter (START)
--damageMin, --damageStartMin FLOAT --damageMax, --damageStartMax FLOAT
--damageEnd [FLOAT,FLOAT] Damage range to filter (END)
--damageEndMin FLOAT --damageEndMax FLOAT
--magazine [FLOAT,FLOAT] Size of magazine to filter
--magazineMin FLOAT --magazineMax FLOAT
--spreadHip [FLOAT,FLOAT] Spread range to filter (HIP)
--spreadHipMin FLOAT --spreadHipMax FLOAT
--spreadAim [FLOAT,FLOAT] Spread range to filter (AIM)
--spreadAimMin FLOAT --spreadAimMax FLOAT
--recoilHip [FLOAT,FLOAT] Recoil range to filter (HIP)
--recoilHipMin FLOAT --recoilHipMax FLOAT
--recoilAim [FLOAT,FLOAT] Recoil range to filter (AIM)
--recoilAimMin FLOAT --recoilAimMax FLOAT
--speed [FLOAT,FLOAT] Movement speed range to filter
--speedMin FLOAT --speedMax FLOAT
--fireRate [FLOAT,FLOAT] Fire rate range to filter
--fireRateMin FLOAT --fireRateMax FLOAT
--health [FLOAT,FLOAT] Health range to filter
--healthMin FLOAT --healthMax FLOAT
--pellet [FLOAT,FLOAT] Pellet range to filter
--pelletMin FLOAT --pelletMax FLOAT
--timeToAim [FLOAT,FLOAT] Time to aim range to filter
--timeToAimMin FLOAT --timeToAimMax FLOAT
--reload [FLOAT,FLOAT] Reload time to filter
--reloadMin FLOAT --reloadMax FLOAT
--detectionRadius [FLOAT,FLOAT] Detection radius range to filter
--detectionRadiusMin FLOAT --detectionRadiusMax FLOAT
--range, --rangeStart [FLOAT,FLOAT] Dropoff studs range to filter (START)
--rangeMin, --rangeStartMin FLOAT --rangeMax, --rangeStartEnd FLOAT
--rangeEnd [FLOAT,FLOAT] Dropoff studs range to filter (END)
--rangeEndMin FLOAT --rangeEndMax FLOAT
--burst [FLOAT,FLOAT] Burst range to filter
--burstMin FLOAT --burstMax FLOAT
```

## Compile from source
### Prerequisites
g++ that supports c++20 onwards.

### Libraries
You will have to download 2 dependencies:

1. [json.hpp](https://github.com/nlohmann/json/releases)
2. [CLI11.hpp](https://www.github.com/CLIUtils/CLI11/releases)

And add it to a new directory called `include` in the same directory as `Calculator.cpp`.

### Run
If compiling for use locally -static is useless and will only increase binary size.
```sh
g++ -std=c++20 -Iinclude Calculator.cpp -o Calculator -Werror -O2 -static
```
No idea how to use cmake lol

## Algorithm Analysis

There are currently 2 methods inside of the calculator: Bruteforce and Prune.
**TL;DR** Prune is effectively the better method although it can use a decent amount of memory if you do not provide restrictive enough filters. Bruteforce will almost always be slower than Prune.

### Bruteforce

`O(n^5) Time Complexity` `O(n) Memory Complexity`

The way this method works is by iterating through all possible combinations of
creating a gun. Each gun can have a unique barrel, magazine, grip, stock, and
core. This is the simplest and first method I derived on solving this. It runs a
total of (n/5)^5 times causing this method to have an O(n^5) time complexity
although it doesn't save any extra data thus having a memory complexity of only
O(n).

### Prune

`O(n^5) Time Complexity` `O(n^4) Memory Complexity`

This method takes advantage of the fact that only a small portion of the
possible combinations are remotely useful. It directly benefits from adding more
and more precise filters to remove unnecessary calculations. Unlike what the
Time and memory complexity suggests, this method is observed to deliver much
faster runtimes based on how inclusive (or exclusive) the provided filters are.
It contains a much more complex algorithm that boils down to 3 steps:
1. It loops through each "level" of creating a gun: (core) -> (core + magazine) -> (core + magazine + barrel) -> (core + magazine + barrel + grip) -> (core + magazine + barrel + grip + stock)
2. For each level it determines whether or not it is a *valid* combination. This is found by checking each property whether it is within range of the filters provided and whether or not the values can be corrected using the rest of the parts to be within range. The values come from checking the highest and lowest possible combination of parts for each stat. For example: The highest increase for firerate using only stocks is 1.07 while the lowest is 0.75. This check is repeated for all stats for every core type (to account for core incompatibility) and for every level. As an example level 1 (core) would use the best part combination of mag + barrel + grip + stock but for level 3 (core + magazine + barrel), it would instead use the best part combo of grip + stock.
3. Once it determines whether or not a part is a valid combination, it saves it into a std::vector to be used as a gun basis on the next level.

This method contains a much higher runtime constant than bruteforce, requiring
both of allocating memory, and copying and writing data onto the std::vector. It
also has a much higher runtime formula (Although not asymptotically higher)
being `(a * n/5) + (a * n/5)^2 + (a * n/5)^3 + (a * n/5)^4 + (a * n/5)^5`
which makes it worse than bruteforce if the `a` is very close to 1 (if the filters
are not restrictive enough). It also requires a lot of memory due to the fact
that we need to save valid combinations from previous levels. The exact amount
is `(n)^4 * 2 * sizeof(Gun)` bytes.

Prune requires the values to be independent from eachother. This is an issue
when it comes to pellet modifier calculation that affects both damage and
spread. The way this issue has been worked around is by modifying the damage
range and spread range in a way where the pellet modifier is always within
range. This is done by dividing the bottom end by the (max pellet mod)^5 and
dividing the top end by the (min pellet mod)^1. This combined with no pellet
modifier calculation within the Prune method, will ensure that valid damage and
spread values are within range. Pellet modifier calculation is done on the last
step and filtered out with the actual values provided by the user.

Also the second damage stat is calculated by using a stat called `falloffFactor`
which isn't accessible directly from the game but instead it is either taken
from the discord or calculated automatically using the formula `falloffFactor =
(damageEnd - damageStart) / (damageStart)`. This is also causes an issue with
prune similar to the pellet modifier issue but it has a much simpler modifier
allowing us to calculate this value while inside of the prune method.

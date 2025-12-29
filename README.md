# WeirdGunGameCalc
> A tool to bruteforce all combinations of guns in the roblox game "Weird Gun Game"

## About
`Updated to Christmas Update`<br/>

**Updates To The Calculator (Not The Dataset) Will Be Put On Hold For The Time Being (or when I feel like it)**

Made using c++ this tool reads data from Data/FullData.json and calculates all possible combinations of guns in [Weird Gun Game](https://www.roblox.com/games/94590879393563/Weird-Gun-Game-UPDATE). Please note that I am not affiliated with the [Redscape Interactive Projects](https://www.roblox.com/communities/35232296/Redscape-Interactive-Projects#!/about)
group and don't have *direct* access to the source code of the game. This is
purely a personal project in my attempt to learn c++. Any interesting builds
will be updated in the `InterestingBuilds` folder. If you find any guns that
have stats that are different from the game please notify me on discord
@theme222 or open an issue here on github.

## Dataset

Previously data was read from a list of .txt files that contained the partlist
originally provided by @zyadak. This was then verified later by reading roblox
memory using @emma.5829's script. Currently however, the data has been merged
with @spiderkitty410's google sheet which you can find
[here](https://docs.google.com/spreadsheets/d/1Kc9aME3xlUC_vV5dFRe457OchqUOrwuiX_pQykjCF68/edit?gid=911413911#gid=911413911)
(This is due to the fact that I am not as available as I used to be to update
this every week). Currently I have a script `ParseSheet.py` that will
automatically download the latest version of the sheet as a csv and format +
update the data placing it in the `Data` folder.

## Usage

If you are unfamilliar with programmer *slang* and want the easy step by step guide go [here](https://scribehow.com/shared/How_to_Download_and_Run_Weird_Gun_Game_Calculator__5Ys-2XhCR5-tHUvSz9-y_g)

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

You can also look for some example commands that I use to fill the
`InterestingBuilds` directory in the `Commands.sh` file.

This is the full list of flags supported:

```sh
# Result of Calculator --help (Modified to look better because it's kinda ugly. Lookin at you CLI11)
-h,   --help                 Print this help message and exit
-f,   --file TEXT            Path to the directory containing the json file (Default: Data)
-o,   --output TEXT          Path to the output file (Default: Results.txt)
-t,   --threads INT          Number of threads to use [1, 64] (Default: AUTODETECT)
-s,   --sort TEXT            Sorting type (TTK, FIRERATE, ADSSPREAD, HIPFIRESPREAD, RECOIL,
                             SPEED, HEALTH, MAGAZINE, RELOAD, DPS) (Default: TTK)
-n,   --number INT           Number of top guns to display (Default: 10)
-m,   --method TEXT          Method to use for calculation (BRUTEFORCE, PRUNE) (Default: PRUNE)
-i,   --include TEXT         Categories to include in the calculation (AR, Sniper, LMG,
                             SMG, Shotgun, Weird, Sidearm) (REQUIRED)
--mh, --defaultMaxHealth INT Set the player max health for TTK calculation (Default: 100)
-p,   --priority TEXT        Sort direction priority (HIGHEST, LOWEST) (Default: AUTO)
-v,     --version            Display the current version and exit.
--detailed                   Display all stats of the gun including irrelevant ones
--debug                      Enter debug mode
--testInstall                Test the current installation and attempt to read from data files.

--fb, --forceBarrel TEXT ...   Force the calculator to use a list of barrels
--fm, --forceMagazine TEXT ... Force the calculator to use a list of magazines
--fg, --forceGrip TEXT ...     Force the calculator to use a list of grips
--fs, --forceStock TEXT ...    Force the calculator to use a list of stocks
--fc, --forceCore TEXT ...     Force the calculator to use a list of cores
--bb, --banBarrel TEXT ...     Ban the calculator from using a list of barrels
--bm, --banMagazine TEXT ...   Ban the calculator from using a list of magazines
--bg, --banGrip TEXT ...       Ban the calculator from using a list of grips
--bs, --banStock TEXT ...      Ban the calculator from using a list of stocks
--bc, --banCore TEXT ...       Ban the calculator from using a list of cores

# Filters here also contain a min and max version (removed for brevity). 
# For example --speed -> --speedMin --speedMax
--damage, --damageStart [FLOAT,FLOAT] Damage range to filter (START)
--damageEnd             [FLOAT,FLOAT] Damage range to filter (END)
--magazine              [FLOAT,FLOAT] Size of magazine to filter
--spreadHip             [FLOAT,FLOAT] Spread range to filter (HIP)
--spreadAim             [FLOAT,FLOAT] Spread range to filter (AIM)
--recoilHip             [FLOAT,FLOAT] Recoil range to filter (HIP)
--recoilAim             [FLOAT,FLOAT] Recoil range to filter (AIM)
--speed                 [FLOAT,FLOAT] Movement speed range to filter
--fireRate              [FLOAT,FLOAT] Fire rate range to filter
--health                [FLOAT,FLOAT] Health range to filter
--pellet                [FLOAT,FLOAT] Pellet range to filter
--timeToAim             [FLOAT,FLOAT] Time to aim range to filter
--reload                [FLOAT,FLOAT] Reload time to filter
--detectionRadius       [FLOAT,FLOAT] Detection radius range to filter
--range, --rangeStart   [FLOAT,FLOAT] Dropoff studs range to filter (START)
--rangeEnd              [FLOAT,FLOAT] Dropoff studs range to filter (END)
--burst                 [FLOAT,FLOAT] Burst range to filter
--TTK                   [FLOAT,FLOAT] TTK range to filter
--DPS                   [FLOAT,FLOAT] DPS range to filter
```

## Compile from source
### Prerequisites
g++ that supports c++20 onwards. (Or really any c++ compiler)

### Libraries
You will have to download 2 dependencies:

1. [json.hpp](https://github.com/nlohmann/json/releases)
2. [CLI11.hpp](https://www.github.com/CLIUtils/CLI11/releases)

And add it to a new directory called `include` in the same directory as `wggcalc-cli.cpp`.

The directory structure should look like this:
```
├── wggcalc-cli.cpp
├── wggcalc.hpp
├── include
│   ├── CLI11.hpp
│   └── json.hpp
├── ...
```

### Run
```sh
# Replace wggcalc with wggcalc.exe if on Windows
g++ -Iinclude -std=c++20 wggcalc-cli.cpp -o wggcalc -O3 -ffast-math -flto -Werror -static
```
No idea how to use cmake lol

## Algorithm Analysis

There are currently **3 methods** inside of the calculator: **Bruteforce**, **Prune**, and **DynamicPrune**.

**TL;DR**

* **DynamicPrune is the recommended default.** it is fast if there are a small subset of guns that are much better than the rest. (Also works well with good filters) 
* **Prune** is extremely fast when your filters are restrictive but has shown to eat up a huge amount of memory. 
* **Bruteforce** is the simplest and sadly the *slowest*

The core difficulty of this problem is that a gun is built from **5 independent parts**
(barrel, magazine, grip, stock, core). If there are `n` total parts, then each group
contains roughly `n/5` parts and the total search space is:

```
(n/5)^5   ≈   O(n^5)
```

All three methods ultimately must respect this — the trick is **how much work they skip**
and **how much memory they use to do it.**

---

### Bruteforce

`O(n^5) Time Complexity` `O(1) Memory Complexity`

The way this method works is by iterating through all possible combinations of
creating a gun. Each gun can have a unique barrel, magazine, grip, stock, and
core. This is the simplest and first method I derived on solving this. It runs a
total of (n/5)^5 times causing this method to have an O(n^5) time complexity
although it doesn't save any extra data thus having a memory complexity of only
O(1).

### Prune

`O(n^5) Time Complexity` `O(n^4) Memory Complexity`

Prune is based on the observation that **most possible gun combinations are
obviously bad or invalid.** So instead of building full guns immediately, it
builds them **incrementally** across 5 “levels”:

1. Core
2. Core + Magazine
3. Core + Magazine + Barrel
4. Core + Magazine + Barrel + Grip
5. Core + Magazine + Barrel + Grip + Stock

At each level, it asks:

> “Is there *any* way for this partial gun to still become valid after adding
the remaining parts?”

To answer that, it precomputes **best/worst-case stat bounds** from the
remaining parts with a "HighLow" datastructure. Example: if the best possible
stock can only increase firerate by ×1.07, and your current build would still
fail the filter even after that boost… then the branch is discarded immediately.

Once it finishes a level it saves the partially completed gun to then be
evaluated by the next level. The space requirement is roughly: `O(n^4 *
sizeof(Gun))` because at the deepest “almost-complete” level, there can still be
many valid candidates outstanding.

Also, Prune assumes that **stats are independent from eachother**, which complicates pellet modifiers and damage falloff. To handle this:

* pellet effects are bounded by adjusting damage, spread and pellet ranges during initialization
* falloff is modeled using a derived `falloffFactor`
* during the last layer these values gets recalculated to accurately apply pellet effects

---

### DynamicPrune  

`O(n^5) Time Complexity` `O(n^2) Memory Complexity`

DynamicPrune is derived from Prune with the same logic: Highlow and level
filtering, but adds a dynamically changing threshold that adapts to the search
space. By taking advantage of the fact that the amount of results we want
is infinitely smaller than the total guns that fit our filters. We can only
continue to compute values that are better than the ones we have already found

DynamicPrune also computes guns on 5 different levels. 

1. Core
2. Core + Magazine
3. Core + Magazine + Barrel
4. Core + Magazine + Barrel + Stock
5. Core + Magazine + Barrel + Stock + Grip 

*You might notice that I switched up the Stock and the Grip order. This is so I
can compute the gun calculation in the same method as ingame allowing me to not
require the use of adjusting damage spread and pellet range. And instead compute
the stats incrementally through each level with greater accuracy*

DynamicPrune uses a DFS style approach instead of Prune's BFS to conserve memory
since the graph is incredibly wide but only 5 layers deep. Thus reducing the
memory requirement from `O(n^4 * sizeof(Gun))`.

It also precomputes core penalty onto the part itself during each core iteration thus giving us a significant speedup while increasing memory to `O(n^2)` (technically O(n * t) where t is the number of threads).

DynamicPrune uses an atomic variable `currentBestThreshold_a` to keep track of "The worst
stat across all our priority queues." Since all the guns must be stored in
a priority queue and these are seperate across multiple threads, we need to keep
track of the stat of the gun ready to be popped from the queue (when the top of
the queue contains the worst gun). Using this we can simply check if the guns
that are waiting to be computed could beat this threshold. If not, they are
pruned immediately.

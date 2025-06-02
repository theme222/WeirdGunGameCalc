# WeirdGunGameCalc
> A tool to bruteforce all combinations of guns in the roblox game "Weird Gun Game"

## About
`Updated to Detection Radius Update`<br/>
`Original data from @zyadak`<br/>

Made using c++ this tool reads data from Data/FullData.json and calculates all possible combinations.
If you find any gun have stats that are different from the game please notify me on discord @theme222 or open an issue here on github (Except for any parts that contain pellets. I still have no idea what dark magic formula is used to calculate that).

## Data
The data it reads is the output of `FileFormatter.py` that formats the .txt files in RawData scraped using a `DataScreenshotter.py` provided by @zyadak.

## Compile from source
### Prerequisites
g++ that supports c++20 onwards.

### Libraries
You will have to download 2 dependencies:

1. [json.hpp](https://github.com/nlohmann/json/releases)
2. [CLI11.hpp](https://www.github.com/CLIUtils/CLI11/releases)

And add it to a new directory called `include` in the same directory as `Calculator.cpp`.

### Run
```sh
g++ -std=c++20 -Iinclude Calculator.cpp -o Calculator -Werror
```
No idea how to use cmake lol

## Usage
Currently only supported on linux (If enough people complain I'll try
to compile it to windows). Run by providing options as filters. Look in
`Commands.sh` for example commands to run (Those are the ones I use for the
`InterestingBuilds` directory.)  You can look at the full list of possible flags
by running `./Calculator --help`.

**TL;DR** Prune uses a lot of memory (2 GB+) for fast runtime and doesn't support shotguns. Use Bruteforce by adding `--method BRUTEFORCE` to the command if any of this is a concern.

## Algorithm Analysis

There are currently 2 methods inside of the calculator: Bruteforce and Prune.

### Bruteforce

`O(n^5) Time Complexity`
`O(n) Memory Complexity`

The way this method works is by iterating through all possible combinations of
creating a gun. Each gun can have a unique barrel, magazine, grip, stock, and
core. This is the simplest and first method I derived on solving this. It runs a
total of (n/5)^5 times causing this method to have an O(n^5) time complexity
although it doesn't save any extra data thus having a memory complexity of only
O(n).

### Prune (BETA)
`O(n^5) Time Complexity`
`O(n^4) Memory Complexity`

Unlike what the Time and memory complexity suggests, this method is observed to
deliver much faster runtimes based on how inclusive (or exclusive) the provided
filters are. It contains a much more complex algorithm that boils down to 3 steps:
1. It loops through each "level" of creating a gun: (core) -> (core + magazine) -> (core + magazine + barrel) -> (core + magazine + barrel + grip) -> (core + magazine + barrel + grip + stock)
2. For each level it determines whether or not it is a *valid* combination. This is found by checking each property whether it is within range of the filters provided and whether or not the values can be corrected using the rest of the parts to be within range. The values come from checking the highest and lowest possible combination of parts for each stat. For example: The highest increase for firerate using only stocks is 1.07 while the lowest is 0.75. This check is repeated for all stats for every core type (to account for core incompatibility) and for every level. As an example level 1 (core) would use the best part combination of mag + barrel + grip + stock but for level 3 (core + magazine + barrel), it would instead use the best part combo of grip + stock.
3. Once it determines whether or not a part is a valid combination, it saves it into a std::vector to be used as a gun basis on the next level.

This method contains a much higher runtime constant than bruteforce, requiring
both of allocating memory, initializing the best part combo, and copying and
writing data onto the std::vector. It also has a much higher runtime formula
being (a \* n/5) + (a \* n/5)^2 + (a \* n/5)^3 + (a \* n/5)^4 + (a \* n/5)^5
which makes it worse than bruteforce if the a is very close to 1 (if the filters
are not restrictive enough). It also requires a lot of memory due to the fact
that we need to save valid combinations from previous levels. The exact amount
is (n)^4 \* 2 \* sizeof(Gun).

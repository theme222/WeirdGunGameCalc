# WeirdGunGameCalc
> A tool to bruteforce all combinations of guns in the roblox game "Weird Gun Game"

## About
`Updated to Killstreak Update`<br/>
`Original data from @zyadak`<br/>

Made using c++ this tool reads data from Data/FullData.json and calculates all possible combinations.
If you find any gun have stats that are different from the game please notify me on discord @theme222 or open an issue here on github (Except for any parts that contain pellets. I still have no idea what dark magic formula is used to calculate that).

## Data
The data it reads is the output of `FileFormatter.py` that formats the .txt files in RawData scraped using a `DataScreenshotter.py` provided by @zyadak.

## Compile from source
### Prerequisits
g++ that supports c++20 onwards.

### Libraries
You will have to download the [json.hpp]('https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp') and [CLI11.hpp]('https://github.com/CLIUtils/CLI11/releases') file from the github and add it to a directory called "include".

### Run
```sh
g++ -std=c++20 -Iinclude Calculator.cpp -o Calculator -Werror
```
No idea how to use cmake

## Usage
Currently only supported on linux (If enough people complain I'll try to compile it to windows). Run by providing options as filters, please use `./Calculator --help` for more info

## Runtime analysis
Current method of bruteforcing all possible combinations of WGG requires
considering all possible combinations of cores, barrels, magazines, grips, and
stocks (We are ignoring scopes for simplicity and the fact that there currently
exists only one type of scope that has stats and it is completely useless).
Assuming the amount of parts is n, and on average there are around equal amount
of parts per category (There is a bias towards barrels), and the fact that each
gun must take in consideration of all the bonuses and debuffs of each part by
running float calculations on every stat which lets assume is a constant value
c, the total runtime of this is (c(n/5))^5 which comes to O(n^5). I am trying to
find a way to either cache repeated calculations or implement a more efficient
algorithm.

**O(n^5)** Could be improved but I have no idea how <br/>
**n** Definitely can be improved and there are plans to reduce this <br/>
**c** On the brink of being fully optimized anything else that reduces the constant is negligible <br/>

### Current list of optimizations
- Parallel processing using std::thread
- String compression by translating into an integer
- Preloading data into objects and placing them into arrays
- Iterator doesn't go over parts that have the same name as the core (Because all of its stats get removed)
- Only calculating required stats that are being compared using integer flags (uint32) (Only calculating the full gun when finished)
- Sorting guns using std::priority_queue and popping any guns exceeding the number of top guns to be displayed
- Time to aim and magazine size filters get filtered before calculations begin
- Penalty calculation uses the values from string compression by indexing a 2 dimensional array
- Uses sentinels to check if user has provided a specific filter of that type or not
- All parts are reused and only references to objects get copied over to the main gun (I dare you to find a pass by value more than 8 bytes in size)

If you have any suggestions or improvements, feel free to open an issue or submit a pull request on the GitHub repository.

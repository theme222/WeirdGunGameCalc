# WeirdGunGameCalc
> A tool to bruteforce all combinations of guns in the roblox game "Weird Gun Game"

## About
`Updated to Major Update`<br/>
`Using nholmann/json to read json files`<br/>
`Original data from @zyadak https://discord.com/channels/1326315584417435648/1326315585046450237/1363241874701291722`<br/>

Made using c++ this tool reads data from Data/FullData.json and calculates all possible combinations.
If you find any gun have stats that are different from the game please notify me on discord @theme222 (Except for any parts that contain pellets. I still have no idea what dark magic formula is used to calculate that).

## Data
The data it reads is the output of `FileFormatter.py` that formats the .txt files in RawData scraped using a `DataScreenshotter.py` provided by @zyadak.

## Runtime
It takes around 30 seconds to bruteforce all combinations using a load of optimizations and parallel processing.

## Compile from source
### Prerequisits
You will have to download the [json.hpp]('https://github.com/nlohmann/json/blob/develop/single_include/nlohmann/json.hpp') and [CLI11.hpp]('https://github.com/CLIUtils/CLI11/releases') file from the github and add it to a directory called "include".

### Run
```sh
g++ -std=c++20 -Iinclude Calculator.cpp -o Calculator -Werror
```
No idea how to use cmake

## Usage
Currently only supported on linux (If enough people complain I'll try to compile it to windows). Run by providing options as filters, please use `./Calculator --help` for more info

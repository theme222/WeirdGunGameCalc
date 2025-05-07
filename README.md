# WeirdGunGameCalc
> A tool to bruteforce all combinations of guns in the roblox game "Weird Gun Game"

## About
`Updated to The Shotguns update`<br/>
`Using nholmann/json to read json files`<br/>
`Original data from @zyadak https://discord.com/channels/1326315584417435648/1326315585046450237/1363241874701291722`<br/>

Made using c++ this tool reads data from Data/FullData.json and calculates all possible combinations.

## Data
The data it reads is the output of `FileFormatter.py` that formats the .txt files in RawData scraped using a `DataScreenshotter.py` provided by @zyadak.

## Runtime
It takes around 30 seconds to bruteforce all combinations using a load of optimizations and parallel processing.

## Compilation
I have no idea how to use cmake so...
```sh
g++ -std=c++20 -Iinclude Calculator.cpp -o Calculator -Werror
```
You will have to download the json.hpp and CLI11.hpp file from the github and add it to a directory called "include".

## Usage
Currently only supported in linux (If enough people complain I'll try to compile it to windows). Run by providing options as filters, please use `./Calculator --help` for more info

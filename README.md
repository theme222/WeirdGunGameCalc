# WeirdGunGameCalc
> A tool to bruteforce all combinations of guns in the roblox game "Weird Gun Game" 

## About
`Updated to https://discord.com/channels/1326315584417435648/1326316188145291365/1364502735755673701`<br/>
`Using nholmann/json to read json files`<br/>
`Original data from @zyadak https://discord.com/channels/1326315584417435648/1326315585046450237/1363241874701291722`<br/>

Made using c++ this tool reads data from Data/FullData.json and tries out all possible combinations. 
Currently to use it you have to modify the source code yourself 💀, but this may change in the future.

## Data 
The data it reads is the output of `FileFormatter.py` that formats the .txt files in RawData provided by @zyadak.

## Runtime
It takes around 4-6 minutes to bruteforce all combinations currently but there are plans to optimize this further.

## Interesting Builds
I will try to keep this directory updated with any builds that the program spits out as interesting. 

## Compilation
I have no idea how to use cmake so...
```sh
g++ -Iinclude Caculator.cpp -o Calculator
./Calculator > Results.txt
```

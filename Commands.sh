# This are commands for my setup specifically. You can config these example commands and run them in any order you want.

# You don't need to recompile obviously.
g++ -Iinclude -std=c++20 Calculator.cpp -o Calculator &&
# This doesn't need to be re ran every time because it saves it's results in Data already.
python FileFormatter.py &&

# You can look at the example results in the corresponding output paths.
./Calculator -o InterestingBuilds/FastestTTKShortRange.txt -i SMG AR LMG --magazineMin 30 --speedMin 0 --damageMax 99.99 --spreadAimMax 1 --recoilAimMax 35 --rangeMin 100 --pelletMax 1 --bc "FE-200SH" &&
./Calculator -o InterestingBuilds/FastestTTKMediumRange.txt -i SMG AR LMG --magazineMin 30 --speedMin 0 --damageMax 99.99 --spreadAimMax 1 --recoilAimMax 35 --rangeMin 200 --pelletMax 1 --bc "FE-200SH" &&
./Calculator -o InterestingBuilds/MagDumpTurret.txt -i SMG AR LMG --magazineMin 75 --speedMin 0 --damageMax 99.99 --spreadAimMax 1.5 --recoilAimMax 50 --rangeMin 100 --pelletMax 1 --healthMin 10 --bc "FE-200SH" &&
./Calculator -o InterestingBuilds/Fastest4TapNonSMG.txt -i AR LMG --damageMin 25 --speedMin 0 --magazineMin 30 --spreadAimMax 0.65 --recoilAimMax 30 --rangeMin 150 --pelletMax 1 --bc "FE-200SH" &&
./Calculator -o InterestingBuilds/Fastest3TapGun.txt -i SMG AR LMG --magazineMin 30 --speedMin 0 --damage 33.4 99.99 --spreadAimMax 0.65 --recoilAimMax 40 --pelletMax 1 --rangeMin 100 &&
./Calculator -o InterestingBuilds/FastestHeadshotSniper.txt -i Sniper BR --speedMin 0 --damageMin 50 --spreadAimMax 0.15 --sort FIRERATE --rangeMin 350 --pelletMax 1 --magazineMin 5 &&
./Calculator -o InterestingBuilds/Fastest1TapSniper.txt -i Sniper BR --damageMin 100 --spreadAimMax 0.149 --sort FIRERATE --rangeMin 350 --pelletMax 1 --speedMin -5  --magazineMin 5 &&
./Calculator -o InterestingBuilds/FastestTTKBR.txt -i BR --speedMin -10 --spreadAimMax 0.6 --recoilAimMax 30 --magazineMin 20 --rangeMin 150 --pelletMax 1

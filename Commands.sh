#!/bin/bash
set -e # Stops on error

# This are commands for my setup specifically. You can config these example commands and run them in any order you want.

# You don't need to recompile obviously.
g++ -Iinclude -std=c++20 wggcalc-cli.cpp -o wggcalc -O3 -ffast-math -flto -Werror -static

# This doesn't need to be re ran every time because it saves it's results in Data already.
python ParseSheet.py

IB="InterestingBuilds"
BANSEMI=(--bc "FC200SH" "M1 Carbine" "G11" "Loaf" "ZIPO-2222") # This is a shell expansion. What it does is it basically substitutes the value in the parenthesis into the command line.

# You can look at the example results in the corresponding output paths.

####################### AUTOMATICS #######################
./wggcalc -o $IB/FastestTTKShortRange.txt -i SMG AR LMG Weird --magazineMin 30 --speedMin 0 --damageMax 99.99 --spreadAimMax 1 --recoilAimMax 35 --rangeMin 100 --pelletMax 1 "${BANSEMI[@]}"
./wggcalc -o $IB/FastestTTKMediumRange.txt -i SMG AR LMG Weird --magazineMin 30 --speedMin 0 --damageMax 99.99 --spreadAimMax 0.6 --recoilAimMax 30 --rangeMin 200 --pelletMax 1 "${BANSEMI[@]}"
./wggcalc -o $IB/Fastest3TapGun.txt -i SMG AR LMG --magazineMin 30 --speedMin 0 --damage 33.4 99.99 --spreadAimMax 0.65 --recoilAimMax 40 --pelletMax 1 --rangeMin 100 "${BANSEMI[@]}"
./wggcalc -o $IB/FastestTTKAllowPellet.txt -i SMG AR LMG --speedMin 0 --magazineMin 30 --spreadAimMax 0.7 --rangeMin 150 --recoilAimMax 35 "${BANSEMI[@]}"
./wggcalc -o $IB/LowRecoilBeam.txt -i SMG AR LMG Weird --recoilAimMax 7 --magazineMin 45 --spreadAimMax 0.4 --rangeMin 150 --speedMin 0 "${BANSEMI[@]}"
./wggcalc -o $IB/SnipersWorstNightmare.txt -i SMG AR LMG Weird --recoilAimMax 7 --magazineMin 75 --spreadAimMax 0.4 --rangeMin 175 "${BANSEMI[@]}" -s HEALTH --TTKMax 0.25
./wggcalc -o $IB/F2PNewbieFriendlyTTK.txt -i SMG AR LMG --banPriceType ROBUX LIMITED SPECIAL --magazineMin 30 --speedMin 0 --damageMax 99.99 --spreadAimMax 0.8 --recoilAimMax 35 --rangeMin 150 "${BANSEMI[@]}" 
# ./wggcalc -o $IB/MagDumpTurret.txt -i SMG AR LMG --magazineMin 75 --speedMin 0 --damageMax 99.99 --spreadAimMax 1.5 --recoilAimMax 50 --rangeMin 100 --pelletMax 1 "${BANSEMI[@]}"
# ./wggcalc -o $IB/Fastest4TapNonSMG.txt -i AR LMG --damageMin 25 --speedMin 0 --magazineMin 30 --spreadAimMax 0.65 --recoilAimMax 30 --rangeMin 150 --pelletMax 1 "${BANSEMI[@]}"
# ./wggcalc -o $IB/ZombieDeleterAuto.txt -i SMG AR LMG Weird -s DPS --spreadAimMax 0.7 --rangeMin 130 --magazineMin 40 --reloadMax 2 --recoilAimMax 35 --speedMin 0 "${BANSEMI[@]}"
# ./wggcalc -o $IB/ZombieDeleterSemi.txt -i Shotgun -s DPS --spreadAimMax 2 --magazineMin 20 --speedMin 0 --reloadMax 4 --recoilAimMax 25 --rangeMin 40
####################### AUTOMATICS #######################

####################### SNIPERS #######################
./wggcalc -o $IB/FastestHeadshotSniper.txt -i Sniper --speedMin 0 --damageMin 50 --spreadAimMax 0.15 --sort FIRERATE --rangeMin 200 --pelletMax 1 --magazineMin 10
./wggcalc -o $IB/Fastest1TapSniper.txt -i Sniper BR --damageMin 100 --spreadAimMax 0.149 --sort FIRERATE --rangeMin 300 --pelletMax 1 --speedMin -5  --magazineMin 5
./wggcalc -o $IB/FastestGuaranteed1Tap.txt -i Sniper BR --damageEndMin 100 --spreadAimMax 0.15 --sort FIRERATE --speedMin -5 --magazineMin 5
####################### SNIPERS #######################

####################### SEMI AUTOS #######################
./wggcalc -o $IB/FastestHeadshotBR.txt -i BR --speedMin 0 --damageMin 66.67 --spreadAimMax 0.15 --sort FIRERATE --rangeMin 200 --pelletMax 1 --magazineMin 10
./wggcalc -o $IB/FastestTTKBR.txt -i BR --speedMin -10 --spreadAimMax 0.6 --recoilAimMax 7.5 --magazineMin 20 --rangeMin 150 --pelletMax 1
# ./wggcalc -o $IB/M1Carbine.txt --fc "M1 Carbine" --spreadHipMax 0.6 --magazineMin 20 --rangeMin 100 --fireRateMin 900 --recoilAimMax 6
####################### SEMI AUTOS #######################

####################### SHOTGUNS #######################
./wggcalc -o $IB/LowSpreadShogun.txt -i Shotgun --sort FIRERATE --damageMin 100 --speedMin 0 --spreadAimMax 0.85
./wggcalc -o $IB/HipfireShotgun.txt -i Shotgun --sort RECOILHIP --damageMin 100 --speedMin 0 --spreadHipMax 2 --fireRateMin 50
./wggcalc -o $IB/Shotgun.txt -i Shotgun --sort FIRERATE --damageMin 140 --speedMin 0 --magazineMin 10 --spreadAimMax 1.8 --rangeMin 80
./wggcalc -o $IB/AutoShotgun.txt -i Shotgun --sort FIRERATE --fc Siberia-12 AUTO-12 --damageMin 60 --spreadAimMax 2 --speedMin 0 --rangeMin 50 --magazineMin 20 --recoilAimMax 100
####################### SHOTGUNS #######################

####################### SECONDARIES #######################
./wggcalc -o $IB/HipfireSecondaryAuto.txt -i Sidearm --fc "B17" --spreadHipMax 1 --magazineMin 30 --speedMin 0 --rangeMin 20
./wggcalc -o $IB/HipfireSecondarySemi.txt -i Sidearm --bc "B17" "Deagle" --spreadHipMax 1 --magazineMin 20 --speedMin 5 --rangeMin 40
####################### SECONDARIES #######################

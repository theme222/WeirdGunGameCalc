python FileFormatter.py &&
./Calculator -o InterestingBuilds/FastestTTKSMG.txt -i SMG AR LMG --magazineMin 30 --speedMin 0 --damageMax 99.99 --spreadAimMax 1.049 --recoilAimMax 35;
./Calculator -o InterestingBuilds/FastestTTKNonSMG.txt -i AR LMG --magazineMin 30 --speedMin 0 --damageMax 99.99 --spreadAimMax 0.649 --recoilAimMax 30;
./Calculator -o InterestingBuilds/FastestHeadshotSniper.txt -i Sniper --speedMin 0 --damageMin 49.95 --spreadAimMax 0.149 --sort FIRERATE;
./Calculator -o InterestingBuilds/Fastest1TapSniper.txt -i Sniper --damageMin 99.95 --spreadAimMax 0.149 --sort FIRERATE;
./Calculator -o InterestingBuilds/Fastest4TapNonSMG.txt -i AR LMG --damageMin 24.95 --speedMin 0 --magazineMin 30 --spreadAimMax 0.65;
./Calculator -o InterestingBuilds/LowSpread1TapShotgun.txt -i Shotgun --damageMin 100 --sort FIRERATE --speedMin -10 --spreadAimMax 0.5;
./Calculator -o InterestingBuilds/Fastest3TapGun.txt -i SMG AR LMG --magazineMin 30 --speedMin 0 --damage 33.4 99.99 --spreadAimMax 0.65 --recoilAimMax 40;

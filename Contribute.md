# How to Contribute to WeirdGunGameCalc

The simplest and easiest way to contribute to the repository is to help notify and update the dataset that contains all the parts that are available inside of the game.

Before we do anything let's get to know the different files in the repository and what they do for us

## Files & Directories

* `RawData`   This directory contains our data from the game. It is an unformatted version that is easily human readable and editable. This directory contains 5 files:
<br/>
  1. `Barrels.txt` Contains all the barrels inside of the game.
  2. `Grips.txt` Contains all the grips inside of the game
  3. `Cores.txt` Contains all the cores inside of the game.
  4. `Magazines.txt` Contains all the magazines inside of the game.
  5. `Stocks.txt` Contains all the stocks inside of the game.

There are currently no plans on adding any sights into the calculator due to the fact that they are mostly cosmetic (Except for a couple problems with range increase, spread pellet increase and that one shield sight thing).

* `Data` This directory contains one singular file being `FullData.json`. this file is the output when I ran the `FileFormatter.py` program to format the files inside of the `RawData`. This file is what the calculator will read from when extracting it's data.
* `OldData` Unrelated. Simply contains old part values from previous updates. It is currently unused.
* `Calculator.cpp` The main calculator. This is the file that contains all the c++ code that compiles to the binaries/executables.
* `DataScreenshotter.py` This file helps with data collection which will be explained more [here](#data-collection)
* `FileFormatter.py` This file turns turns the data from `RawData` and outputs a proper json file to `Data`. (The file is not system agnostic, it only runs on linux currently).
* `Epic prompt engineering.txt` this file will also help us in [data collection](#data-collection) as well.

## Data Collection

This section will contain the data collection method that I personally use incase the game updates and releases many new guns (If you want to notify about a small balancing change , opening up an issue is much easier). This isn't required but makes the process easier if you know how to use the tools. You can also do this manually by just simply writing the values into a text file and format it based on how it looks inside of the raw data files.

1. **Screenshot**: Using `DataScreenshotter.py` will help with taking screenshots of the gun parts by automatically screenshotting the specific area that is of use. Run the file by running `python DataScreenshotter.py`. This file has two modes: (Re)Selection and Screenshot. Select the area to screenshot by pressing `ctrl-alt-r` and clicking the top left position of the part information (Specifically the info from the shop itself to not have any core incompatibillity issues) and `ctrl-alt-s` to take a screenshot of that part. All screenshots gets saved in a folder called `Screenshots`.
2. **AI Prompting**: Once we have the screenshots we can use AI, specifically [Gemini 2.0 Flash](https://aistudio.google.com) And pasting the contents of `Epic prompt engineering.txt` into the AI. After that place a screenshot containing the M1919a6 Browning as a baseline and then place in the rest of the images to extract the text.

## Uploading

(This step is mainly for people that don't have much experience with github. Obviously the best method would be to fork the repo and open a pull request with the new parts.)
After extracting the text in a properly formatted way, we can now modify the data from `RawData`. Simply download the corresponding file, copy and paste the part in the proper category and then open up an issue here on github for me to replace into the dataset.

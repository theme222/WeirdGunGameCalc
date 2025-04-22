"""
I Wrote this in python as it wasn't intended to be run multiple times and it shouldn't be doing any of the heavy lifting unlike the cpp program
"""
import os
import json

filenames = ["Barrels.txt", "Grips.txt", "Guns.txt", "Magazines.txt", "Stocks.txt", "Weird.txt"]


# Step 0
def Setup():
    print("Running Step 0")
    if os.path.exists("Data"): os.system("rm -r Data")
    os.system(f"cp -r RawData Data")


# Step 1
def CleanData():
    print("Running Step 1")
    for name in filenames:
        newContent = []

        with open(f"Data/{name}", 'r') as file:
            content = file.read().split('\n')

            for line in content:
                # Remove data in parenthesis
                index = line.find("(")
                if index != -1: line = line[:index]

                # Strip any extra spaces
                line = line.strip()

                # Remove the line that contains the word Effects
                if line.find("Effects") != -1: continue

                # Replace the word Gun in Barrels.txt with Barrels
                if name == "Barrels.txt": line = line.replace("Gun", "Barrel")

                newContent.append(line)

        with open(f"Data/{name}", 'w') as file:
            file.write('\n'.join(newContent))


# Step 2
def TurnToJSON():
    print("Running Step 2")

    categories = ["Assault Rifle", "Sniper", "SMG", "LMG"]
    types = ["Barrels", "Cores", "Grips", "Magazines", "Stocks"]

    finalJSON = {"Barrels": [], "Cores": [], "Grips": [], "Magazines": [], "Stocks": []}

    """
    {
        cores: [...],
        barrels: [...],
        grips: [...],
        magazines: [...],
        stocks: [...],
    }


    [
    category: AR,
    name: blah,
    damageMax: 69,
    damageMin: 42,
    ...
    ]
    """

    for name in filenames:
        with open(f"Data/{name}", "r") as file:
            content = file.read().split('\n')
            currentCategory = None
            currentType = None
            currentPart = {}

            for line in content:
                if line == "": continue
                if line == "---":
                    currentPart = {}
                    continue

                assert len(line.split(": ")) == 2, line

                key, value = line.split(': ')

                # Extracting the category and type
                if key == "Category":
                    if name == "Guns.txt":
                        currentCategory = value
                        currentType = "Cores"
                    else:
                        currentCategory = " ".join(value.strip().split(" ")[:-1])
                        currentType = value.strip().split(" ")[-1]
                        if currentType == "Guns": currentType = "Cores"
                    continue

                assert currentCategory and currentType, "You fucked up bitch"

                if key + "s" == currentType or "Gun" == key: key = "Name"
                currentPart[key] = value

                finalJSON[currentType].append(currentPart)

    with open("Data/FullData.json", 'w') as file:
        json.dump(finalJSON, file, indent=2)


def CleanUp():
    print("Running step 3")
    os.system("rm Data/*.txt")


def main():
    Setup()
    CleanData()
    TurnToJSON()
    CleanUp()


if __name__ == '__main__':
    main()

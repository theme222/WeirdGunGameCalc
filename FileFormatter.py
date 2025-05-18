"""
I Wrote this in python as it wasn't intended to be run multiple times and it shouldn't be doing any of the heavy lifting unlike the cpp program
"""
import os
import json

filenames = ["Barrels.txt", "Grips.txt", "Guns.txt", "Magazines.txt", "Stocks.txt", "Weird.txt"]

known_categories = {"Assault Rifle", "Sniper", "SMG", "LMG", "Shotgun", "Weird"}
known_types = {"Barrels", "Cores", "Grips", "Magazines", "Stocks"}
known_properties = {'Reload_Speed', 'Recoil_Hip_Vertical', 'Recoil', 'Magazine_Size', 'Fire_Rate', 'ADS_Spread', 'Health', 'Damage', 'Suppression', 'Dropoff_Studs', 'Category', 'Recoil_Aim_Vertical', 'Recoil_Hip_Horizontal', 'Spread', 'Time_To_Aim', 'Hipfire_Spread', 'Recoil_Aim_Horizontal', 'Movement_Speed_Modifier', 'Name', 'Movement_Speed', 'Reload_Time', 'Pellets'}
known_names = {
    'MAC-10', 'M1919a6 Browning', 'Type 99', 'Scar H', 'MP9', 'Honk', 'Thompson', 'Plunger', 'M1891 Carcano',
    'Skorpion vz. 61', 'M1917 Browning', 'Improvised', 'Scar L', 'Lee Enfield', 'Arctic Warfare',
    'Horizontal', 'Willy Fischy', 'Anvil', 'Shower', 'Stat Randomizer', 'Famas F1', 'Speed Coil',
    'Water Pipe', 'Ketchup', 'M1919 Browning', 'UMP-40', 'MP 40', 'Type 2a Nambu', 'Banana', 'Shovel',
    'STG-44', 'Oil', 'Fedorov Avtomat', 'Tap', 'Rice Pot', 'Scar Drum', 'M-16', 'Hecate II', 'Remington 700',
    'M91 Moschetto', 'DP-27', 'M60', 'Quack', 'AUG', 'Restroom', 'AKM', 'XXL AKM', 'Snake', 'As-Val', 'MG42',
    'Mustard', 'Hell-Trooper-23', 'Kriss Vector', 'Words as Weapons', 'The Chief', 'Mas-38', 'Bicycle',
    'PPSh-41', 'Spas-12', 'Remington 870 Wingmaster', 'M1887', 'M1897 Trench Shotgun', 'Remington 870 Breacher',
    'Remington 870 MCS', 'Remington 870', 'Spas-12 Folded', 'Hex Spitter', 'PP-19 Bizon', 'M1918 BAR', 'G36C',
    'APS', 'MP5', 'Circuit Judge', 'MP5A3'
}


# Step 0
def Setup():
    print("Running Setup")
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


    finalJSON = {"Barrels": [], "Cores": [], "Grips": [], "Magazines": [], "Stocks": []}

    for name in filenames:

        currentCategory = None
        currentType = None
        currentPart = {}

        with open(f"Data/{name}", "r") as file:
            content = file.read().split('\n')

        for line in content:
            if line == "": continue
            if line == "---":
                if currentPart: finalJSON[currentType].append(currentPart)
                currentPart = {}
                continue

            assert len(line.split(": ")) == 2, f"This line Doesn't match the logic {line} on file {name}"

            key, value = line.split(': ')

            # Extracting the category and type
            if key == "Category":
                if name == "Guns.txt":
                    currentCategory = value[:-1]
                    currentType = "Cores"
                else:
                    currentCategory = " ".join(value.strip().split(" ")[:-1])
                    currentType = value.strip().split(" ")[-1]
                    if currentType == "Guns": currentType = "Cores"
                continue

            assert currentCategory and currentType, "You fucked up bitch"

            if key + "s" == currentType or "Gun" == key: key = "Name"
            currentPart["Category"] = currentCategory
            currentPart[key] = value

    with open("Data/FullData.json", 'w') as file:
        json.dump(finalJSON, file, indent=2)


def ValueFormat():
    print("Running step 3")
    with open(f"Data/FullData.json", 'r') as file:
        JSONData = json.load(file)

    newJSONData = {"Barrels": [], "Cores": [], "Grips": [], "Magazines": [], "Stocks": []}
    for partType, partList in JSONData.items():
        for part in partList:
            newPart = {}
            for key, value in part.items():

                # Remove all % from values
                value = value.replace("%", "")

                # Remove the "s" from Reload_Time
                if key == "Reload_Time": value = value.replace("s", "")

                # Convert to float (if possible)
                try:
                    value = float(value)  # float("+1.2") -> 1.2 😀
                except ValueError:
                    pass

                # Format the damage range and recoil in cores
                if partType == "Cores":
                    # String check is required due to weird guns not having a damage dropoff
                    if (key == "Damage" or key == "Dropoff_Studs") and isinstance(value, str):
                        value = [float(v) for v in value.split(" > ")]
                    elif "Recoil" in key:
                        value = [float(v) for v in value.split(" - ")]

                # If it can be turned into an int do it
                if isinstance(value, float):
                    if value % 1 == 0: value = int(value)
                elif isinstance(value, list):
                    assert len(value) == 2, "UHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH???????????"
                    if value[0] % 1 == 0: value[0] = int(value[0])
                    if value[1] % 1 == 0: value[1] = int(value[1])

                newPart[key] = value

            newJSONData[partType].append(newPart)

    with open("Data/FullData.json", 'w') as file:
        json.dump(newJSONData, file, indent=2)


# Step 4
def ValidateData():
    print("Running step 4")

    with open("Data/FullData.json", 'r') as file:
        JSONData = json.load(file)

    for t in JSONData.keys():
        if t not in known_types:
            print(f"Type {t} not found")

    for partList in JSONData.values():
        for part in partList:
            for partProperty in part.keys():
                if partProperty not in known_properties:
                    print(f"Property {partProperty} not found")

            if part["Name"] not in known_names:
                print(f"Name {part['Name']} not found")

            if part["Category"] not in known_categories:
                print(f"Category {part['Category']} not found")


def CleanUp():
    print("Running Clean up")
    os.system("rm Data/*.txt")


def main():
    Setup()
    CleanData()
    TurnToJSON()
    ValueFormat()
    ValidateData()
    CleanUp()


if __name__ == '__main__':
    main()

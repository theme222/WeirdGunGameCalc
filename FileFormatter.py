"""
I Wrote this in python as it wasn't intended to be run multiple times and it shouldn't be doing any of the heavy lifting unlike the cpp program
"""
import os
import json

filenames = ["Barrels.txt", "Grips.txt", "Cores.txt", "Magazines.txt", "Stocks.txt"]

known_categories = {"Assault Rifle", "Sniper", "SMG", "LMG", "Shotgun", "Weird", "BR"}
known_types = {"Barrels", "Cores", "Grips", "Magazines", "Stocks"}

known_properties = {'Reload_Speed', 'Recoil_Hip_Vertical', 'Recoil',
    'Magazine_Size', 'Fire_Rate', 'ADS_Spread', 'Health', 'Damage', 'Suppression',
    'Dropoff_Studs', 'Category', 'Recoil_Aim_Vertical', 'Recoil_Hip_Horizontal',
    'Spread', 'Time_To_Aim', 'Hipfire_Spread', 'Recoil_Aim_Horizontal',
    'Movement_Speed_Modifier', 'Name', 'Movement_Speed', 'Reload_Time', 'Pellets',
    'Detection_Radius', 'Range', 'Burst', 'Falloff_Factor'
}

known_names = {
    'MAC-10', 'M1919 .30 Cal', 'Type 99', 'Scar H', 'NP9', 'Honk', 'Tommy', 'Plunger', 'M1891',
    'Skorpion vz. 61', 'M1917 WC', 'Improvised', 'Scar L', 'Lee Enfield', 'AWM',
    'Horizontal', 'Willy Fischy', 'Anvil', 'Shower', 'Stat Randomizer', 'Famas F1', 'Speed Coil',
    'Water Pipe', 'Ketchup', 'M1919', 'UNP-40', 'MP 40', 'Type 2a Nambu', 'Banana', 'Shovel',
    'STG-44', 'Oil', 'Fedorov', 'Tap', 'Rice Pot', 'Scar Drum', 'M-16', 'Hecate II', 'M40',
    'M91 Moschetto', 'DP-27', 'M60', 'Quack', 'AUG', 'Restroom', 'AKM', 'XXL AKM', 'Snake', 'As-Val', 'MG42',
    'Mustard', 'Hell-Trooper-23', 'Kriss Vector', 'Words as Weapons', 'The Chief', 'Mas-38', 'Bicycle',
    'PPSh-41', 'Spas-12', 'M870 Wingmaster', 'M1887', 'M1897 Trench Shotgun', 'M870 Breacher',
    'M870 MCS', 'M870', 'Spas-12 Folded', 'Hex Spitter', 'PP-19 Bizon', 'BAR', 'C36C',
    'APS', 'NP5', 'Circuit Judge', 'NP5A3', 'Negev', 'M2000', 'Pistol Pistol', 'Auto-5', 'Croissant',
    'Loaf', 'Taco', 'Eclair', 'Pastel', 'P90', 'Chauchat', 'Dragunov', 'M1Garand', 'FAL', 'FC200SH', 'Light Gun',
    'Gaming Rig', 'Controller', 'Graphics Card', 'Keyboard', 'Musket', 'Snake Sniper', 'M110', 'SKS', 'Blunderbuss',
    'AR-8X', 'Freedom Dispenser', 'Second Amendment', 'JSG12', 'AWM', 'Arisaka', 'Sten gun', 'M1 Carbine',
    'M1919 Tripod', 'FG-42', 'G11', 'AK-12', 'QBZ-95', 'Lightning Shot', '10KG Dumbbell', 'VSSR', 'BarrelSight',
    'Screwdriver', 'Cirrus - Hand', 'Napo - Scissors', 'Rat - Emoji', 'Derpy - Staff', 'Uzi', 'G3',
    'Dominus', 'Lipstick', 'Lewis', 'Mega Soaker'
}

current_categories = {
    "Assault Rifle": 0,
    "Sniper": 1,
    "SMG": 2,
    "LMG": 3,
    "Weird": 4,
    "Shotgun": 5,
    "BR": 6
}

current_penalties = [
    [1.00, 0.70, 0.80, 0.75, 1.00, 0.75, 0.80],
    [0.70, 1.00, 0.60, 0.80, 1.00, 0.60, 0.85],
    [0.80, 0.60, 1.00, 0.65, 1.00, 0.65, 0.70],
    [0.75, 0.80, 0.65, 1.00, 1.00, 0.75, 0.85],
    [1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00],
    [0.70, 0.50, 0.65, 0.75, 1.00, 1.00, 0.60],
    [0.80, 0.85, 0.70, 0.85, 1.00, 0.60, 1.00]
]


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
                if name == "Cores.txt":
                    currentCategory = value
                    currentType = "Cores"
                else:
                    currentCategory = " ".join(value.strip().split(" ")[:-1])
                    currentType = value.strip().split(" ")[-1]
                continue

            assert currentCategory and currentType, "You fucked up bitch"

            if key + "s" == currentType: key = "Name"
            currentPart["Category"] = currentCategory
            currentPart[key] = value

    with open("Data/FullData.json", 'w') as file:
        json.dump(finalJSON, file, indent=2)


# Step 3
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

    totalError = 0

    with open("Data/FullData.json", 'r') as file:
        JSONData = json.load(file)

    for t in JSONData.keys():
        if t not in known_types:
            print(f"Type {t} not found")
            totalError += 1

    for partType, partList in JSONData.items():
        for part in partList:
            for partProperty in part.keys():
                if partProperty not in known_properties:
                    print(f"Property {partProperty} not found {part}")
                    totalError += 1

            if "Reload_Time" in part and partType != "Magazines":
                print(f"Part {part} contains Reload_Time but is not a Magazine")

            if part["Name"] not in known_names:
                print(f"Name {part['Name']} not found")
                totalError += 1

            if part["Category"] not in known_categories:
                print(f"Category {part['Category']} not found")
                totalError += 1

    if totalError > 0:
        print(f"Total errors: {totalError}")
        exit(1)


def CleanUp():
    print("Running Clean up")
    os.system("rm Data/*.txt")


def Penalties():

    """
    It doesn't really look that good when dumping from a script but you can look at the current penalties in this python script instead haha
    The penalties will get accessed based on this string to index conversion table.
    """

    print("Running Penalties")

    with open("Data/Categories.json", 'w') as file:
        json.dump(current_categories, file, indent=2)

    with open("Data/Penalties.json", 'w') as file:
        json.dump(current_penalties, file)


def main():
    Setup()
    CleanData()
    TurnToJSON()
    ValueFormat()
    ValidateData()
    CleanUp()
    Penalties()


if __name__ == '__main__':
    main()

import os
import csv
import json
from pathlib import Path
# from FileFormatter import Penalties

SHEETID = '1Kc9aME3xlUC_vV5dFRe457OchqUOrwuiX_pQykjCF68'
SHEETFOLDER = Path('SheetData')

PARTSHEET = SHEETFOLDER / 'parts.csv'
PARTSHEETGID = '503295784'
PARTSHEET2 = SHEETFOLDER / 'parts2.csv'
PARTSHEET2GID = '319672878'
CORESHEET = SHEETFOLDER / 'cores.csv'
CORESHEETGID = '911413911'

OUTPUTFILE = Path('Data') / 'FullData.json'


translatePropertyName_part = {
    # These will get processed case by case (also will be uncaptilizing everything incase of issues)
    # 'Reload_Time',
    'reload': 'Reload_Speed',
    'recoil': 'Recoil',
    'magazine': 'Magazine_Size',
    'fire rate': 'Fire_Rate',
    'movement speed': 'Movement_Speed',
    'health': 'Health',
    'damage': 'Damage',
    'spread': 'Spread',
    'pellets': 'Pellets',
    'detection radius': 'Detection_Radius',
    'range': 'Range',
    'equip time': 'Equip_Time'
}

validPartCategories = ['AR', 'Sniper', 'SMG', 'LMG', 'Shotgun', 'BR', 'Weird', 'Sidearm']
validPartTypes = ['Barrels', 'Magazines', 'Grips', 'Stocks']
validPriceTypes = ['Coin', 'WC', 'Follow', 'Robux', 'Free', 'Spin', 'Limited', 'Missions', 'Verify discord', 'Unknown'] # The calculator will only detect "Coin" "WC" "Robux". Anything else will be turned into "Special"

def FindSameName(obj, name):
    for value in obj:
        if value["Name"] == name:
            return value

def DeepCompare(obj1, obj2):
    for partType, partList in obj1.items():
        print(f"\n------------------------- {partType} -----------------------------\n")
        for part in partList:
            if part["Name"] == "Stat Randomizer": continue
            part2 = FindSameName(obj2[partType], part["Name"])
            if part2 is None:
                print(f"Part {part['Name']} not found in {partType}")
                continue
            for i, v in part.items():
                try:
                    if v != part2[i]:
                        print(part, '\n', part2)
                        print()
                        break
                except KeyError:
                    print(part, '\n', part2)
                    print()
                    break

def FormatNumber(value: str, doubleNum=False):
    if value.strip() == "" or value.strip() == "🎲":
        return None
    value = value.replace("°","").replace("s","").replace("rpm","").replace("%", "").replace(",","").replace(">","-").strip()
    output = 0

    if doubleNum:
        if " - " in value: # " > " gets turned into " - " above
            vals = value.split(" - ")
            if "x" in vals[0]:
                vals[0] = float(vals[0].split("x")[0]) * float(vals[0].split("x")[1])
            if "x" in vals[1]:
                vals[1] = float(vals[1].split("x")[0]) * float(vals[1].split("x")[1])
            output = vals
            output[0] = float(output[0])
            output[1] = float(output[1])
        else:
            raise RuntimeError(f"Couldn't find separator for {value}")

        assert len(output) == 2, f"Expected two numbers, got {value}"
        for i, e in enumerate(output):
            if int(e) == e:
                output[i] = int(e)
    else:
        output = float(value)
        if int(output) == output:
            output = int(output)

    return output

def DetectPriceType(price, row) -> str:
    price = price.strip().replace(",", "") # Remove commas from the values more than 1,000
    
    if ("WC" in price):
        return "WC"
    try:
        price = int(price)
        return "Coin"
    except ValueError: # Price must be either Robux or Limited or Free
        price = price.capitalize()
        if (price not in validPriceTypes):
            print(f"WARNING: Invalid price type detected at row {row}")
            return "Unknown"
    return price

def DownloadSheet():

    os.system(f'rm -f {SHEETFOLDER}/*')
    os.system(f'wget -O {PARTSHEET} "https://docs.google.com/spreadsheets/d/{SHEETID}/export?format=csv&id={SHEETID}&gid={PARTSHEETGID}"')
    os.system(f'wget -O {CORESHEET} "https://docs.google.com/spreadsheets/d/{SHEETID}/export?format=csv&id={SHEETID}&gid={CORESHEETGID}"')
    os.system(f'wget -O {PARTSHEET2} "https://docs.google.com/spreadsheets/d/{SHEETID}/export?format=csv&id={SHEETID}&gid={PARTSHEET2GID}"')
    
def ParsePartsv2(outputData):
    with open(PARTSHEET2, 'r') as file:
        data = [row[1:] for row in list(csv.reader(file))[2:]] # include price info

    currentCategory = 'AR'
    currentType = ''
    
    seenParts = {x: set() for x in validPartCategories}

    for row in data:
        try: 
            if len(row) == 0: continue
            assert len(row) == 15, f"invalid row length {row} expected 15"
    
            name = row[1].strip()
            
            # Check if it is a category divider
            categoryType = name.split(' ')
            if len(categoryType) == 2:
                if categoryType[0] == "Notable": # Remove notable scopes
                    break
                if categoryType[0] in validPartCategories and categoryType[1] in validPartTypes:
                    currentCategory = categoryType[0]
                    currentType = categoryType[1]
                    continue
    
            # if name[:-6] in validPartCategories: # remove the word Cores from the back
            #     currentCategory = name[:-6]
            #     continue
            
            if (name in seenParts[currentCategory]):
                raise ValueError(f"Duplicate part name '{name}' in category '{currentCategory}'")
                
            part = {
                "Price_Type": DetectPriceType(row[0], row),
                "Name": name,
                "Category": currentCategory,
            }
    
            propertyList = [
                "Magazine_Size",
                "Reload_Time",
                "Damage",
                "Detection_Radius",
                "Equip_Time",
                "Fire_Rate",
                "Health",
                "Movement_Speed",
                "Pellets",
                "Range",
                "Recoil",
                "Reload_Speed",
                "Spread",
            ]
    
            for i in range(2, 15):
                property = row[i].strip()
                if property == '': continue
    
                property = [property[:property.index(' ')], property[property.index(' ') + 1:]]
                assert len(property) == 2, f"Invalid property format: {property} {row}"
    
                value, propertyName = property
                # propertyName = propertyName.strip().lower()
    
                propertyName = propertyList[i-2]
    
                part[propertyName] = FormatNumber(value)
    
            outputData[currentType].append(part)
        except Exception as e:
            print(f"Error parsing row {row}", flush=True)
            raise e


def ParseCores(outputData):

    with open(CORESHEET, 'r') as file:
        data = [row[1:] for row in list(csv.reader(file))[2:]]

    currentCategory = 'AR'
    for row in data:
        try:
            if len(row) == 0: continue
            assert len(row) == 18, f"invalid row length {len(row)} expected 18"
            
            name = row[1].strip()
    
            if name[:-6] in validPartCategories: # remove the word Cores from the back
                currentCategory = name[:-6]
                continue
    
            core = {
                "Price_Type": DetectPriceType(row[0], row),
                "Name": name,
                "Category": currentCategory,
            }
    
            propertyList = [
                "Damage",
                "Dropoff_Studs",
                "Fire_Rate",
                "Hipfire_Spread",
                "ADS_Spread",
                "Time_To_Aim",
                "Detection_Radius",
                "Burst",
                "Movement_Speed_Modifier",
                "Suppression",
                "Health",
                "Equip_Time",
                "Recoil_Hip_Horizontal",
                "Recoil_Hip_Vertical",
                "Recoil_Aim_Horizontal",
                "Recoil_Aim_Vertical"
            ]
    
            for i in range(2, 18):
                if i == 1: # Damage x Pellet
                    pellet  = row[i].split(" > ")[0].split("x")
                    if len(pellet) == 2:
                        core["Pellets"] = int(pellet[1])
    
                formattedVal = FormatNumber(row[i], doubleNum=(i <= 3 or i >= 14))
                if formattedVal is not None:
                    core[propertyList[i-2]] = formattedVal
    
            outputData["Cores"].append(core)
        
        except Exception as e:
            print(f"Error parsing row {row}", flush=True)
            raise e

def SaveData(outputData):
    with open(OUTPUTFILE, 'w') as file:
        json.dump(outputData, file, indent=2)

def Compare():
    with open(OUTPUTFILE, 'r') as file:
        SheetData = json.load(file)
    with open("Data/FullData.json", 'r') as file:
        MyData = json.load(file)

    DeepCompare(SheetData, MyData)

# Anything with current_ is part of the Data used for calculations. Not part of the FileFormatter.py which is why there are two category variables
# The number is the index for current_penalties
current_categories = {
    "Primary": {
        "AR": 0,
        "Sniper": 1,
        "SMG": 2,
        "Shotgun": 3,
        "LMG": 4,
        "Weird": 5,
        "BR": 6,
    },
    "Secondary": {
        "Sidearm": 7
    }
}

current_penalties = [
    [1.00, 0.70, 0.79, 0.75, 0.75, 1.00, 0.80, 0.65],
    [0.70, 1.00, 0.60, 0.60, 0.80, 1.00, 0.85, 0.50],
    [0.80, 0.60, 1.00, 0.65, 0.65, 1.00, 0.70, 0.70],
    [0.70, 0.50, 0.65, 1.00, 0.75, 1.00, 0.60, 0.65],
    [0.75, 0.80, 0.65, 0.75, 1.00, 1.00, 0.85, 0.50],
    [1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00],
    [0.80, 0.85, 0.70, 0.60, 0.85, 1.00, 1.00, 0.65],
    [0.65, 0.50, 0.75, 0.65, 0.50, 1.00, 0.65, 1.00],
]

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
    DownloadSheet()
    outputData = {"Barrels": [], "Magazines": [], "Grips": [], "Stocks": [], "Cores": []}
    # ParseParts(outputData)
    ParsePartsv2(outputData)
    ParseCores(outputData)
    fullData = {"Data": outputData, "Penalties": current_penalties, "Categories": current_categories}
    SaveData(fullData)
    # Compare()


if __name__ == "__main__":
    main()

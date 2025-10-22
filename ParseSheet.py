import os
import csv
import json
from pathlib import Path
from FileFormatter import Penalties

SHEETID = '1Kc9aME3xlUC_vV5dFRe457OchqUOrwuiX_pQykjCF68'
SHEETFOLDER = Path('SheetData')

PARTSHEET = SHEETFOLDER / 'parts.csv'
PARTSHEETGID = '503295784'
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

def DownloadSheet():

    os.system(f'rm -f {SHEETFOLDER}/*')
    os.system(f'wget -O {PARTSHEET} "https://docs.google.com/spreadsheets/d/{SHEETID}/export?format=csv&id={SHEETID}&gid={PARTSHEETGID}"')
    os.system(f'wget -O {CORESHEET} "https://docs.google.com/spreadsheets/d/{SHEETID}/export?format=csv&id={SHEETID}&gid={CORESHEETGID}"')

def ParseParts(outputData):

    with open(PARTSHEET, 'r') as file:
        data = [a.split(',')[2:] for a in file.read().split('\n')[1:]] # Split by newlines then by commas and then ignore the first two entries

    currentCategory = ''
    currentType = ''
    for row in data:
        if len(row) == 0: continue
        name = row[0]

        # Check if it is a category divider
        categoryType = name.split(' ')
        if len(categoryType) == 2:
            if categoryType[0] == "Notable": # Remove notable scopes
                break
            if categoryType[0] in validPartCategories and categoryType[1] in validPartTypes:
                currentCategory = categoryType[0]
                currentType = categoryType[1]
                continue

        part = { "Name": name, "Category": currentCategory}

        for property in row[1:]:
            if property == '':
                break
            property = [property[:property.index(' ')], property[property.index(' ') + 1:]]
            assert len(property) == 2, f"Invalid property format: {property} {row}"

            value, propertyName = property
            propertyName = propertyName.strip().lower()


            if currentType == "Magazines" and propertyName == 'reload':
                propertyName = 'Reload_Time' if 's' in value else 'Reload_Speed'
            else:
                assert propertyName in translatePropertyName_part, f"Invalid property name: {propertyName} {row}"
                propertyName = translatePropertyName_part[propertyName]

            part[propertyName] = FormatNumber(value)
        outputData[currentType].append(part)


def ParseCores(outputData):

    with open(CORESHEET, 'r') as file:
        data = [row[2:] for row in list(csv.reader(file))[2:]]

    currentCategory = 'AR'
    for row in data:

        if len(row) == 0: continue
        assert len(row) == 17, f"invalid row length {row} expected 17"
        name = row[0]

        if name[:-6] in validPartCategories: # remove the word Cores from the back
            currentCategory = name[:-6]
            continue

        core = {
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

        for i in range(1, 17):
            if i == 1: # Damage x Pellet
                pellet  = row[i].split(" > ")[0].split("x")
                if len(pellet) == 2:
                    core["Pellets"] = int(pellet[1])

            formattedVal = FormatNumber(row[i], doubleNum=(i <= 2 or i >= 13))
            if formattedVal is not None:
                core[propertyList[i-1]] = formattedVal

        outputData["Cores"].append(core)

def SaveData(outputData):
    with open(OUTPUTFILE, 'w') as file:
        json.dump(outputData, file, indent=2)

def Compare():
    with open(OUTPUTFILE, 'r') as file:
        SheetData = json.load(file)
    with open("Data/FullData.json", 'r') as file:
        MyData = json.load(file)

    DeepCompare(SheetData, MyData)


def main():
    DownloadSheet()
    outputData = {"Barrels": [], "Magazines": [], "Grips": [], "Stocks": [], "Cores": []}
    ParseParts(outputData)
    ParseCores(outputData)
    SaveData(outputData)
    Penalties() # so this can be ran in one file instead of two. (I don't think I'll be running FileFormatter anymore)
    # Compare()


if __name__ == "__main__":
    main()

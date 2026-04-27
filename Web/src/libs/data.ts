import type { PartCoreList } from "./types";

export const fullData_STRING = await fetch("/FullData.json").then(r => r.text());
export const fullData_DATA = JSON.parse(fullData_STRING);

export const PARTCORELIST = fullData_DATA.Data as PartCoreList;
export const CATEGORIES = fullData_DATA.Categories;
export const PENALTIES = fullData_DATA.Penaltiles as number[][];

export const PARTNAMES: string[] = [];
export const FIRINGMODES: string[] = [];

{
  const nameSet = new Set<string>();
  for (const part of PARTCORELIST.Barrels) nameSet.add(part.Name);
  for (const part of PARTCORELIST.Magazines) nameSet.add(part.Name);
  for (const part of PARTCORELIST.Grips) nameSet.add(part.Name);
  for (const part of PARTCORELIST.Stocks) nameSet.add(part.Name);
  for (const part of PARTCORELIST.Cores) nameSet.add(part.Name);
  for (const part of Array.from(nameSet).sort()) PARTNAMES.push(part);
}

{
  const firingModeSet = new Set<string>();
  for (const part of PARTCORELIST.Cores) firingModeSet.add(part.Firing_Mode);
  for (const part of Array.from(firingModeSet).sort()) FIRINGMODES.push(part);
}


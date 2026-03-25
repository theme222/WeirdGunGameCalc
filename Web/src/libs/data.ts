import type { PartCoreList } from "./types";

export const fullData_STRING = await fetch("/FullData.json").then(r => r.text());
export const fullData_DATA = JSON.parse(fullData_STRING);

export const PARTCORELIST = fullData_DATA.Data as PartCoreList;
export const CATEGORIES = fullData_DATA.Categories;
export const PENALTIES = fullData_DATA.Penaltiles as number[][];

export const PARTNAMES: string[] = [];

{
  const nameSet = new Set<string>();
  for (const pn of PARTCORELIST.Barrels) nameSet.add(pn.Name);
  for (const pn of PARTCORELIST.Magazines) nameSet.add(pn.Name);
  for (const pn of PARTCORELIST.Grips) nameSet.add(pn.Name);
  for (const pn of PARTCORELIST.Stocks) nameSet.add(pn.Name);
  for (const pn of PARTCORELIST.Cores) nameSet.add(pn.Name);
  const sorted = Array.from(nameSet).sort();
  for (const pn of sorted) PARTNAMES.push(pn);
}


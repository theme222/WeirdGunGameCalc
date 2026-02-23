export const fullData_DATA = await fetch("/FullData.json").then(r => r.json());

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
  for (const pn of nameSet) PARTNAMES.push(pn);
}


export type fpair = [number, number];

export interface Part {
  Price_Type: string,
  Name: string,
  Category: string,
  Magazine_Size?: number,
  Reload_Time?: number,
  Damage?: number,
  Detection_Radius?: number,
  Equip_Time?: number,
  Fire_Rate?: number,
  Health?: number,
  Movement_Speed?: number,
  Pellets?: number,
  Range?: number,
  Recoil?: number,
  Reload_Speed?: number,
  Spread?: number,
}

export interface Core {
  Price_Type: string,
  Name: string,
  Category: string,
  Damage?: fpair,
  Dropoff_Studs?: fpair, 
  Fire_Rate?: number,
  Hipfire_Spread?: number,
  ADS_Spread?: number,
  Time_To_Aim?: number,
  Detection_Radius?: number,
  Burst?: number,
  Movement_Speed_Modifier?: number,
  Suppression?: number,
  Health?: number,
  Equip_Time?: number,
  Recoil_Hip_Horizontal?: fpair,
  Recoil_Hip_Vertical?: fpair,
  Recoil_Aim_Horizontal?: fpair,
  Recoil_Aim_Vertical?: fpair
}

export interface Gun {
  Barrel: Part | null;
  Magazine: Part | null;
  Grip: Part | null;
  Stock: Part | null;
  Core: Core | null;
}

export interface PartCoreList {
  Barrels: Part[];
  Magazines: Part[];
  Grips: Part[];
  Stocks: Part[];
  Cores: Core[];
}

import type { filterAndSortStrings } from "./filter.const";

export type FilterType = 'string' | 'number' | 'stringarr' | 'numberrange' | 'sort';

export interface Filter {
  title: string;
  filterType: FilterType;
  readonly validStrings?: readonly string[]; // only on filterType === 'string'
  readonly options?: readonly string[];
  required?: boolean;
}

export interface FilterItem {
  id: number; // I think this can just be the index inside of the filter List
  title: string;
  filterType: FilterType;
  readonly validStrings?: readonly string[]; // only on filterType === 'string'
  readonly options?: readonly string[];
  required?: boolean;
  writeable: {
    selectedOption?: string;
    value: string[] | number[];
  };
}

export type fpair = [number, number];

export interface Part {
  Price_Type: string,
  Name: string,
  Category: string,
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
  Magazine_Cap?: number,
  Spread?: number,
}

export interface Barrel extends Part {}
export interface Magazine extends Part {
  Magazine_Size?: number,
  Reload_Time?: number,
}
export interface Grip extends Part {}
export interface Stock extends Part {}

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

export interface PartCoreList {
  Barrels: Barrel[];
  Magazines: Magazine[];
  Grips: Grip[];
  Stocks: Stock[];
  Cores: Core[];
}

export class Gun {
  // This class will most likely contain static information and will be unchanging. (Very little methods.)
  barrel: string = "";
  magazine: string = "";
  grip: string = "";
  stock: string = "";
  core: string = "";
  
  damage: fpair = [0, 0];
  dropoffStuds: fpair = [0, 0];
  falloffFactor: number = 0;
  pellets: number = 0;
  burst: number = 0;
  reloadTime: number = 0;
  magazineSize: number = 0;
  health: number = 0;
  fireRate: number = 0;
  timeToAim: number = 0;
  equipTime: number = 0;
  movementSpeedModifier: number = 0;
  hipfireSpread: number = 0;
  adsSpread: number = 0;
  detectionRadius: number = 0;
  recoilHipHorizontal: fpair = [0, 0];
  recoilHipVertical: fpair = [0, 0];
  recoilAimHorizontal: fpair = [0, 0];
  recoilAimVertical: fpair = [0, 0];
  TTKM: number = 0;
  TTKS: number = 0;
  DPM: number = 0;
  DPS: number = 0;
 
}

export type WorkerAction = "calculate" | "queryVersion" | "startup";

export type WorkerResult = {
  success: true;
  action: "calculate";
  startTime: number;
  endTime: number;
  data: Gun[];
} | {
  success: true;
  action: "queryVersion";
  startTime: number;
  endTime: number;
  data: string;
} | {
  success: true;
  action: "startup";
  startTime: number;
  endTime: number;
  data: null;
} | {
  success: false;
  action: WorkerAction;
  startTime: number;
  endTime: number;
  data: Error;
}
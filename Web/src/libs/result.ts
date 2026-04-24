import { ref, reactive, watch } from "vue";
import { filterPropStrings, propertyStrings } from "./filter.const";
import { currentFilters, getFilterItem } from "./filter";
import { latestResult } from "./calc";
import type { fpair, Gun } from "./types";

export const statsToCompare = reactive(filterPropStrings.map((stat) => ({ stat, show: false })));
const statsToShowDefault: (typeof filterPropStrings)[number][] = ['TTK', 'Damage Start', 'Damage End', 'Fire Rate', 'Magazine Size', 'DropoffStuds End', 'DropoffStuds Start', 'Spread Aim', 'Spread Hip', 'Speed', 'Health', 'Recoil Aim', 'Recoil Hip', 'Reload Time']; // Truly one of the typescript moments of all time
statsToShowDefault.forEach((stat) => statsToCompare.find((s) => s.stat === stat)!.show = true);

export const currentSortingType = ref('TTK');

watch(latestResult, () => {
  // Attempt to fetch the latest sorting type from the filter
  currentSortingType.value = currentFilters.list.find((filter) => filter.title === 'Sort Type')!.writeable.value[0] as string; 
})

export function getFilterAsProperty(gun: Gun, filterName: (typeof propertyStrings)[number]): fpair | number {
   switch (filterName) {
     case 'TTK': return gun.TTKS;
     case 'TTK End': return gun.TTKSEnd;
     case 'TTE': return gun.TTE;
     case 'Damage Start': return gun.damage[0];
     case 'Damage End': return gun.damage[1];
     case 'Damage': return gun.damage;
     case 'Fire Rate': return gun.fireRate;
     case 'Pellets': return gun.pellets;
     case 'Spread Aim': return gun.adsSpread;
     case 'Spread Hip': return gun.hipfireSpread;
     case 'Recoil Aim': return gun.recoilAimVertical;
     case 'Recoil Hip': return gun.recoilHipVertical;
     case 'Health': return gun.health;
     case 'DropoffStuds Start': return gun.dropoffStuds[0];
     case 'DropoffStuds End': return gun.dropoffStuds[1];
     case 'DropoffStuds': return gun.dropoffStuds;
     case 'Detection Radius': return gun.detectionRadius;
     case 'Time To Aim': return gun.timeToAim;
     case 'Equip Time': return gun.equipTime;
     case 'Burst': return gun.burst;
     case 'Speed': return gun.movementSpeedModifier;
     case 'Magazine Size': return gun.magazineSize;
     case 'Reload Time': return gun.reloadTime;
     case 'DPS': return gun.DPS;
     case 'DPS End': return gun.DPSEnd;
   }
 }
 
 export function ffv(value: fpair | number): string { // Format filter value
   if (typeof value === 'number') return parseFloat(value.toFixed(3)).toString();
   return `${parseFloat(value[0].toFixed(3))} - ${parseFloat(value[1].toFixed(3))}`;
 }
 
 export function ffvNumber(value: number): number { // Format filter value as number
   return parseFloat(value.toFixed(3));
 }
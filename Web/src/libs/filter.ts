import { reactive, ref } from 'vue';
import { addToast, removeToast } from './toast';

let filterId = 0;

export type FilterType = 'string' | 'number' | 'manystring';

export interface Filter {
  title: string;
  filterType: FilterType;
  validStrings?: string[]; // only on filterType === 'string'
  required?: boolean;
}

export interface FilterItem {
  id: number;
  title: string;
  filterType: FilterType;
  validStrings?: string[]; // only on filterType === 'string'
  writeable: {
    filterOption: 'min' | 'max' | 'from'; // Only in filterType === 'number'
    value: string[] | number[];
  }
}

// TODO DYNAMICALLY LOAD THIS
export const validGunStrings = [
  'MAC-10', 'M1919 .30 Cal', 'Type 99', 'Scar H', 'NP9', 'Honk', 'Tommy', 'Plunger', 'M1891',
  'Skorpion vz. 61', 'M1917 WC', 'Improvised', 'Scar L', 'Lee Enfield',
  'Horizontal', 'Willy Fischy', 'Anvil', 'Shower', 'Stat Randomizer', 'Famas F1', 'Speed Coil',
  'Water Pipe', 'Ketchup', 'M1919', 'UNP-40', 'MP 40', 'Type 2a Nambu', 'Banana', 'Shovel',
  'STG-44', 'Oil', 'Fedorov', 'Tap', 'Rice Pot', 'Scar Drum', 'M-16', 'Hecate II', 'M40',
  'M91 Moschetto', 'DP-27', 'M60', 'Quack', 'AUG', 'Restroom', 'AKM', 'XXL AKM', 'As-Val', 'MG42',
  'Mustard', 'Hell-Trooper-23', 'Kriss Vector', 'Words as Weapons', 'The Chief', 'Mas-38', 'Bicycle',
  'PPSh-41', 'Spas-12', 'M870 Wingmaster', 'M1887', 'M1897 Trench Shotgun', 'M870 Breacher',
  'M870 MCS', 'M870', 'Spas-12 Folded', 'Hex Spitter', 'PP-19 Bizon', 'BAR', 'C36C',
  'APS', 'NP5', 'Circuit Judge', 'NP5A3', 'Negev', 'M2000', 'Pistol Pistol', 'Auto-5', 'Croissant',
  'Loaf', 'Taco', 'Eclair', 'Pastel', 'P90', 'Chauchat', 'Dragunov', 'M1Garand', 'FAL', 'FC200SH', 'Light Gun',
  'Gaming Rig', 'Controller', 'Graphics Card', 'Keyboard', 'Musket', 'Snake Sniper', 'M110', 'SKS', 'Blunderbuss',
  'AR-8X', 'Freedom Dispenser', 'Second Amendment', 'JSG12', 'AWM', 'Arisaka', 'Sten gun', 'M1 Carbine',
  'M1919 Tripod', 'FG-42', 'G11', 'AK-12', 'QBZ-95', 'Lightning Shot', '10KG Dumbbell', 'VSSR', 'BarrelSight',
  'Screwdriver', 'Cirrus - Hand', 'Napo - Scissors', 'Rat - Emoji', 'Derpy - Staff', 'Uzi', 'G3',
  'Dominus', 'Lipstick', 'Lewis', 'Mega Soaker', 'M1911', 'Tec-9', 'B17', 'Deagle', 'Car Muffler', 'Lamp Shade',
  'Luger', 'Beretta', 'Double Barrel', 'Winchester', 'Type 14', 'Makarov', 'P08 Artillery', 'M249 SAW', 'Mosin',
  'Obreez', 'Paintball', 'Spray Can', 'MP18', 'Burton', 'Cyber 3517', 'T10', 'EG200', 'Broza', 'Kolibri'
]

export const categoryStrings = [
  'AR',
  'SMG',
  'Sniper',
  'BR',
  'LMG',
  'Shotgun',
  'Sidearm'
]

export const filterAndSortStrings = [
  'TTK',
  'Damage Start', 
  'Damage End',
  'Fire Rate',
  'Pellets',
  'Spread Hip',
  'Spread Aim',
  'Recoil Hip',
  'Recoil Aim',
  'Health',
  'Range Stud Start',
  'Range Stud End',
  'Detection Radius',
  'Time To Aim',
  'Burst',
  'Speed',
  'Magazine Size',
  'Reload Time',
  'DPS'
] 

export const filterList: Filter[] = [
  { title: 'Categories', filterType: 'string', required: true, validStrings: categoryStrings },
  { title: 'Sort Type', filterType: 'string', required: true, validStrings: filterAndSortStrings},
  { title: 'Total Results', filterType: 'number', required: true },
  { title: 'Force Core', filterType: 'string', validStrings: validGunStrings },
  { title: 'Force Magazine', filterType: 'string', validStrings: validGunStrings },
  { title: 'Force Barrel', filterType: 'string', validStrings: validGunStrings },
  { title: 'Force Stock', filterType: 'string', validStrings: validGunStrings },
  { title: 'Force Grip', filterType: 'string', validStrings: validGunStrings },
];

for (const filter of filterAndSortStrings) {
  filterList.push({
    title: filter,
    filterType: 'number'
  })
}

export const currentFilters = reactive<{ list: FilterItem[] }>({ list: [] });

export function addFilter(selectedFilterToAdd: Filter) {

  // check if filter already exists
  if (currentFilters.list.some((filter) => filter.title === selectedFilterToAdd.title)) {
    addToast(`Filter ${selectedFilterToAdd.title} already exists`, 'warning');
    return;
  }

  currentFilters.list.push({
    id: filterId++,
    writeable: {
      filterOption: 'min',
      value: selectedFilterToAdd.filterType === 'number' ? [0, 0] : [],
    },
    ...selectedFilterToAdd,
  });
  
  console.log(currentFilters);
  addToast(`Added ${selectedFilterToAdd.title} to filters`, 'info');
}

export function removeFilter(id: number) {
  const index = currentFilters.list.findIndex((filter) => filter.id === id);
  const filterToRemove = currentFilters.list[index];
  if (index !== -1) {
    currentFilters.list.splice(index, 1);
    addToast(`Removed ${filterToRemove?.title} from filters`, 'info');
  }
}
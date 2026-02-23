import { reactive, ref } from 'vue';
import { addToast, removeToast } from './toast';
import { PARTNAMES } from './data';

let filterId = 0;

export type FilterType = 'string' | 'number' | 'stringarr' | 'numberrange' | 'sort';

export interface Filter {
  title: string;
  filterType: FilterType;
  validStrings?: string[]; // only on filterType === 'string'
  options?: string[];
  required?: boolean;
}

export interface FilterItem {
  id: number;
  title: string;
  filterType: FilterType;
  validStrings?: string[]; // only on filterType === 'string'
  options?: string[];
  writeable: {
    selectedOption?: string;
    value: string[] | number[];
  };
}

export const categoryStrings = ['AR', 'SMG', 'Sniper', 'BR', 'LMG', 'Shotgun', 'Sidearm'];

export const filterAndSortStrings = [
  'TTK', 'Damage Start', 'Damage End', 'Fire Rate', 'Pellets', 'Spread Hip', 'Spread Aim', 'Recoil Hip',
  'Recoil Aim', 'Health', 'Range Stud Start', 'Range Stud End',
  'Detection Radius', 'Time To Aim', 'Burst', 'Speed', 'Magazine Size', 'Reload Time', 'DPS',
];

export const numberRangeOptions = ['min', 'max', 'from']
export const sortTypeOptions = ['highest first', 'lowest first']

export const filterList: Filter[] = [
  { title: 'Categories', filterType: 'stringarr', required: true, validStrings: categoryStrings },
  {
    title: 'Sort Type',
    filterType: 'sort',
    options: sortTypeOptions,
    required: true,
    validStrings: filterAndSortStrings,
  },
  { title: 'Total Results', filterType: 'number', required: true },
  { title: 'Force Core', filterType: 'stringarr', validStrings: PARTNAMES },
  { title: 'Force Magazine', filterType: 'stringarr', validStrings: PARTNAMES },
  { title: 'Force Barrel', filterType: 'stringarr', validStrings: PARTNAMES },
  { title: 'Force Stock', filterType: 'stringarr', validStrings: PARTNAMES },
  { title: 'Force Grip', filterType: 'stringarr', validStrings: PARTNAMES },
];

for (const filter of filterAndSortStrings) {
  filterList.push({
    title: filter,
    filterType: 'numberrange',
    options: numberRangeOptions 
  });
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
      selectedOption: selectedFilterToAdd.options?.[0] || undefined, // Funky ahh syntax over here
      value: selectedFilterToAdd.filterType === 'number' ? [0, 0] : []
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

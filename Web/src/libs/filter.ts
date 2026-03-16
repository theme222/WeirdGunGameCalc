import { reactive, watch } from 'vue';
import { addToast } from './toast';
import { PARTNAMES } from './data';
import { onFilterChange } from './calc';
import type { Filter, FilterItem } from './types';
import { categoryStrings, sortTypeOptions, filterAndSortStrings, numberRangeOptions } from './filter.const';

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
  { title: 'Ban Core', filterType: 'stringarr', validStrings: PARTNAMES },
  { title: 'Ban Magazine', filterType: 'stringarr', validStrings: PARTNAMES },
  { title: 'Ban Barrel', filterType: 'stringarr', validStrings: PARTNAMES },
  { title: 'Ban Stock', filterType: 'stringarr', validStrings: PARTNAMES },
  { title: 'Ban Grip', filterType: 'stringarr', validStrings: PARTNAMES },
];

function getFilterItem(title: string, writeableOverride?: FilterItem["writeable"]): FilterItem | undefined {
  let index = null
  for (let i = 0; i < filterList.length; i++) {
    if (filterList[i]!.title === title) {
      index = i;
      break;
    }
  }
  
  if (index === null) return undefined;
  const selectedFilterToAdd = filterList[index]!;
  
  const filter: FilterItem = {
    id: index,
    writeable: {
      selectedOption: selectedFilterToAdd.options?.[0] || undefined, // Funky ahh syntax over here
      value: [],
    },
    ...selectedFilterToAdd,
  };
  
  if (writeableOverride) 
    filter.writeable = writeableOverride;
  
  return filter;
}

for (const filter of filterAndSortStrings) {
  filterList.push({
    title: filter,
    filterType: 'numberrange',
    options: numberRangeOptions,
  });
}

function sortFilterList() {
  
}

function getFiltersFromURL(): { list: FilterItem[] } {
  const params = new URLSearchParams(window.location.search);

  const defaultState = [
    getFilterItem('Categories', { value: ["AR", "SMG", "LMG"] })!,
    getFilterItem('Sort Type', { selectedOption: "Auto", value: ["TTK"] })!,
    getFilterItem('Total Results', { value: [10] })!,
  ];

  // Thx chatgpt
  const hydratedList: FilterItem[] = [];

  for (const [key, value] of params.entries()) {
    try {
      // Convert underscores back to spaces to find the matching filter
      const originalTitle = key.replace(/_/g, ' ');
      const baseFilter = getFilterItem(originalTitle);
      
      if (!baseFilter) continue;
  
      let parsedSelectedOption = undefined;
      let parsedValues: any[] = [];
  
      // Check if it has our custom "selectedOption:Values" format
      if (value.includes(':')) {
        const parts = value.split(':');
        parsedSelectedOption = parts[0];
        parsedValues = parts[1] ? parts[1].split(',') : [];
      } else {
        parsedValues = value ? value.split(',') : [];
      }
  
      // Vue v-model needs actual numbers for number inputs, not strings.
      if (baseFilter.filterType === 'number' || baseFilter.filterType === 'numberrange') {
        parsedValues = parsedValues.map(v => Number(v));
      }
      
      baseFilter.writeable = { selectedOption: parsedSelectedOption, value: parsedValues }
      hydratedList.push(baseFilter);
    }
    catch (e) {
      console.error(e);
    }
  }
  
  // default state contains filters that are required. 
  // Individually check required filters and add them if not present.
  for (const filter of defaultState) {
    if (!hydratedList.some(f => f.title === filter.title)) {
      hydratedList.push(filter);
    }
  }
  
  hydratedList.sort((a, b) => a.id - b.id);
  return { list: hydratedList };
}

export const currentFilters = reactive<{ list: FilterItem[] }>(getFiltersFromURL());

watch(
  currentFilters,
  (newState) => {
    // order the currentFilters by the id
    currentFilters.list = newState.list.sort((a, b) => a.id - b.id);
    
    const url = new URL(window.location.href);
    
    // 1. Clear out all previous filter parameters so removed filters disappear
    filterList.forEach(f => url.searchParams.delete(f.title.replace(/\s+/g, '_')));

    // 2. Build the new clean parameters
    newState.list.forEach(filter => {
      const key = filter.title.replace(/\s+/g, '_');
      let valStr = '';
      
      if (filter.writeable.selectedOption) 
        valStr += filter.writeable.selectedOption + ':';
      
      // Add the array values joined by commas (e.g., "AR,SMG")
      if (filter.writeable.value && filter.writeable.value.length > 0) 
        valStr += filter.writeable.value.join(',');

      if (valStr)
        url.searchParams.set(key, valStr);
    });

    let finalUrl = url.toString();
    // This is because URLSearchParams is being a little bitch and not allowing , and : to exist
    finalUrl = finalUrl.replace(/%2C/g, ',').replace(/%3A/g, ':');
    window.history.replaceState({}, '', finalUrl);
    
    onFilterChange();
  },
  { deep: true }
);

export function addFilter(selectedFilterToAdd: Filter) {
  // check if filter already exists
  if (currentFilters.list.some((filter) => filter.title === selectedFilterToAdd.title)) {
    addToast(`Filter ${selectedFilterToAdd.title} already exists`, 'warning');
    return;
  }

  currentFilters.list.push(getFilterItem(selectedFilterToAdd.title)!);
  addToast(`Added ${selectedFilterToAdd.title} to filters`, 'info');
}

export function removeFilter(id: number) {
  const index = currentFilters.list.findIndex((filter) => filter.id === id);
  if (index !== -1) 
    currentFilters.list.splice(index, 1);
}

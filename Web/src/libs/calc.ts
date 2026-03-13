import { ref } from 'vue';
import { currentFilters } from './filter';
import { addToast } from './toast';

export const runOnFilterChange = ref(false);

export function onFilterChange() { 
  if (runOnFilterChange) runCalc();
}

export function runCalc() {
  addToast("Running with filters", "success");
}
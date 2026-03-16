import { ref, watch } from 'vue';
import { currentFilters } from './filter';
import { addToast } from './toast';
import { useDebounceFn, useWebWorker } from '@vueuse/core';

import workerURL from './calc.worker.ts?worker&url';

const { data, post, terminate, worker } = useWebWorker(workerURL, {type: 'module'});

watch(data, async (newData) => {
  console.log(newData);
  addToast("Calculation finished", "success");
});

export const runOnFilterChange = ref(false);

export function onFilterChange() { 
  if (runOnFilterChange.value) runCalc();
}

export const runCalc = useDebounceFn( async () => {
    // addToast("Running with filters", "warning");
    post({ filterList: JSON.parse(JSON.stringify(currentFilters.list)) });
  },
  300
)
import { reactive, ref, watch } from 'vue';
import { currentFilters } from './filter';
import { addToast } from './toast';
import { useDebounceFn, useWebWorker } from '@vueuse/core';

import workerURL from './calc.worker.ts?worker&url';
import type { WorkerResult } from './types';

export const { data, post, terminate, worker } = useWebWorker(workerURL, {type: 'module'});

export const runOnFilterChange = ref(false);
export const isCalculating = ref(false);
export const latestResult = reactive<WorkerResult>({ // Latest Calculation Result
  success: false,
  action: 'calculate',
  startTime: 0,
  endTime: 0,
  data: new Error("No result yet"),
});

export const calculatorVersion = ref("unknown");

watch(data, async (newData: WorkerResult) => {
  if (!newData.success) {
    addToast(`Calculation failed: ${newData.data.message || "wggcalc threw an unknown error"}`, "error");
  }
  switch (newData.action) {
    case 'calculate': {
      isCalculating.value = false;
      
      // Copy stuff over
      latestResult.data = newData.data;
      latestResult.success = newData.success;
      latestResult.startTime = newData.startTime;
      latestResult.endTime = newData.endTime;
      
      if (newData.success) addToast(`Calculation finished (took ${newData.endTime - newData.startTime}ms)`, "success");
      break;
    }
    case 'queryVersion': {
      if (newData.success) calculatorVersion.value = newData.data;
      break;
    }
    case 'startup': {
      console.log("Web worker is now ready");
      post({ action: 'queryVersion' })
      break;
    }
  }
});


watch(currentFilters, () => {
  if (runOnFilterChange.value) runCalc();
})

export const runCalc = useDebounceFn( async () => {
    // addToast("Running with filters", "warning");
    isCalculating.value = true;
    post({ action: 'calculate', data: JSON.parse(JSON.stringify(currentFilters.list)) });
  },
  300
)
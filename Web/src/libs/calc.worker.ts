// @ts-ignore
import createModule from './wggcalc.js';
import { fullData_STRING } from './data.js';
import { type FilterItem } from './types.js';
import { filterAndSortStrings } from './filter.const.js';

function sortTypeJSToWasm(sortType: string): string {
  const WasmSortList = [
    'TTK',
    'DAMAGE',
    'DAMAGEEND',
    'FIRERATE',
    'PELLETS',
    'SPREADAIM',
    'SPREADHIP',
    'RECOILAIM',
    'RECOILHIP',
    'HEALTH',
    'RANGE',
    'RANGEEND',
    'DETECTIONRADIUS',
    'TIMETOAIM',
    'BURST',
    'SPEED',
    'MAGAZINESIZE',
    'RELOAD',
    'DPS',
  ];
  
  return WasmSortList[filterAndSortStrings.indexOf(sortType)!]!;
}

function sortPriorityJSToWasm(priority: string): string  {
  if (priority === 'Highest') return 'HIGHEST';
  if (priority === 'Lowest') return 'LOWEST';
  if (priority === 'Auto') return 'AUTO'
  return '';
}

class WggCalcManager {
  instance: any;
  isReady: boolean;
  initPromise: Promise<any>;

  constructor() {
    this.instance = null;
    this.isReady = false;
    this.initPromise = this.init();
  }

  async init() {
    if (this.instance) return this.instance;
    
    try {
      // Initialize the Wasm module
      this.instance = await createModule();
      this.isReady = true;
      await this.instance.InitDataset(fullData_STRING);
      console.log("Wasm Dataset Initialized");
      return this.instance;
    } catch (e) {
      console.error("Failed to load Wasm:", e);
    }
    
  }

  // A helper to ensure we are ready before calling functions
  async ensureReady() {
    await this.initPromise;
  }

  // WASM API WRAPPERS //
  // async initializeData(jsonString: string) {
  //   await this.ensureReady();
  //   this.instance.InitDataset(jsonString);
  // }

  async addFilterRange(title: string, type: string, min: number, max: number) {
    await this.ensureReady();
    this.instance.AddFilterRange(title, type, min, max);
  }

  async addFilterForceBan(title: string, partsArray: string[]) {
    await this.ensureReady();
    
    // Convert JS Array to the Emscripten VectorString type you registered
    const vector = new this.instance.VectorString();
    partsArray.forEach(p => vector.push_back(p));
    
    this.instance.AddFilterForceBan(title, vector);
    
    // Clean up the vector memory in the Wasm heap
    vector.delete();
  }
  
  async addFilterRequired(total: number, sortType: string, sortPriority: string, categories: string[]) {
    await this.ensureReady();
    
    const vector = new this.instance.VectorString();
    categories.forEach(p => vector.push_back(p));
    
    this.instance.AddFilterRequired(total, sortType, sortPriority, vector );
    vector.delete();
  } 

  async clear() {
      await this.ensureReady();
      this.instance.Clear();
  }
  
  async runCalc() {
    await this.ensureReady();
    this.instance.RunCalc();
  }
  
  // WASM API WRAPPERS //
}

// Export a single instance (The Singleton)
const WasmCalc = new WggCalcManager();

self.onmessage = async (e) => {
  const { filterList }: { filterList: FilterItem[] } = e.data;
  
  await WasmCalc.clear();
  
  // Find the required filters first
  const totalVal = filterList.find(f => f.title === "Total Results")?.writeable.value[0] as number;
  const sortType = sortTypeJSToWasm(filterList.find(f => f.title === "Sort Type")?.writeable.value[0] as string);
  const sortPriority = sortPriorityJSToWasm(filterList.find(f => f.title === "Sort Type")?.writeable.selectedOption as string);
  const categories = filterList.find(f => f.title === "Categories")?.writeable.value as string[];
  
  if (totalVal && sortType && sortPriority && categories) {
    await WasmCalc.addFilterRequired(totalVal, sortType, sortPriority, categories);
  }
  
  for (const filter of filterList) {
    if (filter.required) continue; 
    
    if (filter.filterType === "numberrange") {
      let minValue = -69420.5;
      let maxValue = 69420.5;
      
      if (filter.writeable.selectedOption === "min") {
        minValue = filter.writeable.value[0] as number;
      }
      else if (filter.writeable.selectedOption === "max") {
        maxValue = filter.writeable.value[0] as number;
      }
      else {
        minValue = filter.writeable.value[0] as number;
        maxValue = filter.writeable.value[1] as number;
      }
      console.log(filter, minValue, maxValue)
      
      await WasmCalc.addFilterRange(filter.title, filter.writeable.selectedOption as string, minValue, maxValue);
    }
    else if (filter.filterType === "stringarr")
      await WasmCalc.addFilterForceBan(filter.title, filter.writeable.value as string[]);
  }
  await WasmCalc.runCalc();
  
  postMessage({ timestamp: Date.now()}); // make sure it triggers the watch
}
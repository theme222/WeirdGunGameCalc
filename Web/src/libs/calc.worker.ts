import createModule, { type EmbindModule as EmbindModule_WASM, type Gun as Gun_WASM, type fpair as fpair_WASM } from './wggcalc.js';
import { fullData_STRING } from './data.js';
import { type FilterItem, type fpair, Gun } from './types.js';
import { filterAndSortStrings } from './filter.const.js';

function sortTypeJSToWasm(sortType: string): string | undefined {
  const wasmSortList = [
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
    'EQUIPTIME',
    'BURST',
    'SPEED',
    'MAGAZINE',
    'RELOAD',
    'DPS',
  ];
  
  return wasmSortList[filterAndSortStrings.indexOf(sortType as any)] ?? undefined;
}

function sortPriorityJSToWasm(priority: string): string  {
  if (priority === 'Highest First') return 'HIGHEST';
  if (priority === 'Lowest First') return 'LOWEST';
  if (priority === 'Auto') return 'AUTO'
  return '';
}

function fpairObjectWasmToJS(wasmObject: fpair_WASM): fpair { 
  return [wasmObject["first"], wasmObject["second"]];
}

function gunObjectWasmToJS(wasmObject: Gun_WASM ): Gun { // Frees wasmObject 
  const g = new Gun();
  // This class will most likely contain static information and will be unchanging. (Very little methods.)
  g.barrel = wasmObject["barrel"];
  g.magazine = wasmObject["magazine"];
  g.grip = wasmObject["grip"];
  g.stock = wasmObject["stock"];
  g.core = wasmObject["core"];
    
  g.damage = [wasmObject["damage"], wasmObject["damageEnd"]];
  g.dropoffStuds = fpairObjectWasmToJS(wasmObject["dropoffStuds"]);
  g.falloffFactor = wasmObject["falloffFactor"];
  g.pellets = wasmObject["pellets"];
  g.burst = wasmObject["burst"];
  g.reloadTime = wasmObject["reloadTime"];
  g.magazineSize = wasmObject["magazineSize"];
  g.health = wasmObject["health"];
  g.fireRate = wasmObject["fireRate"];
  g.timeToAim = wasmObject["timeToAim"];
  g.equipTime = wasmObject["equipTime"];
  g.movementSpeedModifier = wasmObject["movementSpeedModifier"];
  g.hipfireSpread = wasmObject["hipfireSpread"];
  g.adsSpread = wasmObject["adsSpread"];
  g.detectionRadius = wasmObject["detectionRadius"];
  g.recoilHipHorizontal = fpairObjectWasmToJS(wasmObject["recoilHipHorizontal"]);
  g.recoilHipVertical = fpairObjectWasmToJS(wasmObject["recoilHipVertical"]);
  g.recoilAimHorizontal = fpairObjectWasmToJS(wasmObject["recoilAimHorizontal"]);
  g.recoilAimVertical = fpairObjectWasmToJS(wasmObject["recoilAimVertical"]);
  g.TTKM = wasmObject["TTKM"];
  g.TTKS = wasmObject["TTKS"];
  g.DPM = wasmObject["DPM"];
  g.DPS = wasmObject["DPS"];
  
  wasmObject["delete"]();
  return g;
}

class WggCalcManager {
  instance: EmbindModule_WASM | null;
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
      this.instance["InitDataset"](fullData_STRING);
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
    this.instance!["AddFilterRange"](title, type, min, max);
  }

  async addFilterForceBan(title: string, partsArray: string[]) {
    await this.ensureReady();
    
    // Convert JS Array to the Emscripten VectorString type you registered
    const vector = new this.instance!["VectorString"]();
    partsArray.forEach(p => vector["push_back"](p));
    
    this.instance!["AddFilterForceBan"](title, vector);
    
    // Clean up the vector memory in the Wasm heap
    vector["delete"]();
  }
  
  async addFilterRequired(total: number, sortType: string, sortPriority: string, categories: string[]) {
    await this.ensureReady();
    
    const vector = new this.instance!["VectorString"]();
    categories.forEach(p => vector["push_back"](p));
    
    this.instance!["AddFilterRequired"](total, sortType, sortPriority, vector);
    vector["delete"]();
  } 

  async clear() {
    await this.ensureReady();
    this.instance!["Clear"]();
  }
  
  async runCalc() {
    await this.ensureReady();
    const data = this.instance!["RunCalc"]();
    
    const results = [];
    for (let i = 0; i < data["size"](); i++) {
      results.push(gunObjectWasmToJS(data["get"](i)!));
    }
    
    data["delete"]();
    return results;
  }
  
  async getVersion() {
    await this.ensureReady();
    const v = this.instance!["GetVersion"]();
    console.log(`Running wggcalc ${v}`);
    return v;
  }
  
  // WASM API WRAPPERS //
}

// Export a single instance (The Singleton)
const WasmCalc = new WggCalcManager();

self.onmessage = async (e) => {
  const { action, data }: { action: string, data: any } = e.data;
  
  // I love reinventing redux for no reason
  switch (action) {
    case 'calculate':
      await handleCalculate(data);
      break;
    case 'queryVersion':
      await handleQueryVersion();
      break;
  }
}

async function handleQueryVersion() {
  const startTime = Date.now();
  try {
    const version = await WasmCalc.getVersion();
    postMessage({ success: true, startTime, endTime: Date.now(), data: version, action: 'queryVersion' });
  }
  catch (e) {
    postMessage({ success: false, startTime, endTime: Date.now(), data: e, action: 'queryVersion' });
    console.error(e)
  }
}

async function handleCalculate(filterList: FilterItem[]) {
  const startTime = Date.now();
  try {
    await WasmCalc.clear();
    
    // Find the required filters first
    const totalVal = filterList.find(f => f.title === "Total Results")?.writeable.value[0] as number;
    const sortType = sortTypeJSToWasm(filterList.find(f => f.title === "Sort Type")?.writeable.value[0] as string);
    const sortPriority = sortPriorityJSToWasm(filterList.find(f => f.title === "Sort Type")?.writeable.selectedOption as string);
    const categories = filterList.find(f => f.title === "Categories")?.writeable.value as string[];
    
    if (totalVal <= 0) throw new Error("Total Results must be greater than 0");
    if (sortType === undefined) throw new Error("Sort Type is invalid");
    if (sortPriority === undefined) throw new Error("Sort Priority is invalid");
    if (categories.length === 0) throw new Error("Categories must not be empty");
    
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
        
        const wasmTitle = sortTypeJSToWasm(filter.title);
        if (wasmTitle === undefined) continue;
        await WasmCalc.addFilterRange(wasmTitle, filter.writeable.selectedOption as string, minValue, maxValue);
      }
      else if (filter.filterType === "stringarr")
        await WasmCalc.addFilterForceBan(filter.title, filter.writeable.value as string[]);
    }
    const results = await WasmCalc.runCalc();
    postMessage({ success: true, startTime, endTime: Date.now(), data: results, action: 'calculate'}); // make sure it triggers the watch
  }
  catch (e) {
    postMessage({ success: false, startTime, endTime: Date.now(), data: e, action: 'calculate'});
    console.error(e)
  }
}

self.postMessage({ success: true, startTime: Date.now(), endTime: Date.now(), action: 'startup', data: null });

export type FilterType = 'string' | 'number' | 'stringarr' | 'numberrange' | 'sort';

export interface Filter {
  title: string;
  filterType: FilterType;
  validStrings?: string[]; // only on filterType === 'string'
  options?: string[];
  required?: boolean;
}

export interface FilterItem {
  id: number; // I think this can just be the index inside of the filter List
  title: string;
  filterType: FilterType;
  validStrings?: string[]; // only on filterType === 'string'
  options?: string[];
  required?: boolean;
  writeable: {
    selectedOption?: string;
    value: string[] | number[];
  };
}

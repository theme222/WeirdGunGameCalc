export const categoryStrings = [ "AR", "BR", "LMG", "SMG", "Shotgun", "Sidearm", "Sniper", "Weird" ] as const;

export const filterAndSortStrings = [
  "Burst",
  "DPS",
  "Damage End",
  "Damage Start",
  "Detection Radius",
  "Equip Time",
  "Fire Rate",
  "Health",
  "Magazine Size",
  "Pellets",
  "Range Stud End",
  "Range Stud Start",
  "Recoil Aim",
  "Recoil Hip",
  "Reload Time",
  "Speed",
  "Spread Aim",
  "Spread Hip",
  "TTK",
  "Time To Aim",
] as const;

// I tried going [...].sort() but I lose the const assertation so gonna have to just hard code this :/
export const filterTitles = [
  "Ban Barrel",
  "Ban Core",
  "Ban Grip",
  "Ban Magazine",
  "Ban Stock",
  "Burst",
  "Categories",
  "DPS",
  "Damage End",
  "Damage Start",
  "Detection Radius",
  "Equip Time",
  "Fire Rate",
  "Force Barrel",
  "Force Core",
  "Force Grip",
  "Force Magazine",
  "Force Stock",
  "Health",
  "Magazine Size",
  "Pellets",
  "Range Stud End",
  "Range Stud Start",
  "Recoil Aim",
  "Recoil Hip",
  "Reload Time",
  "Sort Type",
  "Speed",
  "Spread Aim",
  "Spread Hip",
  "TTK",
  "Time To Aim",
  "Total Results"
] as const;

export const numberRangeOptions = ['min', 'max', 'from'] as const;
export const sortTypeOptions = ['Auto', 'Highest First', 'Lowest First'] as const;
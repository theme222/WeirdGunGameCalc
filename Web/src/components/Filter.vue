<script setup>
import { ref, defineModel } from 'vue'
import { stringSearch } from '@/libs/search';
import { TrashIcon } from '@heroicons/vue/24/solid';

const props = defineProps(["title", "color", "currentlyRemoving", "removeCaller", "filterType"]);
const suggestions = ref([]);
const model = defineModel();

const words = [
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

function updateSuggestions(word) {
  const result = stringSearch(words, word, 5);
  suggestions.value = result.filter((item) => item.dist > 0).map((item) => item.value);
}

function handleInputChange(event) {
  updateSuggestions(event.target.value);
  model.value.value = event.target.value;
}
</script>

<template>
  <div
    class="w-full px-2 bg-base-100 flex justify-between items-center h-12 outline rounded-md"
    v-bind:class="[`text-${color}`, `outline-${color}`]"
  >
    <h3 class="text-lg">{{ title }}</h3>
    
    <div v-if="filterType === 'string'">
      <!-- STRING FILTER -->
      <div v-if="!currentlyRemoving" class="dropdown dropdown-end">
        <input
          type="text"
          class="input"
          :class="[`input-${color}`]"
          placeholder="None"
          :value="model.value"
          @input="handleInputChange"
          @change="handleInputChange"
        />
        <ul tabindex="-1" class="dropdown-content rounded-box z-1 w-52 p-2 shadow-sm text-ellipsis">
          <li
            v-for="word in suggestions"
            :key="word"
            class="cursor-pointer"
            @click="
              model.value = word;
              updateSuggestions(word);
            "
          >
            {{ word }}
          </li>
        </ul>
      </div>
      <div v-else>
        <button class="btn btn-sm btn-error" @click="removeCaller(model.value)">
          <TrashIcon class="size-4" />
          Remove
        </button>
      </div>
      <!-- STRING FILTER -->
    </div>
    
    <div v-else-if="filterType==='number'">
      <!-- NUMBER FILTER -->
      <TransitionGroup tag="div" name="list" class="flex justify-end items-center gap-2">
        <select class="select max-w-20 mr-4" v-model="model.filterOption">
          <option>min</option>
          <option>max</option>
          <option>from</option>
        </select>
        <input class="input input-bordered max-w-20" v-model="model.value[0]" type="number" />
        <p v-if="model.filterOption === 'from'">-</p>
        <input
          v-if="model.filterOption === 'from'"
          class="input input-bordered max-w-20"
          v-model="model.value[1]"
          type="number"
        />
      </TransitionGroup>
      <!-- NUMBER FILTER -->
    </div>
    
  </div>
</template>

<style scoped>
.list-move,
.list-enter-active,
.list-leave-active {
  transition: all 0.3s ease;
}

.list-leave-active {
  position: absolute;
}

.list-enter-from,
.list-leave-to {
  opacity: 0;
  transform: scale(0.5);
}
</style>

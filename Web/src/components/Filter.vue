<script setup>
import { ref, defineModel } from 'vue'
import { stringSearch } from '@/libs/search';
import { TrashIcon } from '@heroicons/vue/24/solid';
import { removeFilter } from '@/libs/filter';

const props = defineProps({
  id: Number,
  title: String,
  color: String,
  currentlyRemoving: Boolean,
  validStrings: { // Array of strings
    type: Array,
    default: () => []
  },
  removeCaller: Function,
  filterType: String
});

const suggestions = ref([]);
const model = defineModel();

function updateSuggestions(word) {
  console.log(props.validStrings)
  const result = stringSearch(props.validStrings, word, 5);
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
    
    <div v-if="currentlyRemoving" >
        <button class="btn btn-sm btn-error" @click="removeFilter(id)">
          <TrashIcon class="size-4" />
          Remove
        </button>
    </div>
    
    <div v-else-if="filterType === 'string'" class="dropdown dropdown-end">
      <!-- STRING FILTER -->
      <input
        type="text"
        class="input"
        :class="[`input-${color}`]"
        placeholder="None"
        :value="model.value"
        @input="handleInputChange"
        @change="handleInputChange"
      />
      <ul tabindex="-1" class="dropdown-content rounded-box z-1 w-52 p-2 shadow-sm text-ellipsis bg-base-100 ">
        <li
          v-for="word in suggestions"
          :key="word"
          class="cursor-pointer py-1 border-b border-white"
          @click="
            model.value = word;
            updateSuggestions(word);
          "
        >
          {{ word }}
        </li>
      </ul>
      <!-- STRING FILTER -->
    </div>
    
    <div v-else-if="filterType==='number'" class="dropdown dropdown-end">
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

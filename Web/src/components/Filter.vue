<script setup>
import { ref, defineModel } from 'vue';
import { stringSearch } from '@/libs/search';
import { TrashIcon } from '@heroicons/vue/24/solid';
import { removeFilter } from '@/libs/filter';
import { PlusCircleIcon } from '@heroicons/vue/24/outline';
import { addToast } from '@/libs/toast';
import SuggestionBox from './SuggestionBox.vue';

const props = defineProps({
  id: Number,
  title: String,
  color: String,
  currentlyRemoving: Boolean,
  validStrings: {
    // Array of strings
    type: Array,
    default: () => [],
  },
  inputValidator: Function, // returns boolean
  removeCaller: Function,
  filterType: String,
  required: Boolean,
  options: {
    type: Array,
    default: () => [],
  },
});

const tempInput = ref(''); // For stringarr input that isn't saved yet
const suggestions = ref([]);
const model = defineModel();
// {
//     selectedOption?: string; // Only in filterType === 'number'
//     value: string[] | number[];
// }

if (model.value.value.size != 0) {
  // pass
}
else if (props.filterType === 'stringarr') {
  model.value.value = [];
} else if (['string', 'number', 'sorting'].includes(props.filterType)) {
  model.value.value = [null];
} else if (props.filterType === 'numberrange') {
  model.value.value = [0, 0];
}

function updateSuggestions(word) {
  const result = stringSearch(props.validStrings, word, 5);
  suggestions.value = result.filter((item) => item.dist > 0).map((item) => item.value);
}

function handleInputChange(event) {
  updateSuggestions(event.target.value);
}

function addInputToList() {
  if (!tempInput.value) return; // undefined null or empty string

  if (!props.validStrings.includes(tempInput.value)) {
    addToast('Invalid filter to add', 'error');
    return;
  }

  if (model.value.value.includes(tempInput.value)) {
    addToast('Duplicate filter', 'warning');
    return;
  }

  model.value.value.push(tempInput.value);
}

function removeInputFromList(index) {
  model.value.value.splice(index, 1);
}
</script>

<template>
  <div
    class="w-full px-2 bg-base-100 flex justify-center items-center h-12 outline rounded-md gap-5"
    v-bind:class="[`text-${color}`, `outline-${color}`]"
  >
    <h3 class="text-lg text-nowrap shrink-0">{{ title }}</h3>

    <TransitionGroup tag="div" name="list" class="flex flex-1 min-w-0">
      <!-- CURRENTLY REMOVING -->
      <div v-if="currentlyRemoving && !required" class="flex justify-end min-w-0 w-full">
        <button class="btn btn-sm btn-error" @click="removeFilter(id)">
          <TrashIcon class="size-4" />
          Remove
        </button>
      </div>
      <!-- CURRENTLY REMOVING -->

      <!-- STRING FILTER -->
      <div v-else-if="filterType === 'string'" class="dropdown dropdown-end w-full min-w-0 flex justify-end">
        <input
          type="text"
          class="input"
          :class="[`input-${color}`]"
          placeholder="None"
          @input="handleInputChange"
          @keyup.enter="model.value[0] = suggestions[0]"
          v-model="model.value[0]"
        />
        <SuggestionBox
          :suggestions="suggestions"
          :onSelectSuggestion="(selected) => (model.value[0] = selected)"
          :updateSuggestions="updateSuggestions"
        />
      </div>
      <!-- STRING FILTER -->

      <!-- NUMBER FILTER -->
      <div v-else-if="filterType === 'number'" class="w-full min-w-0 flex justify-end">
        <input
          type="number"
          class="input w-fit"
          :class="[`input-${color}`]"
          placeholder="None"
          @input="handleInputChange"
          v-model="model.value[0]"
          step="1"
          min="1"
        />
      </div>
      <!-- NUMBER FILTER -->

      <!-- STRINGARR FILTER -->
      <div v-else-if="filterType === 'stringarr'" class="flex justify-end items-center gap-2 w-full min-w-0">
        <div class="overflow-x-auto flex items-center min-w-0 flex-1">
          <button
            v-for="(str, index) in model.value"
            class="btn btn-ghost shrink-0"
            :class="{ 'ml-auto': index === 0 }"
            @click="removeInputFromList(index)"
          >
            {{ str }}
          </button>
        </div>
        <div class="dropdown dropdown-end shrink-0">
          <input
            type="text"
            class="input w-40"
            :class="[`input-${color}`]"
            placeholder="None"
            @input="handleInputChange"
            @keyup.enter="tempInput = suggestions[0]; addInputToList()"
            v-model="tempInput"
          />
          <SuggestionBox
            :suggestions="suggestions"
            :onSelectSuggestion="(selected) => (tempInput = selected)"
            :updateSuggestions="updateSuggestions"
          />
        </div>
        <button @click="addInputToList" class="btn btn-ghost btn-sm btn-circle shrink-0">
          <PlusCircleIcon class="size-6" />
        </button>
      </div>
      <!-- STRINGARR FILTER -->

      <!-- NUMBER RANGE FILTER -->
      <div v-else-if="filterType === 'numberrange'" class="flex justify-end items-center gap-2 min-w-0 w-full">
        <select class="select max-w-20 mr-4" v-model="model.selectedOption">
          <option v-for="opt in options" :key="opt">{{ opt }}</option>
        </select>
        <input class="input input-bordered max-w-20" v-model="model.value[0]" type="number" step="0.1" />
        <p v-if="model.selectedOption === 'from'">-</p>
        <input
          v-if="model.selectedOption === 'from'"
          class="input input-bordered max-w-20"
          v-model="model.value[1]"
          type="number"
          step="0.1"
        />
      </div>
      <!-- NUMBER RANGE FILTER -->

      <!-- SORT FILTER -->
      <div v-else-if="filterType === 'sort'" class="flex justify-end items-center gap-2 w-full min-w-0">
        <select class="select max-w-35 mr-4" v-model="model.selectedOption">
          <option v-for="opt in options" :key="opt">{{ opt }}</option>
        </select>
        <div class="dropdown dropdown-end">
          <input
            type="text"
            class="input max-w-40"
            :class="[`input-${color}`]"
            placeholder="None"
            @input="handleInputChange"
            @keyup.enter="model.value[0] = suggestions[0]"
            v-model="model.value[0]"
          />
          <SuggestionBox
            :suggestions="suggestions"
            :onSelectSuggestion="(selected) => (model.value[0] = selected)"
            :updateSuggestions="updateSuggestions"
          />
        </div>
      </div>
      <!-- SORT FILTER -->
    </TransitionGroup>
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

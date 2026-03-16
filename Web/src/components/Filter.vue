<script setup>
import { ref } from 'vue';
import { stringSearch } from '@/libs/search';
import { TrashIcon } from '@heroicons/vue/24/solid';
import { removeFilter } from '@/libs/filter';
import { PlusCircleIcon } from '@heroicons/vue/24/outline';
import { addToast } from '@/libs/toast';
import SuggestionBox from './SuggestionBox.vue';
import NumberInput from './NumberInput.vue';
import StringInput from './StringInput.vue';


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
  removeCaller: Function,
  filterType: String,
  required: Boolean,
  options: {
    type: Array,
    default: () => [],
  },
});

const tempInput = ref(''); // For stringarr input that isn't saved yet
const model = defineModel();
// {
//     selectedOption?: string; // Only in filterType === 'number'
//     value: string[] | number[];
// }

if (model.value.value.length != 0) {
  // pass
} else if (props.filterType === 'stringarr') {
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
    <h3 class="md:text-lg text-nowrap shrink-0">{{ title }}</h3>

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
      <div v-else-if="filterType === 'string'" class="w-full min-w-0 flex justify-end">
        <StringInput
          v-model="model.value[0]"
          :validStrings="validStrings"
          :color="color"
        />
      </div>
      <!-- STRING FILTER -->

      <!-- NUMBER FILTER -->
      <div v-else-if="filterType === 'number'" class="w-full min-w-0 flex justify-end">
        <NumberInput
          v-model="model.value[0]"
          :color="color"
          :step="1"
          :scrollStep="1"
          :min="0"
        />
      </div>
      <!-- NUMBER FILTER -->

      <!-- STRINGARR FILTER -->
      <div v-else-if="filterType === 'stringarr'" class="flex justify-end items-center gap-2 w-full min-w-0">
        <div class="overflow-x-auto flex items-center min-w-0 flex-1">
          <button
            v-for="(str, index) in model.value"
            class="btn btn-xs md:btn-md btn-ghost shrink-0"
            :class="{ 'ml-auto': index === 0 }"
            @click="removeInputFromList(index)"
          >
            {{ str }}
          </button>
        </div>
        <StringInput
          v-model="tempInput"
          :validStrings="validStrings"
          :onEnter="addInputToList"
          :color="color"
        />
        <button @click="addInputToList" class="btn btn-ghost btn-sm btn-circle shrink-0">
          <PlusCircleIcon class="size-6" />
        </button>
      </div>
      <!-- STRINGARR FILTER -->

      <!-- NUMBER RANGE FILTER -->
      <div v-else-if="filterType === 'numberrange'" class="flex justify-end items-center gap-2 min-w-0 w-full">
        <select class="select select-xs sm:select-md max-w-20 mr-4" v-model="model.selectedOption">
          <option v-for="opt in options" :key="opt">{{ opt }}</option>
        </select>
        <NumberInput
          class="max-w-20"
          v-model="model.value[0]"
          :color="color"
          :step="0.001"
          :scrollStep="0.1"
        />
        <p v-if="model.selectedOption === 'from'">-</p>
        <NumberInput
          class="max-w-20"
          v-if="model.selectedOption === 'from'"
          v-model="model.value[1]"
          :color="color"
          :step="0.001"
          :scrollStep="0.1"
        />
      </div>
      <!-- NUMBER RANGE FILTER -->

      <!-- SORT FILTER -->
      <div v-else-if="filterType === 'sort'" class="flex justify-end items-center gap-2 w-full min-w-0">
        <select class="select select-xs sm:select-md max-w-35 mr-4" v-model="model.selectedOption">
          <option v-for="opt in options" :key="opt">{{ opt }}</option>
        </select>
        <StringInput
          v-model="model.value[0]"
          :validStrings="validStrings"
          :color="color"
        />
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

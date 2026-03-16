<script setup>
import { ref } from 'vue';
import SuggestionBox from './SuggestionBox.vue';
import { stringSearch } from '@/libs/search.js';

const model = defineModel();
const props = defineProps({
  color: String,
  validStrings: {
    type: Array,
    default: () => [],
  },
  onEnter: {
    type: Function,
    default: () => {},
  },
});

const inputRef = ref(null);
const suggestions = ref([]);

function updateSuggestions(word) {
  const result = stringSearch(props.validStrings, word, 5);
  suggestions.value = result.filter((item) => item.dist > 0).map((item) => item.value);
}

function handleInputChange(event) {
  updateSuggestions(event.target.value);
}

</script>

<template>
<div class="dropdown dropdown-end shrink-0">
  <input
    type="text"
    :class="[`input input-xs sm:input-md input-${color}`]"
    placeholder="None"
    @input="handleInputChange"
    @keyup.enter="model = suggestions[0]; onEnter()"
    v-model="model"
  />
  <SuggestionBox
    :suggestions="suggestions"
    :onSelectSuggestion="(selected) => (model = selected)"
    :updateSuggestions="updateSuggestions"
  />
</div>
</template>
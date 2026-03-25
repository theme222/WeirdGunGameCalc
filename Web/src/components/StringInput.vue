<script setup>
import { ref } from 'vue';
import SuggestionBox from './SuggestionBox.vue';
import { stringSearch } from '@/libs/search.js';
import { onMounted } from 'vue';

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
const suggestionIndex = ref(0);

function updateSuggestions(word) {
  let result = stringSearch(props.validStrings, word, 999);
  if (word.length !== 0) result = result.filter((item) => item.dist > 0);
  suggestions.value = result.map(item => item.value); 
}

function handleInputChange(event) {
  updateSuggestions(event.target.value);
}

onMounted(() => {
  updateSuggestions('');
})

</script>

<template>
<div class="dropdown dropdown-end shrink">
  <input
    type="text"
    ref="inputRef"
    :class="[`input input-xs sm:input-md input-${color}`]"
    placeholder="None"
    @input="handleInputChange"
    @keyup.enter="() => {model = suggestions[suggestionIndex]; onEnter()}"
    @keyup.down="suggestionIndex = Math.min(suggestionIndex + 1, suggestions.length - 1)"
    @keyup.up="suggestionIndex = Math.max(suggestionIndex - 1, 0)"
    v-model="model"
  />
  <SuggestionBox
    :suggestions="suggestions"
    :onSelectSuggestion="(selected) => {model = selected; inputRef.focus()}"
    :updateSuggestions="updateSuggestions"
    v-model="suggestionIndex"
  />
</div>
</template>
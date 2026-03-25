<script setup>
import { ref, watch } from 'vue';

const currentIndex = defineModel();
const suggestionsListRef = ref(null);

const props = defineProps({
  suggestions: {
    type: Array,
    required: true
  },
  updateSuggestions: {
    type: Function,
    required: true
  },
  onSelectSuggestion: {
    type: Function,
    required: true
  }
})

watch(currentIndex, () => {
  // Handle currentIndex changing by scrolling to the selected item.
  suggestionsListRef.value.children[currentIndex.value]?.scrollIntoView({ behavior: 'smooth', block: 'nearest' })
})

</script>

<template>
<ul ref="suggestionsListRef" v-if="suggestions.length > 0" tabindex="-1" class="dropdown-content rounded-box z-30 w-52 shadow-sm text-ellipsis bg-base-100 border max-h-52 overflow-y-scroll">
  <li
    v-for="(word, index) in suggestions"
    :key="word"
    :class="{ 'font-black bg-primary': index === currentIndex, 'bg-base-100': index % 2 == 0, 'bg-base-300': index % 2 == 1 }"
    class="cursor-pointer py-2 px-4 hover:font-bold text-xs sm:text-base"
    @click="
      onSelectSuggestion(word);
      updateSuggestions(word);
    "
    @mouseenter="() => currentIndex = index"
  >
    {{ word }}
  </li>
</ul>
</template>
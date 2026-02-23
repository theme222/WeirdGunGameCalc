<script setup>
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
</script>

<template>
<ul v-if="suggestions && suggestions.length > 0" tabindex="-1" class="dropdown-content rounded-box z-1 w-52 p-2 shadow-sm text-ellipsis bg-base-100 ">
  <li
    v-for="word in suggestions"
    :key="word"
    class="cursor-pointer py-1 border-b border-white"
    @click="
      onSelectSuggestion(word);
      updateSuggestions(word);
    "
  >
    {{ word }}
  </li>
</ul>
</template>
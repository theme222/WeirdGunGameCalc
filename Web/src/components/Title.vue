
<script setup>
const props = defineProps({
  title: String,
  subtitle: String
});
</script>

<template>
<div class="flex flex-col items-center w-full py-8 gap-1">
  <h1 class="font-bold text-3xl text-white rounded">{{ title }}</h1>
  <h2 class="opacity-50 text-sm text-center">{{ subtitle }}</h2>
</div>
</template>
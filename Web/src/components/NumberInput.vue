<script setup>
import { ref, watch } from 'vue';

const model = defineModel();
const props = defineProps({
  color: String,
  step: Number, // true step value
  scrollStep: Number, // step value for scroll or arrow
  min: {
    type: Number,
    optional: true
  },
  max: {
    type: Number,
    optional: true
  }
});

const inputRef = ref(null);
</script>

<template>
<input
  type="number"
  :class="[`input input-xs sm:input-md input-${color}`]"
  v-model.number="model"
  :step="step"
  :min="min"
  :max="max"
  @wheel.prevent="(event) => {
    const nextValue = parseFloat(model) + (scrollStep * -Math.sign(event.deltaY));
    model = parseFloat(nextValue.toFixed(3));
  }"
/>
</template>
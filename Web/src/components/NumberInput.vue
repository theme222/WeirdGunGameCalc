<script setup>
import { ArrowsPointingOutIcon } from '@heroicons/vue/24/outline';
import { useSlider } from '@/libs/slider';
import { onMounted, ref, watch } from 'vue';
import { ffvNumber } from '@/libs/result';

const model = defineModel();
const props = defineProps({
  color: String,
  step: Number, // true step value
  scrollStep: Number, // step value for scroll
  min: {
    type: Number,
    optional: true
  },
  max: {
    type: Number,
    optional: true
  }
});

function formatValue(value) {
  value = Math.min(Math.max(value, props.min ?? -Infinity), props.max ?? Infinity);
  value = Math.floor(value / props.scrollStep) * props.scrollStep;
  return ffvNumber(value);
}

const sliderOrigin = ref(null);

onMounted(() => {
  const { deltaX, isDragging } = useSlider(sliderOrigin);
  
  let startVal = 0;
  
  watch(isDragging, (dragging) => {
    if (dragging) {
      startVal = model.value;
      document.body.classList.add('cursor-ew-resize-lock');
    }
    else {
      document.body.classList.remove('cursor-ew-resize-lock');
    }
  });
  
  watch(deltaX, (delta) => {
    if (!delta) return;
    model.value = formatValue(startVal + (delta * props.scrollStep * 0.1));
  });
});

</script>

<template>
<label
  :class="[`input input-xs sm:input-md input-${color}`]"
  for="numberInput"
>
  <input
    type="number"
    name="numberInput"
    v-model.number="model"
    :step="step"
    :min="min"
    :max="max"
    @wheel.prevent="(event) => {
      const nextValue = parseFloat(model) + (scrollStep * -Math.sign(event.deltaY));
      model = parseFloat(nextValue.toFixed(3));
    }"
  />
  
  <ArrowsPointingOutIcon 
    class="size-5 cursor-ew-resize" 
    ref="sliderOrigin"
  />
</label>
</template>
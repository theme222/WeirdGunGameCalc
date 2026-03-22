<script setup>
import { currentSortingType, statsToCompare } from '@/libs/result';
import { ChevronDownIcon, ChevronUpIcon } from '@heroicons/vue/24/outline';
import { ref, toRaw } from 'vue';
import { getFilterAsProperty, ffv } from '@/libs/result';

const collapseVal = ref(false);

const props = defineProps({
  id: Number,
  gun: Object 
});

const colorMap = {
  'barrel': 'accent',
  'magazine': 'secondary',
  'grip': 'warning',
  'stock': 'info',
  'core': 'primary',
};

console.log(statsToCompare)

</script>

<template>
<div class="rounded-md bg-base-100 border p-3 overflow-y-hidden w-full" :class=" collapseVal ? 'h-fit' : 'h-14'">
  <input type="checkbox" class="hidden" :name="`collapse-val-${props.id}`" :id="`collapse-val-${props.id}`" v-model="collapseVal" />
  <label class="font-semibold flex justify-between items-center pb-2 gap-8 cursor-pointer relative" :for="`collapse-val-${props.id}`">
    <div class="flex items-center gap-2 shrink-0 ">
      <div class="text-2xl font-light opacity-30 tabular-nums">{{props.id + 1}}</div>
      <div class="font-black rounded-md tooltip tooltip-right md:text-lg mr-4" data-tip="sort">{{ currentSortingType }}: {{ ffv(getFilterAsProperty(toRaw(props.gun), currentSortingType)) }}</div>
      <div 
        class="font-black rounded-md tooltip tooltip-right text-sm flex items-center"
        v-for="[key, value] in Object.entries(colorMap)" 
        :key="key" 
        :class="`text-${value} tooltip-${value}`" 
        :data-tip="key"
      >
        <!-- <p class="status bg-white status-sm mr-2 hidden md:inline-block"/> -->
        <div>{{ props.gun[key] }}</div>
      </div>
    </div>
    <div class="sticky right-0 flex justify-end items-center size-5">
      <ChevronDownIcon class="size-5" v-if="collapseVal" />
      <ChevronUpIcon class="size-5" v-else />
    </div>
  </label>
  <ul class="list-disc px-4">
    <div v-for="stat in statsToCompare">
      <li v-if="stat.show">
          {{ stat.stat }}: {{ ffv(getFilterAsProperty(toRaw(props.gun), stat.stat)) }}
      </li>
    </div>
  </ul>
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
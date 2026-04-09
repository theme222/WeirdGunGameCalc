<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue'

const width = ref(window.innerWidth)

const breakpoints = [
  { name: '2xl', min: 1536 },
  { name: 'xl',  min: 1280 },
  { name: 'lg',  min: 1024 },
  { name: 'md',  min: 768 },
  { name: 'sm',  min: 640 },
  { name: 'xs',  min: 0 },
]

const current = computed(() => {
  return breakpoints.find(bp => width.value >= bp.min)
})

const onResize = () => {
  width.value = window.innerWidth
}

onMounted(() => {
  window.addEventListener('resize', onResize)
})

onUnmounted(() => {
  window.removeEventListener('resize', onResize)
})
</script>

<template>
  <div
    class="fixed bottom-3 right-3 z-50 rounded-md bg-black/80 px-3 py-1.5
           text-xs font-mono text-white shadow-lg"
  >
    <span class="text-gray-300">{{ width }}px</span>
    <span class="mx-1">•</span>
    <span class="font-semibold text-emerald-400">
      {{ current.name }}
    </span>
  </div>
</template>
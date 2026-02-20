<script setup>
import { toastState } from "@/libs/toast"
import Toast from "@/components/Toast.vue"
</script>


<template>
  <div class="toast toast-top toast-end">
    <Toast
      v-for="t in toastState.toasts"
      :key="t.id"
      v-bind="t"
    />
  </div>
</template>

<style scoped>
</style>
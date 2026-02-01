<script setup>
import { useRouter } from 'vue-router'
import Title from './components/Title.vue';
import ViewportIndicator from './components/ViewportIndicator.vue';

const isDev = import.meta.env.DEV;

</script>

<template>
<!-- <div id="background" class="fixed w-full -z-20 h-full bg-base-300"></div> -->
  <nav>
    <Title />
  </nav>
  <main>
    <RouterView />
  </main>
  <ViewportIndicator v-if="isDev"/>
</template>

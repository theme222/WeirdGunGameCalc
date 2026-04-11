<script setup>
import { drawerOpen } from '@/libs/drawer.ts'
import { useRouter } from 'vue-router';
import { Bars3Icon } from '@heroicons/vue/24/solid';
import { ensureURLCorrectness } from '@/libs/filter';

const router = useRouter();

function pushKeepQuery(url) {
  const currentQuery = new URLSearchParams(window.location.search);
  router.push(ensureURLCorrectness(`${url}?${currentQuery.toString()}`));
}

</script>

<template>
<!-- I don't really wanna use daisyuis version -->
<button @click="drawerOpen = !drawerOpen" class="fixed top-2 left-2 z-51 cursor-pointer">
  <Bars3Icon class="size-8" />
</button>
<div class="fixed top-0 left-0 max-w-60 h-full bg-base-300/95 transition-transform duration-300 z-50 flex flex-col items-center gap-2 pt-12" :class="{ '-translate-x-60': !drawerOpen }">
  <button class="w-full cursor-pointer hover:bg-base-100 py-2 px-10 transition-colors duration-200" @click="pushKeepQuery('/calc')"> Calculator </button>
  <button class="w-full cursor-pointer hover:bg-base-100 py-2 px-10 transition-colors duration-200" @click="pushKeepQuery('/part')"> Part Query </button>
</div>
<!-- <div class="fixed top-0 left-0 w-screen h-screen z-49" :class="{ 'pointer-events-none': !drawerOpen }" @click="drawerOpen = false"></div> -->
</template>
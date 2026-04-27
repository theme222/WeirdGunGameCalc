<script setup>
import Title from '@/components/Title.vue'
import { MagnifyingGlassIcon } from '@heroicons/vue/24/outline';
import StringInput from '@/components/StringInput.vue'
import { PARTCORELIST, PARTNAMES } from '@/libs/data';
import { ref, reactive } from 'vue';

const currentQuery = ref('');
const results = reactive({
  Barrel: null,
  Magazine: null,
  Grip: null,
  Stock: null,
  Core: null,
});

function queryPart() {
  results.Barrel = PARTCORELIST.Barrels.find((barrel) => barrel.Name === currentQuery.value) || null;
  results.Magazine = PARTCORELIST.Magazines.find((magazine) => magazine.Name === currentQuery.value) || null;
  results.Grip = PARTCORELIST.Grips.find((grip) => grip.Name === currentQuery.value) || null;
  results.Stock = PARTCORELIST.Stocks.find((stock) => stock.Name === currentQuery.value) || null;
  results.Core = PARTCORELIST.Cores.find((core) => core.Name === currentQuery.value) || null;
  console.log(results);
}

function getResultsListComputed() {
  const kvResults = Object.entries(results);
  return kvResults.filter(([_, value]) => value !== null);
}

</script>

<template>
<div class="flex flex-col items-center w-full py-8 px-4 gap-1">
  <h1 class="font-bold text-3xl text-white rounded">Part Query</h1>
  <h2 class="opacity-50 text-sm text-center">Double check part stats here ( If they are different head over to the <a href="https://docs.google.com/spreadsheets/d/1Kc9aME3xlUC_vV5dFRe457OchqUOrwuiX_pQykjCF68/edit?gid=1702472906#gid=1702472906" class="link">sheet</a> and comment the issue there )</h2>
</div>

<main class="w-full flex justify-center items-start gap-6 flex-wrap mb-5">
  
  <div class="flex flex-col justify-center items-center gap-5">
    
    <div class="flex justify-center items-center gap-2">
      <StringInput :validStrings="PARTNAMES" :onEnter="queryPart" v-model="currentQuery" :onSelectSuggestion="queryPart"/>
      <button class="btn btn-info text-white btn-xs sm:btn-md sm:btn-square" @click="queryPart"><MagnifyingGlassIcon class="size-5 hidden sm:inline"/><span class="inline sm:hidden">search</span></button>
    </div>
    
    <div v-if="getResultsListComputed().length > 0" class="flex flex-col gap-5 p-2 border bg-base-200 min-h-20 max-w-[95vw] flex-wrap rounded-md">
      <div v-for="[type, part] in getResultsListComputed()" :key="type" class="flex flex-col p-5 border bg-base-200 rounded-md">
        <h3 class="text-xl mb-2 font-bold">{{ type }}</h3>
        <ul class="grid grid-cols-2 list-disc px-3 gap-x-8">
          <li v-for="[key, value] in Object.entries(part)" :key="key" >{{ key }}: {{ value }}</li>
        </ul>
      </div>
    </div>
    
  </div>
  
</main>
</template>
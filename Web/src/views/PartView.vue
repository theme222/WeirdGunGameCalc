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
<Title :title="'Part Query'" :subtitle="'Double check if any parts in the dataset are different from in game'"/>
<main class="w-full flex justify-center items-start gap-6 flex-wrap mb-5">
  
  <div class="flex flex-col justify-center items-center gap-5">
    
    <div class="flex justify-center items-center gap-2">
      <StringInput :validStrings="PARTNAMES" :onEnter="queryPart" v-model="currentQuery"/>
      <button class="btn btn-info btn-square text-white" @click="queryPart"><MagnifyingGlassIcon class="size-5"/></button>
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
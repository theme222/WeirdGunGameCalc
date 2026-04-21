<script setup>
import { ref, markRaw, onMounted } from 'vue';
import Filter from '@/components/Filter.vue';
import Result from '@/components/Result.vue';
import HelpModal from '@/components/HelpModal.vue';
import { SquaresPlusIcon } from '@heroicons/vue/24/solid';
import { TrashIcon as OutlineTrashIcon, NumberedListIcon } from '@heroicons/vue/24/outline';
import { addFilter, removeFilter, filterList, currentFilters } from '@/libs/filter';
import { runCalc, runOnFilterChange } from '@/libs/calc';
import { latestResult, isCalculating } from '@/libs/calc';
import { statsToCompare } from '@/libs/result';
import { getFiltersFromURL } from '@/libs/filter';
import StringInput from '@/components/StringInput.vue';
import { filterTitles } from '@/libs/filter.const';
import Title from '@/components/Title.vue';


const selectedFilterToAdd = ref('');
const currentlyRemoving = ref(false);

function toggleRemoveFilter() {
  currentlyRemoving.value = !currentlyRemoving.value;
}

onMounted(() => {
  currentFilters.list = getFiltersFromURL().list; // This is incase we go to and from the calc page
})

</script>

<template>

<div class="flex flex-col items-center w-full py-8 gap-1">
  <h1 class="font-bold text-3xl text-white rounded">WGGCALC</h1>
  <h2 class="opacity-50 text-sm text-center">Lets go ruin the loadouts channel for the rest of eternity</h2>
</div>

<main class="w-full flex justify-center items-start gap-6 flex-wrap mb-5">
  
  <div class="flex flex-col justify-center items-center gap-5 ">
    <div class="bg-base-200 border rounded-md min-h-80 w-180 p-3 max-w-[95vw]">
      <div class="flex items-center justify-between mb-3">
        <h2 class="font-semibold text-2xl">Filter</h2>
        <div class="flex justify-center items-center gap-6">
          <StringInput 
            v-model="selectedFilterToAdd" 
            :validStrings="filterTitles" 
            :onEnter="() => addFilter(selectedFilterToAdd)" 
            :onSelectSuggestion="addFilter"
            :placeholder="'Type in a filter here...'"/>
          <button class="btn btn-success btn-sm sm:btn-md" @click="addFilter(selectedFilterToAdd)">
            <SquaresPlusIcon class="size-6" />
            <span
              class="absolute -z-10 opacity-0 sm:z-0 sm:static sm:opacity-100 transition-opacity duration-400"
              >Add Filter</span
            >
          </button>
          <button
            class="btn btn-circle btn-sm sm:btn-md"
            :class="{ 'btn-error': currentlyRemoving, 'btn-ghost': !currentlyRemoving }"
            @click="toggleRemoveFilter"
          >
            <OutlineTrashIcon class="size-6" />
          </button>
        </div>
      </div>

      <TransitionGroup
        name="list"
        tag="div"
        class="w-full flex flex-col justify-center items-center gap-3 "
      >
        <Filter
          v-for="filter in currentFilters.list"
          class="w-full"
          :id="filter.id"
          :key="filter.id"
          :title="filter.title"
          :currentlyRemoving="currentlyRemoving"
          :removeCaller="removeFilter"
          :validStrings="filter.validStrings"
          :filterType="filter.filterType"
          :required="filter.required || false"
          :options="filter.options"
          v-model="filter.writeable"
        />
      </TransitionGroup>
    </div>
    
    <div class="w-full flex justify-center items-center gap-5">
      <button class="btn btn-primary btn-sm sm:btn-md" @click="runCalc">
        <span class="hidden sm:inline">CALCULATE</span>
        <span class="inline sm:hidden">RUN</span>
      </button>
      <label for="updateOnChange" class="flex justify-center items-center gap-3">
        <input type="checkbox" id="updateOnChange" name="updateOnChange" v-model="runOnFilterChange" class="checkbox"/>
        <span class="text-sm">Auto Update On Change</span>
      </label>
    </div>
  </div>
      
  <div class="flex flex-col justify-center items-center max-w-200">
    <div class="bg-base-200 border rounded-md max-w-[95vw] min-w-0 w-180 shrink h-171 px-3 overflow-y-scroll relative">
      <div class="flex justify-between items-center sticky top-0 bg-base-200 py-4 z-2">
        <h2 class="font-semibold text-xl sm:text-2xl w-full">Results ({{ latestResult.success ? latestResult.data.length : 0 }}) <span class="opacity-50 text-sm ml-2">click the results to view details</span></h2>
        <details class="dropdown dropdown-end">
          <summary class="btn btn-soft btn-warning m-1">
            <span class="hidden sm:block text-nowrap">Edit Stat Visibility</span>
            <NumberedListIcon class="size-4 block sm:hidden" />
          </summary>
          <div class="dropdown-content bg-base-200 rounded-box w-52 h-120 overflow-y-auto">
            <div class="flex flex-col gap-2 p-3">
              <label v-for="(stat, index) in statsToCompare" :key="index" class="flex items-center gap-2 cursor-pointer" :for="stat.stat">
                <input type="checkbox" :id="stat.stat" :value="stat.stat" class="checkbox" v-model="stat.show" />
                <span class="text-sm">{{ stat.stat }}</span>
              </label>
            </div>
          </div>
        </details>
        
      </div>
      <div v-if="!isCalculating">
        <TransitionGroup
          name="list"
          tag="div"
          class="w-full overflow-y-auto flex flex-col items-center gap-3 mb-3 relative"
          v-if="latestResult.success && latestResult.data.length > 0"
        >
          <Result 
            v-for="(gun, index) in latestResult.data" 
            :gun="markRaw(gun)"
            :key="index"
            :id="index"
          />
        </TransitionGroup>
        <div v-else class="w-full h-150 flex justify-center items-center">
          <img src="/noguns.jpg" class="w-60 h-70 opacity-5 select-none" />
        </div>
      </div>
      <div v-else class="w-full overflow-y-auto flex flex-col items-center gap-3 mb-3" >
        <div v-for="(_, index) in [...Array(10)]" class="skeleton h-12 w-full" :key="index"></div>
      </div>
    </div>

  </div>
  
</main>

<HelpModal />
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

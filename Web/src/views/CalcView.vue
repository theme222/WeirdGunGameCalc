<script setup>
import { ref, markRaw } from 'vue';
import Filter from '@/components/Filter.vue';
import Result from '@/components/Result.vue';
import { SquaresPlusIcon } from '@heroicons/vue/24/solid';
import { TrashIcon as OutlineTrashIcon } from '@heroicons/vue/24/outline';
import { addFilter, removeFilter, filterList, currentFilters } from '@/libs/filter';
import { PARTCORELIST } from '@/libs/data';
import { runCalc, runOnFilterChange } from '@/libs/calc';
import { latestResult, isCalculating } from '@/libs/calc';
import { statsToCompare } from '@/libs/result';

const selectedFilterToAdd = ref(filterList[0]);
const currentlyRemoving = ref(false);

function toggleRemoveFilter() {
  currentlyRemoving.value = !currentlyRemoving.value;
}

</script>

<template>
  <main class="w-full flex justify-center items-start gap-6 flex-wrap mb-5">
    
    <div class="flex flex-col justify-center items-center gap-5 ">
      <div class="bg-base-200 border rounded-md min-h-80 w-180 mx-10 p-3 max-w-[95vw]">
        <div class="flex items-center justify-between mb-3">
          <h2 class="font-semibold text-2xl">Filter</h2>
          <div class="flex justify-center items-center gap-6">
            <select class="select min-w-20" v-model="selectedFilterToAdd">
              <option v-for="filter in filterList" :key="filter.title" :value="filter">
                {{ filter.title }}
              </option>
            </select>
            <button class="btn btn-success" @click="addFilter(selectedFilterToAdd)">
              <SquaresPlusIcon class="size-6" />
              <span
                class="absolute -z-10 opacity-0 sm:z-0 sm:static sm:opacity-100 transition-opacity duration-400"
                >Add Filter</span
              >
            </button>
            <button
              class="btn btn-circle"
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
        <button class="btn btn-primary" @click="runCalc">LEMME SEE THEM GUNS</button>
        <label for="updateOnChange" class="flex justify-center items-center gap-3">
          <input type="checkbox" id="updateOnChange" name="updateOnChange" v-model="runOnFilterChange" class="checkbox"/>
          <span>Auto Update On Change</span>
        </label>
      </div>
    </div>
        
    <div class="flex flex-col justify-center items-center max-w-200 mx-4">
      <div class="bg-base-200 border rounded-md max-w-[95vw] min-w-0 w-180 shrink h-191 px-3 overflow-y-scroll relative">
        <div class="flex justify-between items-center sticky top-0 bg-base-200 py-4 z-2">
          <h2 class="font-semibold text-2xl w-full">Results ({{ latestResult.success ? latestResult.data.length : 0 }})</h2>
          <details class="dropdown dropdown-end">
            <summary class="btn btn-soft m-1 w-39">Stats to Compare</summary>
            <div class="dropdown-content bg-base-200 rounded-box w-52">
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
            class="w-full overflow-y-auto flex flex-col items-center gap-3 mb-3"
            v-if="latestResult.success"
          >
            <Result 
              v-for="(gun, index) in latestResult.data" 
              :gun="markRaw(gun)"
              :key="index"
              :id="index"
            />
          </TransitionGroup>
        </div>
        <div v-else class="w-full overflow-y-auto flex flex-col items-center gap-3 mb-3" >
          <div v-for="(_, index) in [...Array(10)]" class="skeleton h-14 w-full" :key="index"></div>
        </div>
      </div>

    </div>
    
  </main>
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

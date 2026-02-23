<script setup>
import { ref, reactive, toRef } from 'vue';
import Filter from '@/components/Filter.vue';
import { SquaresPlusIcon, TrashIcon } from '@heroicons/vue/24/solid';
import { TrashIcon as OutlineTrashIcon } from '@heroicons/vue/24/outline';
import { addToast } from '@/libs/toast';
import { addFilter, removeFilter, filterList, currentFilters } from '@/libs/filter';

const selectedFilterToAdd = ref(filterList[0]);
const currentlyRemoving = ref(false);

function toggleRemoveFilter() {
  currentlyRemoving.value = !currentlyRemoving.value;
}

function submitQuery() {
  
}

</script>

<template>
  <main class="grid grid-cols-1 gap-5 w-full">
    
    <div class="flex w-full justify-center items-center">
      <div class="bg-base-200 border rounded-md w-full max-w-200 min-h-80 mx-10">
        <div class="flex items-center justify-between">
          <h2 class="font-semibold ml-3 my-5 text-2xl">Filter</h2>
          <div class="flex justify-center items-center mr-3 gap-8">
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
          class="w-full flex flex-col justify-center items-center gap-3 px-3 pb-4"
        >
          <div v-for="filter in currentFilters.list" class="w-full"> 
            <Filter
              :id="filter.id"
              :key="filter.id"
              :title="filter.title"
              :currentlyRemoving="currentlyRemoving"
              :removeCaller="removeFilter"
              :validStrings="filter.validStrings"
              :filterType="filter.filterType"
              v-model="filter.writeable"
            />
          </div>
        </TransitionGroup>
      </div>
    </div>
    
    <div class="w-full flex justify-center items-center">
      <button class="btn btn-primary">Submit</button>
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

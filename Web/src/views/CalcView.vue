<script setup lang="ts">
import { ref, reactive, toRef } from 'vue';
import { type Ref } from 'vue';
import Filter from '@/components/Filter.vue';
import { SquaresPlusIcon, TrashIcon } from '@heroicons/vue/24/solid';
import { TrashIcon as OutlineTrashIcon } from '@heroicons/vue/24/outline';


interface Filter {
  title: string;
  filterType: 'string' | 'number';
}

interface FilterItem {
  title: string;
  filterType: 'string' | 'number';
  writeable: {
    filterOption: 'min' | 'max' | 'from'; // Only in filterType === 'number'
    value: string | number[];
  }
}

const filterList: Filter[] = [
  { title: 'Force Stock', filterType: 'string' },
  { title: 'Force Grip', filterType: 'string' },
  { title: 'Damage', filterType: 'number' },
  { title: 'Range', filterType: 'number' },
  { title: 'Firerate', filterType: 'number' },
];

const currentFilters = ref<FilterItem[]>([]);
const selectedFilterToAdd = ref<Filter>(filterList[0]!);
const currentlyRemoving = ref<boolean>(false);

function addFilter() {
  currentFilters.value.push({
    writeable: {
      filterOption: 'min',
      value: selectedFilterToAdd.value?.filterType === 'number' ? [0, 0] : '',
    },
    ...selectedFilterToAdd.value,
  });
  console.log(currentFilters.value)
}

function toggleRemoveFilter() {
  currentlyRemoving.value = !currentlyRemoving.value;
}
</script>

<template>
  <section class="grid grid-cols-1 gap-5 w-full">
    <div class="flex w-full justify-center items-center">
      <div class="bg-base-200 border rounded-md w-full max-w-200 min-h-200 mx-10">
        <div class="flex items-center justify-between">
          <h2 class="font-semibold ml-3 my-5 text-2xl">Filter</h2>
          <div class="flex justify-center items-center mr-3 gap-8">
            <select class="select min-w-20" v-model="selectedFilterToAdd">
              <option v-for="filter in filterList" :key="filter.title" :value="filter">
                {{ filter.title }}
              </option>
            </select>
            <button class="btn btn-success" @click="addFilter">
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
          class="w-full flex flex-col justify-center items-center gap-3 px-3"
        >
          <div v-for="filter in currentFilters" :key="filter.title" class="w-full">
            <Filter
              :title="filter.title"
              :currentlyRemoving="currentlyRemoving"
              :removeCaller="removeFilter"
              :filterType="filter.filterType"
              v-model="filter.writeable"
            />
          </div>
        </TransitionGroup>
      </div>
    </div>
  </section>
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

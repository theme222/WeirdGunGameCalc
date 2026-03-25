<script setup>
import { ref } from 'vue';

const helpModal = ref(null);
const currentPage = ref(0);

const pages = [
  `This is a tool for calculating the best possible gun based on certain criteria. It uses an algorithm to determine valid combinations and sorts them based on the current sort type. I can't 100% gaurantee perfect results for every gun so please notify me on discord @theme222 or github if you find any issues.`,
  `To Start, select a filter type from the input on the top of the filter section and press "Add Filter". There will be 3 required filters which can't be removed: Categories (The allowed categories for the core), Sort Type (The order of the output list), and Total Results (How many to show).`,
  `By pressing the trash icon on the top right of the filter list, it will toggle delete mode. In delete mode, you can click on the button next to the filter to remove it. Press the trash icon again to exit delete mode.`,
  `After adjusting the settings for each filter you can finally press the button below to see the results. Depending on the filters and the number of results, the calculation time may vary. But you should expect a delay of no more than a second.`,
  `After waiting for a bit, the results will be displayed. It will show you guns ranked from best to worst based on your sorting type. Each item in the list will show you the gun's barrel, magazine, grip, stock and core. You can also click to expand the item for more details.`,
  `By checking "Auto Update On Change" the calculator will run whenever you make a change to the filters. You can easily change number filters by scrolling up and down on them. You can also select whether to filter a minimum, maximum or in a range`,
  `For part filters (Force or Ban Parts), once you start typing there will be auto suggestions. You can press Enter to submit the part and it will show up in a list to the side. Press on the name of the part to deselect them.`,
  `In the results section. There will be a button called "Stats to Compare" which will open a menu to select the details you want to show of that gun. By default, it will show the stats that I think are commonly useful (TTK, Damage, Spread, Range, Magazine Size, Recoil, Speed, Health)`,
  `Other than bruteforcing combinations, this website can also be used to simply compute the result of 1 combination. By forcing the barrel, magazine, grip, stock and core. Also, if you ever see a rogue stat card that someone is gatekeeping the parts of, you can simply filter by range each stat (± 0.1 for errors) to find the exact combination they used!`,
  `This website uses Web Assembly to run c++ code on a browser. So while it is pretty fast, you can get better speeds by using the CLI (Since that allows multi threading). Currently it also doesn't have the option to set default max health and banning price types like the CLI. it also doesn't have a way to specify scopes (but honestly who cares about scopes anyways lol).`,
  `Thanks for reading the tutorial! If you want to learn more about how it works, check out the github repo. And just as a reminder, META guns are subjective and each gun has its own usecases. Never stop trying to push the limits!`
];

const titles = [
  `Tutorial (Press esc or click outside to close)`,
  `Getting Started`,
  `Removing Filters`,
  `Calculation`,
  `Viewing Results`,
  `Fine Tuning Results`,
  `Part Filters`,
  `Gun Comparison`,
  `Other Use Cases`,
  `Limitations`,
  `Thanks For Reading And Enjoy!`
]

function clamp(num, min, max) { return Math.min(Math.max(num, min), max); }

function decrement() {
  currentPage.value -= 1
  currentPage.value = clamp(currentPage.value, 0, pages.length - 1)
}

function increment() {
  currentPage.value += 1
  currentPage.value = clamp(currentPage.value, 0, pages.length - 1)
}

</script>

<template>
  <button
    class="btn btn-circle fixed bottom-5 left-5 z-40 btn-outline text-xl"
    @click="helpModal.showModal()"
  >
    ?
  </button>
  <dialog ref="helpModal" class="modal">
    <div class="modal-box">
      <h3 class="text-lg font-bold">{{ titles[currentPage] }}</h3>
      <p class="my-4 text-pretty">
        {{ pages[currentPage] }}
      </p>
      <div class="w-full flex justify-center items-center">
        <button class="btn btn-ghost" @click="decrement"> < </button>
        <button class="px-4">{{ currentPage + 1 }}</button>
        <button class="btn btn-ghost" @click="increment"> > </button>
      </div>
    </div>
    <form method="dialog" class="modal-backdrop">
      <button>close</button>
    </form>
  </dialog>
</template>

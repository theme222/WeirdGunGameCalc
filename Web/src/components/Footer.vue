<script setup>
import { calculatorVersion } from '@/libs/calc';
import { CalculatorIcon } from '@heroicons/vue/24/outline';
</script>


<template>
<footer class="m-8 fixed w-full bottom-0 opacity-50 z-20">
  <aside class="flex justify-center items-center gap-2 w-full text-xs">
    <CalculatorIcon class="size-4" />
    WeirdGunGameCalc v{{ calculatorVersion }} by Theme222 Check out the source code on<a href="https://github.com/theme222/WeirdGunGameCalc" target="_blank" class="underline">GitHub</a>
  </aside>
</footer>
</template>

<script setup>
import { useRouter } from 'vue-router'
import ViewportIndicator from './components/ViewportIndicator.vue';
import ToastContainer from './structure/ToastContainer.vue';
import Footer from './components/Footer.vue';
import Sidebar from './components/Sidebar.vue';

const isDev = import.meta.env.DEV;

</script>

<template>
<!-- <div id="background" class="fixed w-full -z-20 h-full bg-base-300"></div> -->

<Sidebar />
<div>
  <RouterView />
  <ToastContainer />
</div>
<ViewportIndicator v-if="isDev"/>
<Footer />

</template>

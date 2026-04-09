<script setup>
import { useRouter } from 'vue-router';

const router = useRouter();

if (sessionStorage.getItem("redirect")) {
  router.replace(sessionStorage.getItem("redirect"));
  sessionStorage.removeItem("redirect");
}
else {
  router.push('/calc');
}

</script>
<template>
</template>
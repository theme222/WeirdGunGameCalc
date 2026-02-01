import { createRouter, createWebHistory } from 'vue-router'
import BuilderView from '@/views/BuilderView.vue'
import CalcView from '@/views/CalcView.vue'
import HomeView from '@/views/HomeView.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {path: '/builder', component: BuilderView},
    {path: '/calc', component: CalcView},
    {path: '/', component: HomeView}
  ],
})

export default router

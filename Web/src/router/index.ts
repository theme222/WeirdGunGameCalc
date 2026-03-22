import { createRouter, createWebHistory } from 'vue-router'
import PartView from '@/views/PartView.vue'
import CalcView from '@/views/CalcView.vue'
import HomeView from '@/views/HomeView.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {path: '/part', component: PartView},
    {path: '/calc', component: CalcView},
    {path: '/', component: HomeView}
  ],
})

export default router

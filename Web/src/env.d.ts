// src/env.d.ts
declare module '*.vue' {
  import type { DefineComponent } from 'vue'
  // Tell TypeScript that any .vue file exports a Vue component
  const component: DefineComponent<{}, {}, any>
  export default component
}
import { reactive } from 'vue';

let id = 0; // Current id

interface Toast {
  id: number;
  message: string;
  type: string;
}

export const toastState = reactive<{ toasts: Toast[] }>({
  toasts: [],
});

export function addToast(message: string, type = 'info') {
  const toast: Toast = {
    id: id++,
    message,
    type,
  };

  toastState.toasts.push(toast);

  // auto remove after 5s
  setTimeout(() => removeToast(toast.id), 5000);
}

export function removeToast(id: number) {
  const index = toastState.toasts.findIndex((t) => t.id === id);
  if (index !== -1) toastState.toasts.splice(index, 1);
}

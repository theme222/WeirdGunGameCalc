// So tailwind can include these into build files. You don't actually need to import these. Just use `xxx-${color}`
export function getOutlineClass(color: string) {
  switch(color) {
    case 'primary':
      return 'outline-primary';
    case 'secondary':
      return 'outline-secondary';
    case 'accent':
      return 'outline-accent';
    case 'info':
      return 'outline-info';
    case 'success':
      return 'outline-success';
    case 'warning':
      return 'outline-warning';
    case 'error':
      return 'outline-error';
    default:
      return 'outline-base-content';
  }
}

export function getBackgroundClass(color: string) {
  switch(color) {
    case 'primary':
      return 'bg-primary';
    case 'secondary':
      return 'bg-secondary';
    case 'accent':
      return 'bg-accent';
    case 'info':
      return 'bg-info';
    case 'success':
      return 'bg-success';
    case 'warning':
      return 'bg-warning';
    case 'error':
      return 'bg-error';
    default:
      return 'bg-base-content';
  }
}

export function getButtonClass(color: string) {
  switch(color) {
    case 'primary':
      return 'btn-primary';
    case 'secondary':
      return 'btn-secondary';
    case 'accent':
      return 'btn-accent';
    case 'info':
      return 'btn-info';
    case 'success':
      return 'btn-success';
    case 'warning':
      return 'btn-warning';
    case 'error':
      return 'btn-error';
    default:
      return 'btn-base-content';
  }
}

export function getTextClass(color: string) {
  switch(color) {
    case 'primary':
      return 'text-primary';
    case 'secondary':
      return 'text-secondary';
    case 'accent':
      return 'text-accent';
    case 'info':
      return 'text-info';
    case 'success':
      return 'text-success';
    case 'warning':
      return 'text-warning';
    case 'error':
      return 'text-error';
    default:
      return 'text-base-content';
  }
}

export function getSelectClass(color: string) {
  switch(color) {
    case 'primary':
      return 'select-primary';
    case 'secondary':
      return 'select-secondary';
    case 'accent':
      return 'select-accent';
    case 'info':
      return 'select-info';
    case 'success':
      return 'select-success';
    case 'warning':
      return 'select-warning';
    case 'error':
      return 'select-error';
    default:
      return 'select-base-content';
  }
}

export function getInputClass(color: string) {
  switch(color) {
    case 'primary':
      return 'input-primary';
    case 'secondary':
      return 'input-secondary';
    case 'accent':
      return 'input-accent';
    case 'info':
      return 'input-info';
    case 'success':
      return 'input-success';
    case 'warning':
      return 'input-warning';
    case 'error':
      return 'input-error';
    default:
      return 'input-base-content';
  }
}

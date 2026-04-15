// Handles NumberInput slider

import { ref, watch, type Ref } from 'vue';

type Slider = {
  element: Ref<HTMLElement | null>;
  originX: Ref<number>;
  deltaX: Ref<number>;
  originY: Ref<number>;
  deltaY: Ref<number>;
  isDragging: Ref<boolean>;
};

const sliders: Slider[] = [];
let currentIndex = -1;

function onMouseUp(event: MouseEvent) {
  if (sliders[currentIndex] === undefined) return;
  event.stopPropagation();
  event.preventDefault();
  sliders[currentIndex]!.isDragging.value = false;
  currentIndex = -1;
}

function onMouseMove(event: MouseEvent) {
  if (sliders[currentIndex] === undefined) return;
  event.stopPropagation();
  event.preventDefault();
  const sliderObj: Slider = sliders[currentIndex]!;
  const deltaX = event.clientX - sliderObj.originX.value;
  const deltaY = event.clientY - sliderObj.originY.value;
  sliderObj.deltaX.value = deltaX;
  sliderObj.deltaY.value = deltaY;
}

document.addEventListener('mouseup', onMouseUp);
document.addEventListener('mousemove', onMouseMove);

function mouseDownFactory(index: number) {
  return function (event: MouseEvent) {
    event.stopPropagation();
    event.preventDefault();
    sliders[index]!.isDragging.value = true;
    sliders[index]!.originX.value = event.clientX;
    sliders[index]!.originY.value = event.clientY;
    currentIndex = index;
  };
}

export function useSlider(element: Ref<HTMLElement | null>) {
  const originX = ref(0);
  const deltaX = ref(0);
  const originY = ref(0);
  const deltaY = ref(0);
  const isDragging = ref(false);
  const index = sliders.length;

  const mouseDown = mouseDownFactory(index);
  element.value?.addEventListener('mousedown', mouseDown);
  
  sliders.push({
    element,
    originX,
    deltaX,
    originY,
    deltaY,
    isDragging,
  })
  
  return {
    originX,
    deltaX,
    originY,
    deltaY,
    isDragging,
  };
}
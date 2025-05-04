
import os, sys, threading, winsound
from pynput import mouse, keyboard
from PIL import ImageGrab

corner = [100, 100]
size = [328, 576]
save_dir = os.path.join(os.path.dirname(__file__), "Screenshots")
sound_file = os.path.join(os.path.dirname(__file__), "pop.mp3")
os.makedirs(save_dir, exist_ok=True)

def listen_console():
    while True:
        s = input()
        if s.startswith("WH "):
            try:
                w, h = map(int, s[3:].replace(' ', '').split(','))
                size[0], size[1] = w, h
            except: pass

def get_next_path():
    i = 1
    while os.path.exists(p := os.path.join(save_dir, f"{i}.png")): i += 1
    return p

def on_click(x, y, b, p):
    if b == mouse.Button.left and p:
        corner[0], corner[1] = x, y
        return False

def select_corner():
    with mouse.Listener(on_click=on_click) as l: l.join()

def screenshot():
    img = ImageGrab.grab(bbox=(corner[0], corner[1], corner[0]+size[0], corner[1]+size[1]))
    img.save(get_next_path())
    winsound.PlaySound(sound_file, winsound.SND_FILENAME | winsound.SND_ASYNC)

def on_hotkey():
    screenshot()

def on_reselect():
    threading.Thread(target=select_corner, daemon=True).start()

def kb_listener():
    with keyboard.GlobalHotKeys({
        '<ctrl>+<alt>+s': on_hotkey,
        '<ctrl>+<alt>+r': on_reselect
    }) as h:
        h.join()

threading.Thread(target=listen_console, daemon=True).start()
on_reselect()
kb_listener()

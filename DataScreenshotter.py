"""
MODIFIED BY CHATGPT
"""

import os, threading, winsound
from pynput import mouse, keyboard
from PIL import ImageGrab

# Initial corner and size
corner = [100, 100]
size = [328, 576]

# Directory for screenshots
save_dir = os.path.join(os.path.dirname(__file__), "Screenshots")
os.makedirs(save_dir, exist_ok=True)

# -------------------------
# Console commands listener
# -------------------------
def listen_console():
    while True:
        s = input()
        if s.startswith("WH "):
            try:
                w, h = map(int, s[3:].replace(' ', '').split(','))
                size[0], size[1] = w, h
                print(f"[INFO] Screenshot size set to {w}x{h}")
            except:
                print("[ERROR] Invalid format. Use: WH width,height")

# -------------------------
# File naming
# -------------------------
def get_next_path():
    i = 1
    while os.path.exists(p := os.path.join(save_dir, f"{i}.png")):
        i += 1
    return p

# -------------------------
# Mouse listener for corner
# -------------------------
def on_click(x, y, b, p):
    if b == mouse.Button.left and p:
        corner[0], corner[1] = x, y
        print(f"[INFO] Corner set to ({corner[0]}, {corner[1]})")
        return False

def select_corner():
    with mouse.Listener(on_click=on_click) as l:
        l.join()

# -------------------------
# Screenshot function
# -------------------------
def screenshot():
    path = get_next_path()
    img = ImageGrab.grab(bbox=(corner[0], corner[1],
                               corner[0]+size[0],
                               corner[1]+size[1]))
    img.save(path)
    print(f"[INFO] Screenshot saved: {path}")
    winsound.PlaySound("SystemExclamation", winsound.SND_ALIAS | winsound.SND_ASYNC)

# -------------------------
# Hotkey callbacks
# -------------------------
def on_hotkey():
    screenshot()

def on_reselect():
    threading.Thread(target=select_corner, daemon=True).start()

# -------------------------
# Keyboard listener
# -------------------------
def kb_listener():
    with keyboard.GlobalHotKeys({
        '<ctrl>+<alt>+s': on_hotkey,
        '<ctrl>+<alt>+r': on_reselect
    }) as h:
        h.join()

# -------------------------
# Program start
# -------------------------
threading.Thread(target=listen_console, daemon=True).start()
on_reselect()
kb_listener()

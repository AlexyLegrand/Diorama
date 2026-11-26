# /// script
# requires-python = ">=3.10"
# dependencies = ["python-xlib==0.33"]
# ///
import argparse
import os
import subprocess
import sys
import time

from Xlib import X, XK, display
from Xlib.ext import xtest

WINDOW_TIMEOUT = 15.0
FOCUS_DELAY = 0.2
SAVE_FILES = ("progress.dat", "settings.dat")


def find_window(dpy, pid):
    pid_atom = dpy.intern_atom("_NET_WM_PID")
    pending = [dpy.screen().root]
    while pending:
        window = pending.pop()
        try:
            prop = window.get_full_property(pid_atom, X.AnyPropertyType)
            if prop is not None and prop.value[0] == pid and window.get_wm_name():
                return window
            pending.extend(window.query_tree().children)
        except Exception:
            continue
    return None


def wait_for_window(dpy, pid):
    deadline = time.time() + WINDOW_TIMEOUT
    while time.time() < deadline:
        window = find_window(dpy, pid)
        if window is not None:
            return window
        time.sleep(0.2)
    return None


def keycode(dpy, name):
    code = dpy.keysym_to_keycode(XK.string_to_keysym(name))
    if code == 0:
        sys.exit(f"unknown key: {name}")
    return code


def send(dpy, window, code, pressed):
    window.set_input_focus(X.RevertToParent, X.CurrentTime)
    dpy.sync()
    xtest.fake_input(dpy, X.KeyPress if pressed else X.KeyRelease, code)
    dpy.sync()


def screenshot(window, path):
    subprocess.run(["import", "-window", str(window.id), path], check=True)
    print(f"saved {path}")


def run_step(dpy, window, out, step):
    parts = step.split()
    action = parts[0]
    if action == "key":
        code = keycode(dpy, parts[1])
        send(dpy, window, code, True)
        time.sleep(0.05)
        send(dpy, window, code, False)
    elif action == "hold":
        code = keycode(dpy, parts[1])
        send(dpy, window, code, True)
        time.sleep(float(parts[2]))
        send(dpy, window, code, False)
    elif action == "wait":
        time.sleep(float(parts[1]))
    elif action == "shot":
        screenshot(window, os.path.join(out, parts[1] + ".png"))
    elif action == "burst":
        count, delay = int(parts[2]), float(parts[3])
        for i in range(count):
            screenshot(window, os.path.join(out, f"{parts[1]}_{i:02d}.png"))
            time.sleep(delay)
    else:
        sys.exit(f"unknown step: {step}")


def backup_saves():
    saved = {}
    for name in SAVE_FILES:
        if os.path.exists(name):
            with open(name, "rb") as f:
                saved[name] = f.read()
    return saved


def restore_saves(saved):
    for name in SAVE_FILES:
        if name in saved:
            with open(name, "wb") as f:
                f.write(saved[name])
        elif os.path.exists(name):
            os.remove(name)


def main():
    parser = argparse.ArgumentParser(description="Drive Diorama with key presses and capture screenshots.")
    parser.add_argument("--out", default="tools/out", help="folder for screenshots")
    parser.add_argument("--start", type=float, default=4.0, help="seconds to wait after the window appears")
    parser.add_argument("steps", nargs="+")
    args = parser.parse_args()

    os.makedirs(args.out, exist_ok=True)
    saved = backup_saves()
    game = subprocess.Popen(["./diorama"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        dpy = display.Display()
        window = wait_for_window(dpy, game.pid)
        if window is None:
            sys.exit("game window not found")
        time.sleep(args.start)
        time.sleep(FOCUS_DELAY)
        for step in args.steps:
            run_step(dpy, window, args.out, step)
    finally:
        game.terminate()
        game.wait()
        restore_saves(saved)


if __name__ == "__main__":
    main()

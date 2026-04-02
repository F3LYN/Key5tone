# ⌨️ Key5tone

A scalable keystroke and mouse input visualizer for Windows. Built with C++ and Direct2D, this overlay captures global inputs and renders them as crisp vector graphics (SVG) to be used for OBS or other apps. 
<img width="443" height="218" alt="image" src="https://github.com/user-attachments/assets/e57d78c3-5049-458c-9f0e-3560f26a443c" />

Perfect for streamers, tutorial creators, and speedrunners who want to display their inputs in real-time without losing image quality when scaling.

## ✨ Features

* **Global Input Hooking:** Captures keyboard and mouse events globally using low-level Windows API hooks (`WH_KEYBOARD_LL` and `WH_MOUSE_LL`).
* **Crisp Vector Graphics:** Uses Direct2D to render SVGs, allowing the window to be stretched to any size without pixelation.
* **Smart Combo Detection:** Automatically detects and chains modifier keys (CTRL, ALT, SHIFT) with mouse clicks and standard keystrokes.
* **Dynamic Shaders:** Uses Direct2D Effects (`CLSID_D2D1Shadow`) to generate a customizable, tight outline around the complex shapes of the keys and mouse icons.
**MORE TO COME**
* - **Interactive Context Menu:** Right-click the overlay to access a native Windows menu to change outline colors or exit the application.

## 🛠️ Prerequisites

To build this project from source, you will need:
* Windows 10 or 11
* **Desktop development with C++** workload installed.
* Windows SDK (for Direct2D, DirectWrite, and standard Windows API libraries).

## 🚀 USAGE
 1 - Lunch the application. A transparent window will appear.
 2 - Press any keyboard key or click any mouse button to see it rendered on screen.
 3 - Resize: Drag the corners of the window to scale the UI up or down. The SVGs will automatically redraw at native resolution.

 ## DOWNLOAD BELLOW
 
[Key5tone x64.zip](https://github.com/user-attachments/files/26428042/Key5tone.x64.zip)

/* KeyProcess.cpp */
#include <iostream>  // library to print text to the screen
#include <windows.h> // windows API
#include <string> // string library
#include <winuser.h>
#include "KeyTranslator.h"
#include "KeyProcess.h"

std::string gBtnLabel;

LRESULT CALLBACK KeyboardProcess(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode < 0) return CallNextHookEx(NULL, nCode, wParam, lParam);

	// 1. Create a memory of the last key we saw
	static int lastKeyCode = 0;

	// 2. If a key is released, clear our memory so it can be pressed again later
	if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
		lastKeyCode = 0;
	}

	if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
		KBDLLHOOKSTRUCT* keyInfo = (KBDLLHOOKSTRUCT*)lParam;
		int keyCode = (int)keyInfo->vkCode;

		// 3. THE FIX: If this key is the exact same as the last one we processed, 
		// it's an auto-repeat ghost! Ignore it.
		if (keyCode == lastKeyCode) {
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		}
		lastKeyCode = keyCode; // Update our memory

		gBtnLabel = "";

		std::string specialKey = GetSpecialKeyName(keyCode);

		if (!specialKey.empty()) {
			gBtnLabel = specialKey;
		}
		else {
			char keyChar = (char)MapVirtualKeyA(keyCode, MAPVK_VK_TO_CHAR);
			gBtnLabel += keyChar;

			// Check modifiers before adding the key name
			std::string prefix = "";
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000) prefix += "CTRL + ";
			if (GetAsyncKeyState(VK_MENU) & 0x8000)    prefix += "ALT + ";
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)   prefix += "SHIFT + ";

			gBtnLabel = prefix + (specialKey.empty() ? std::string(1, keyChar) : specialKey);
		}

		// sets the timer to clear the keys
		SetTimer(g_hwndOverlay, 1, 2000, NULL);

		InvalidateRect(g_hwndOverlay, NULL, FALSE);

		std::cout << "KEY_VALUE: " << keyCode << " --- ";
		std::cout << "KEY: " << gBtnLabel << "\n";
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK MouseProcess(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode < 0) return CallNextHookEx(NULL, nCode, wParam, lParam);


	// Statement that runs to force exit the function when mouse is moved or buttons are released.

	if (wParam == WM_MOUSEMOVE || wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MBUTTONUP) {
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}

	MSLLHOOKSTRUCT* mouseInfo = (MSLLHOOKSTRUCT*)lParam;
	gBtnLabel = "";	//resets the button label:

	// 1. Build the modifier prefix by checking the keyboard
	std::string prefix = "";
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) prefix += "CTRL + ";
	if (GetAsyncKeyState(VK_MENU) & 0x8000)    prefix += "ALT + ";
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)   prefix += "SHIFT + ";


	// Logic that activates depending on which button on the mouse is being pressed.


	if (wParam == WM_MOUSEWHEEL) {
		short scrollDirection = HIWORD(mouseInfo->mouseData);

		gBtnLabel = prefix + ((scrollDirection > 0) ? "SCROLL UP" : "SCROLL DOWN");
	}
	else {
		gBtnLabel = prefix + GetSpecialKeyName((int)wParam);
	}

	// sets the timer to clear the keys
	SetTimer(g_hwndOverlay, 1, 2000, NULL);

	InvalidateRect(g_hwndOverlay, NULL, FALSE);

	std::cout << "KEY_VALUE: " << wParam << " --- ";
	std::cout << "KEY: " << gBtnLabel << "\n";

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
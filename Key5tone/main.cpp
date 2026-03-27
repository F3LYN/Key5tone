/* main.cpp */
#include <iostream>  // library to print text to the screen
#include <windows.h> // windows API
#include <string> // string library
#include <winuser.h>
#include "KeyTranslator.h"
#include "KeyProcess.h"
#include "OverlayWindow.h"

// Global Variables
HHOOK KeyboardListener;
HHOOK MouseListener;
HWND g_hwndOverlay = NULL;


// ----------- MAIN -----------
int main() {
	if (!SetConsoleOutputCP(CP_UTF8)) {
		std::cerr << "Failed to set output to UTF-8" << std::endl;
	}

	/*
	WH_KEYBOARD_LL: Listens to keyboard strokes, LL = Low level.
	KeyboardProcess: The function that gets triggered every time a key is pressed.
	0 = The thread ID that tells to listen all programs globally.
	*/
	KeyboardListener = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProcess, NULL, 0);
	MouseListener = SetWindowsHookEx(WH_MOUSE_LL, MouseProcess, NULL, 0);
	MSG message;

	g_hwndOverlay = CreateOverlayWindow();
	InitGraphics(g_hwndOverlay);

	std::cout << "Waiting for a key press ...\n";
	
	

	/*
	&message: memory address of the message variable where new events are dumped into.
	NUL, 0, 0 : Values that tell windows to not filter anything out. Read all messages basically.
	*/
	while (GetMessage(&message, NULL, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return 0;
}
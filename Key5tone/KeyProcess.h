/* KeyProcess.h */
#pragma once
#include <string>
# include <windows.h>
extern std::string gBtnLabel;

extern HWND g_hwndOverlay;

LRESULT CALLBACK KeyboardProcess(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MouseProcess(int nCode, WPARAM wParam, LPARAM lParam);
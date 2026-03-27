/* KeyTranslator.cpp */
#include <iostream>  // library to print text to the screen
#include <windows.h> // windows API
#include <string> // string library
#include <winuser.h>
#include "KeyTranslator.h"

std::string GetSpecialKeyName(int keyCode) {
	switch (keyCode) {
	case VK_SPACE: return "SPACE";
	case VK_RETURN: return "ENTER";
	case VK_BACK: return "←";
	case VK_TAB: return "TAB";
	case VK_LSHIFT:return "LEFT\nSHIFT";
	case VK_RSHIFT: return "RIGHT\nSHIFT";
	case VK_SHIFT: return "SHIFT";
	case VK_LCONTROL: return "LEFT\nCTRL";
	case VK_RCONTROL: return "RIGHT\nCTRL";
	case VK_CONTROL: return "CTRL";
	case VK_LMENU: return "LEFT\nALT";
	case VK_RMENU: return "RIGHT\nALT";
	case VK_MENU: return "ALT";
	case VK_PAUSE: return "PAUSE";
	case VK_CAPITAL: return "CAPS";
	case VK_ESCAPE: return "ESC";
	case VK_PRIOR: return "PAGE\nUP";
	case VK_NEXT: return "PAGE\nDOWN";
	case VK_END: return "END";
	case VK_HOME: return "HOME";
	case VK_LEFT: return "⇦";
	case VK_UP: return "⇧";
	case VK_RIGHT: return "⇨";
	case VK_DOWN: return "⇩";
	case VK_PRINT: return "🖶";
	case VK_SNAPSHOT: return "PRT\nSCR";
	case VK_SCROLL: return "SCR\nLK";
	case VK_INSERT: return "INS";
	case VK_DELETE: return "DEL";
	case VK_LWIN: return "田";
	case VK_RWIN: return "田";
	case VK_SLEEP: return "SLEEP";
	case VK_APPS: return "▤";
	case VK_F1: return "F1";
	case VK_F2: return "F2";
	case VK_F3: return "F3";
	case VK_F4: return "F4";
	case VK_F5: return "F5";
	case VK_F6: return "F6";
	case VK_F7: return "F7";
	case VK_F8: return "F8";
	case VK_F9: return "F9";
	case VK_F10: return "F10";
	case VK_F11: return "F11";
	case VK_F12: return "F12";
	case VK_F13: return "F13";
	case VK_F14: return "F14";
	case VK_F15: return "F15";
	case VK_F16: return "F16";
	case VK_F17: return "F17";
	case VK_F18: return "F18";
	case VK_F19: return "F19";
	case VK_F20: return "F20";
	case VK_F21: return "F21";
	case VK_F22: return "F22";
	case VK_F23: return "F23";
	case VK_F24: return "F24";
	case VK_NUMPAD0: return "NUMPAD\n0";
	case VK_NUMPAD1: return "NUMPAD\n1";
	case VK_NUMPAD2: return "NUMPAD\n2";
	case VK_NUMPAD3: return "NUMPAD\n3";
	case VK_NUMPAD4: return "NUMPAD\n4";
	case VK_NUMPAD5: return "NUMPAD\n5";
	case VK_NUMPAD6: return "NUMPAD\n6";
	case VK_NUMPAD7: return "NUMPAD\n7";
	case VK_NUMPAD8: return "NUMPAD\n8";
	case VK_NUMPAD9: return "NUMPAD\n9";
	case VK_MULTIPLY: return "*";
	case VK_ADD: return "+";
	case VK_SEPARATOR: return "SEPARATOR";
	case VK_SUBTRACT: return "-";
	case VK_DECIMAL: return ".";
	case VK_DIVIDE: return "/";
	case VK_NUMLOCK: return "NUM";
	case VK_VOLUME_UP: return "VOLUME UP";
	case VK_VOLUME_DOWN: return "VOLUME DOWN";
	case VK_VOLUME_MUTE: return "VOLUME MUTE";
	case VK_MEDIA_PLAY_PAUSE: return "MEDIA PAUSE";
	case VK_OEM_3: return "~";

	case  WM_LBUTTONDOWN: return "LEFT CLICK";
	case  WM_RBUTTONDOWN: return "RIGHT CLICK";
	case  WM_MBUTTONDOWN: return "MIDDLE CLICK";
	default: return "";
	}
}
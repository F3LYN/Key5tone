/* OverlayWindow.cpp */
#include <algorithm>
#include <iostream>
#define NOMINMAX
#include <windows.h>
#include <commdlg.h> 
#include <string> 
#include <vector>
#include <dwrite.h>
#include <d2d1effects.h>
#include <winuser.h>
#include "OverlayWindow.h"
#include "KeyProcess.h"

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "Comdlg32.lib")

ID2D1Factory1* pFactory = NULL;
ID3D11Device* pD3DDevice = NULL;
IDXGISwapChain1* pSwapChain = NULL;
ID2D1DeviceContext5* pContext = NULL;
IDWriteFactory* pDWriteFactory = NULL;
ID2D1SolidColorBrush* pTextBrush = NULL;
ID2D1SolidColorBrush* pGlowBrush = NULL;

ID2D1Effect* pDilateEffect = NULL;
ID2D1Effect* pColorEffect = NULL;

// --- GLOBAL VARIABLES ---
float gAppScale = 1.0f;
bool g_isResizing = false;
float g_outlineThickness = 1.8f;
bool g_useWireframe = false; // Toggle for wireframe boxes instead of SVGs

D2D1_COLOR_F g_outlineColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
D2D1_COLOR_F g_bgColor = D2D1::ColorF(0.1f, 1.0f, 0.1f, 1.0f);
D2D1_COLOR_F g_textColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);

COLORREF g_customColors[16] = { 0 };

// Trackers for our unified Settings Window
HWND g_hSettingsWnd = NULL;
HWND g_hThicknessEdit = NULL;

// buttons
ID2D1SvgDocument* pSvgNormal = NULL;
ID2D1SvgDocument* pSvgEnter = NULL;
ID2D1SvgDocument* pSvgShift = NULL;
ID2D1SvgDocument* pSvgBackSpace = NULL;
ID2D1SvgDocument* pSvgSpace = NULL;
ID2D1SvgDocument* pSvgCaps = NULL;
ID2D1SvgDocument* pSvgTab = NULL;
ID2D1SvgDocument* pSvgNumpadWide = NULL;
ID2D1SvgDocument* pSvgNumpadTall = NULL;

// mouse Buttons
ID2D1SvgDocument* pSvgRClick = NULL;
ID2D1SvgDocument* pSvgLClick = NULL;
ID2D1SvgDocument* pSvgMClick = NULL;
ID2D1SvgDocument* pSvgScrUP = NULL;
ID2D1SvgDocument* pSvgScrDwn = NULL;


void InitGraphics(HWND hwnd) {
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
	D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, NULL, 0, D3D11_SDK_VERSION, &pD3DDevice, NULL, NULL);

	IDXGIDevice* pDXGIDevice;
	pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
	ID2D1Device* pD2DDevice;
	pFactory->CreateDevice(pDXGIDevice, &pD2DDevice);

	ID2D1DeviceContext* pTempContext = NULL;
	pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pTempContext);
	pTempContext->QueryInterface(__uuidof(ID2D1DeviceContext5), (void**)&pContext);
	pTempContext->Release();

	IDXGIAdapter* pAdapter;
	pDXGIDevice->GetAdapter(&pAdapter);
	IDXGIFactory2* pDXGIFactory;
	pAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&pDXGIFactory);

	DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
	swapDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapDesc.BufferCount = 2;
	swapDesc.SampleDesc.Count = 1;

	RECT rc;
	GetClientRect(hwnd, &rc);
	swapDesc.Width = rc.right - rc.left;
	swapDesc.Height = rc.bottom - rc.top;

	pDXGIFactory->CreateSwapChainForHwnd(pD3DDevice, hwnd, &swapDesc, NULL, NULL, &pSwapChain);

	IDXGISurface* pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&pBackBuffer);

	ID2D1Bitmap1* pTargetBitmap;
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);
	pContext->CreateBitmapFromDxgiSurface(pBackBuffer, &bitmapProperties, &pTargetBitmap);
	pContext->SetTarget(pTargetBitmap);

	pTargetBitmap->Release();
	pBackBuffer->Release();

	pSvgNormal = LoadSVG(L"svg_files\\reg_key_btn.svg");
	pSvgSpace = LoadSVG(L"svg_files\\space_key_btn.svg");
	pSvgEnter = LoadSVG(L"svg_files\\enter_key_btn.svg");
	pSvgShift = LoadSVG(L"svg_files\\shift_key_btn.svg");
	pSvgBackSpace = LoadSVG(L"svg_files\\bcksp_key_btn.svg");
	pSvgCaps = LoadSVG(L"svg_files\\caps_key_btn.svg");
	pSvgTab = LoadSVG(L"svg_files\\tab_key_btn.svg");
	pSvgNumpadTall = LoadSVG(L"svg_files\\nmpTL_key_btn.svg");
	pSvgNumpadWide = LoadSVG(L"svg_files\\nmpWD_key_btn.svg");

	pSvgRClick = LoadSVG(L"svg_files\\mouse_RClick.svg");
	pSvgLClick = LoadSVG(L"svg_files\\mouse_LClick.svg");
	pSvgMClick = LoadSVG(L"svg_files\\mouse_MClick.svg");
	pSvgScrUP = LoadSVG(L"svg_files\\mouse_ScrollUP.svg");
	pSvgScrDwn = LoadSVG(L"svg_files\\mouse_ScrollDOWN.svg");

	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));

	// Create brushes
	pContext->CreateSolidColorBrush(g_textColor, &pTextBrush);
	pContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &pGlowBrush); // Used as a solid stencil

	// Create Solid Outline Effects
	pContext->CreateEffect(CLSID_D2D1Morphology, &pDilateEffect);
	pDilateEffect->SetValue(D2D1_MORPHOLOGY_PROP_MODE, D2D1_MORPHOLOGY_MODE_DILATE);
	pContext->CreateEffect(CLSID_D2D1ColorMatrix, &pColorEffect);
}


/* ---------------- FUNCTION TO OPEN WINDOWS COLOR PICKER ---------------- */
bool OpenColorPicker(HWND hwnd, D2D1_COLOR_F& colorOut) {
	CHOOSECOLOR cc = { 0 };
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hwnd;
	cc.lpCustColors = g_customColors;

	cc.rgbResult = RGB(
		(BYTE)(colorOut.r * 255.0f),
		(BYTE)(colorOut.g * 255.0f),
		(BYTE)(colorOut.b * 255.0f)
	);

	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&cc)) {
		colorOut.r = GetRValue(cc.rgbResult) / 255.0f;
		colorOut.g = GetGValue(cc.rgbResult) / 255.0f;
		colorOut.b = GetBValue(cc.rgbResult) / 255.0f;
		colorOut.a = 1.0f;
		return true;
	}
	return false;
}


/* ---------------- FUNCTION TO HANDLE THE UNIFIED SETTINGS WINDOW ---------------- */
LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE: {
		// 1. Thickness Label and Text Box
		CreateWindowEx(0, L"STATIC", L"Outline Thickness:", WS_CHILD | WS_VISIBLE, 15, 18, 120, 20, hwnd, NULL, NULL, NULL);

		std::wstring thickStr = std::to_wstring(g_outlineThickness);
		thickStr = thickStr.substr(0, thickStr.find(L'.') + 2);

		g_hThicknessEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", thickStr.c_str(),
			WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 140, 15, 80, 25, hwnd, NULL, NULL, NULL);

		// 2. Color Buttons
		CreateWindowEx(0, L"BUTTON", L"Change Outline Color", WS_CHILD | WS_VISIBLE, 15, 50, 205, 30, hwnd, (HMENU)2001, NULL, NULL);
		CreateWindowEx(0, L"BUTTON", L"Change Text Color", WS_CHILD | WS_VISIBLE, 15, 90, 205, 30, hwnd, (HMENU)2002, NULL, NULL);
		CreateWindowEx(0, L"BUTTON", L"Change Background Color", WS_CHILD | WS_VISIBLE, 15, 130, 205, 30, hwnd, (HMENU)2003, NULL, NULL);

		// 3. NEW: Wireframe Toggle Button
		CreateWindowEx(0, L"BUTTON", L"Toggle Wireframe Mode", WS_CHILD | WS_VISIBLE, 15, 170, 205, 30, hwnd, (HMENU)2005, NULL, NULL);

		// 4. Save & Close Button (Pushed down to make room)
		CreateWindowEx(0, L"BUTTON", L"Apply & Close", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 15, 210, 205, 30, hwnd, (HMENU)2004, NULL, NULL);
		return 0;
	}
	case WM_COMMAND: {
		int btnID = LOWORD(wParam);
		HWND parentHwnd = GetParent(hwnd);

		if (btnID == 2001) {
			if (OpenColorPicker(hwnd, g_outlineColor)) InvalidateRect(parentHwnd, NULL, FALSE);
		}
		else if (btnID == 2002) {
			if (OpenColorPicker(hwnd, g_textColor)) {
				pTextBrush->SetColor(g_textColor);
				InvalidateRect(parentHwnd, NULL, FALSE);
			}
		}
		else if (btnID == 2003) {
			if (OpenColorPicker(hwnd, g_bgColor)) InvalidateRect(parentHwnd, NULL, FALSE);
		}
		else if (btnID == 2005) {
			// Flip the wireframe switch instantly!
			g_useWireframe = !g_useWireframe;
			InvalidateRect(parentHwnd, NULL, FALSE);
		}
		else if (btnID == 2004) {
			// Apply the typed thickness value
			wchar_t buffer[32];
			GetWindowText(g_hThicknessEdit, buffer, 32);
			try {
				float newValue = std::stof(std::wstring(buffer));
				if (newValue >= 0.0f) g_outlineThickness = newValue;
			}
			catch (...) {}

			InvalidateRect(parentHwnd, NULL, FALSE);
			DestroyWindow(hwnd); // Close the settings window
		}
		return 0;
	}
	case WM_DESTROY:
		g_hSettingsWnd = NULL; // Free the tracker
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/* ---------------- FUNCTION TO OPEN THE SETTINGS WINDOW ---------------- */
void OpenSettingsWindow(HWND parentHwnd) {
	if (g_hSettingsWnd != NULL) return;

	WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
	wc.lpfnWndProc = SettingsWndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"SettingsPopupClass";
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	RegisterClassEx(&wc);

	POINT pt;
	GetCursorPos(&pt);

	g_hSettingsWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		L"SettingsPopupClass", L"Overlay Settings",
		WS_POPUPWINDOW | WS_CAPTION,
		pt.x, pt.y, 250, 290,
		parentHwnd, NULL, wc.hInstance, NULL
	);

	ShowWindow(g_hSettingsWnd, SW_SHOWDEFAULT);
}


/* ---------------- FUNCTION TO PROCESS THE MAIN WINDOW PANEL ---------------- */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		RenderGraphics();
		ValidateRect(hwnd, NULL);
		return 0;
	case WM_ERASEBKGND:
		return 1;

	case WM_CONTEXTMENU: {
		HMENU hMenu = CreatePopupMenu();
		InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, 1001, L"Settings");
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING, 1005, L"Exit App");

		POINT pt;
		GetCursorPos(&pt);

		int menuID = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
		DestroyMenu(hMenu);

		if (menuID == 1001) {
			OpenSettingsWindow(hwnd);
		}
		else if (menuID == 1005) {
			PostQuitMessage(0);
		}
		return 0;
	}

	case WM_ENTERSIZEMOVE:
		g_isResizing = true;
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;

	case WM_EXITSIZEMOVE:
		g_isResizing = false;
		if (pContext != NULL) {
			ResizeGraphics(hwnd);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		return 0;

	case WM_SIZE:
		if (pContext != NULL && !g_isResizing) {
			ResizeGraphics(hwnd);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		return 0;

	case WM_TIMER:
		gBtnLabel = "";
		InvalidateRect(hwnd, NULL, FALSE);
		KillTimer(hwnd, 1);
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


/* ---------------- FUNCTION TO REBUILD GRAPHICS WHEN WINDOW STRETCHES ---------------- */
void ResizeGraphics(HWND hwnd) {
	if (pContext == NULL || pSwapChain == NULL) return;

	pContext->SetTarget(NULL);

	RECT rc;
	GetClientRect(hwnd, &rc);
	UINT newWidth = rc.right - rc.left;
	UINT newHeight = rc.bottom - rc.top;

	if (newWidth == 0 || newHeight == 0) return;

	float scaleX = (float)newWidth / 800.0f;
	float scaleY = (float)newHeight / 250.0f;

	gAppScale = std::min(scaleX, scaleY);

	pSwapChain->ResizeBuffers(0, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, 0);

	IDXGISurface* pBackBuffer = NULL;
	pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&pBackBuffer);

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	ID2D1Bitmap1* pNewTargetBitmap = NULL;
	pContext->CreateBitmapFromDxgiSurface(pBackBuffer, &bitmapProperties, &pNewTargetBitmap);

	pContext->SetTarget(pNewTargetBitmap);

	pNewTargetBitmap->Release();
	pBackBuffer->Release();
}

HWND CreateOverlayWindow() {
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"OverlayClass";
	RegisterClassEx(&wc);

	HWND hwnd = CreateWindowEx(
		0,
		L"OverlayClass",
		L"Input Overlay",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 250,
		NULL, NULL, wc.hInstance, NULL
	);

	ShowWindow(hwnd, SW_SHOWDEFAULT);

	return hwnd;
}

/* ---------------- FUNCTION TO LOAD THE SVG FILE FROM A GIVEN PATH ---------------- */
ID2D1SvgDocument* LoadSVG(LPCWSTR filepath) {
	ID2D1SvgDocument* tempDoc = NULL;
	IStream* pStream = NULL;

	SHCreateStreamOnFile(filepath, STGM_READ, &pStream);
	if (pStream != NULL) {
		std::cout << "FILE FOUND: The SVG file was loaded \n";
		pContext->CreateSvgDocument(pStream, D2D1::SizeF(100.0f, 100.0f), &tempDoc);
		pStream->Release();
	}
	else {
		std::cout << "ERROR: Could not load SVG file.\n";
	}
	return tempDoc;
}

/* // ---------------- FUNCTION TO UTF8 CHARACTERS ---------------- \\ */
std::wstring ConvertUtf8ToWide(const std::string& str) {
	if (str.empty()) return std::wstring();

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);

	return wstrTo;
}

std::vector<std::string> SplitCombo(std::string text) {
	std::vector<std::string> parts;
	size_t pos = 0;
	while ((pos = text.find(" + ")) != std::string::npos) {
		parts.push_back(text.substr(0, pos));
		text.erase(0, pos + 3);
	}
	parts.push_back(text);
	return parts;
}


/* ---------------- FUNCTION TO HANDLE THE SVG BUTTON PARAMETERS ---------------- */
void DrawSVGKey(ID2D1SvgDocument* pDoc, std::string textToDraw, float xPos, float yPos, float width, float height, float fontSize, float scale) {
	xPos *= gAppScale;
	yPos *= gAppScale;
	width *= gAppScale;
	height *= gAppScale;
	fontSize *= gAppScale;

	// Detect if this key is a mouse action
	bool isMouse = (pDoc == pSvgLClick || pDoc == pSvgRClick || pDoc == pSvgMClick || pDoc == pSvgScrUP || pDoc == pSvgScrDwn);

	IDWriteTextFormat* pMainFormat = NULL;
	pDWriteFactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", &pMainFormat);
	pMainFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pMainFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	std::wstring wideText = L"";
	std::wstring topPart = L"";
	std::wstring bottomPart = L"";
	IDWriteTextFormat* pSmallFormat = NULL;
	D2D1_RECT_F topBox = { 0 }, bottomBox = { 0 }, textBox = { 0 };

	size_t splitPos = textToDraw.find('\n');
	bool isNumpad = (textToDraw.starts_with("NUMPAD") && splitPos != std::string::npos);

	if (isNumpad) {
		topPart = ConvertUtf8ToWide(textToDraw.substr(0, splitPos));
		bottomPart = ConvertUtf8ToWide(textToDraw.substr(splitPos + 1));
		pDWriteFactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL, fontSize * 0.4f, L"en-us", &pSmallFormat);
		pSmallFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		pSmallFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		topBox = D2D1::RectF(xPos, yPos, xPos + width, yPos + (height * 0.45f));
		bottomBox = D2D1::RectF(xPos, yPos - 10.0f + (height * 0.35f), xPos + width, yPos + height);
	}
	else if (!textToDraw.empty()) {
		wideText = ConvertUtf8ToWide(textToDraw);
		textBox = D2D1::RectF(xPos, yPos, xPos + width, yPos + height);
	}

	// NORMAL WIREFRAME (For Keyboards)
	D2D1_ROUNDED_RECT wireframeBox = D2D1::RoundedRect(
		D2D1::RectF(xPos, yPos, xPos + width, yPos + height),
		8.0f * gAppScale, 8.0f * gAppScale
	);

	// HELPER LOGIC TO BUILD THE CUSTOM MOUSE WIREFRAME
	auto DrawMouseWireframe = [&](ID2D1SolidColorBrush* brush, float stroke) {
		float mw = 55.0f * gAppScale; // Mouse Width
		float mh = 90.0f * gAppScale; // Mouse Height
		float mx = xPos + (width - mw) / 2.0f; // Centers the mouse dynamically
		float my = yPos;
		float cx = mx + mw / 2.0f; // Center X

		// 1. Draw Mouse Body
		D2D1_ROUNDED_RECT mouseBody = D2D1::RoundedRect(D2D1::RectF(mx, my, mx + mw, my + mh), 25.0f * gAppScale, 25.0f * gAppScale);

		// 3. Draw Hollow Scroll Wheel
		float divY = my + mh * 0.45f; // Horizontal Line separating buttons from body
		D2D1_ROUNDED_RECT wheel = D2D1::RoundedRect(D2D1::RectF(cx - 8.0f * gAppScale, my + 6.0f * gAppScale, cx + 8.0f * gAppScale, divY - 6.0f * gAppScale), 3.0f * gAppScale, 3.0f * gAppScale);
		pContext->DrawRoundedRectangle(wheel, brush, stroke * 0.4f);

		// 4. Handle Filling
		if (pDoc == pSvgLClick) {
			pContext->PushAxisAlignedClip(D2D1::RectF(mx, my, cx, wheel.rect.top), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			pContext->FillRoundedRectangle(mouseBody, brush);
			pContext->PopAxisAlignedClip();

			pContext->PushAxisAlignedClip(D2D1::RectF(mx, wheel.rect.top, wheel.rect.left, divY), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			pContext->FillRoundedRectangle(mouseBody, brush);
			pContext->PopAxisAlignedClip();
		}
		else if (pDoc == pSvgRClick) {
			pContext->PushAxisAlignedClip(D2D1::RectF(cx, my, mx + mw, wheel.rect.top), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			pContext->FillRoundedRectangle(mouseBody, brush);
			pContext->PopAxisAlignedClip();

			pContext->PushAxisAlignedClip(D2D1::RectF(wheel.rect.right, wheel.rect.top, mx + mw, divY), D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			pContext->FillRoundedRectangle(mouseBody, brush);
			pContext->PopAxisAlignedClip();
		}

		// 5. Fill Middle Click Wheel Solidly
		if (pDoc == pSvgMClick) {
			pContext->FillRoundedRectangle(wheel, brush);
		}

		// 6. Draw the outlines and dividers
		pContext->DrawRoundedRectangle(mouseBody, brush, stroke);

		pContext->DrawLine(D2D1::Point2F(mx, divY), D2D1::Point2F(cx, divY), brush, stroke); // Left segment
		pContext->DrawLine(D2D1::Point2F(cx, divY), D2D1::Point2F(mx + mw, divY), brush, stroke); // Right segment

		// 7. Handle Scrolling Arrows
		if (pDoc == pSvgScrUP) {
			float baseY = my;
			float tipY = my - 22.0f * gAppScale;
			pContext->DrawLine(D2D1::Point2F(cx, baseY), D2D1::Point2F(cx, tipY), brush, stroke);
			pContext->DrawLine(D2D1::Point2F(cx - 8.0f * gAppScale, tipY + 8.0f * gAppScale), D2D1::Point2F(cx, tipY), brush, stroke);
			pContext->DrawLine(D2D1::Point2F(cx + 8.0f * gAppScale, tipY + 8.0f * gAppScale), D2D1::Point2F(cx, tipY), brush, stroke);
		}
		else if (pDoc == pSvgScrDwn) {
			float baseY = divY;
			float tipY = divY + 28.0f * gAppScale;
			pContext->DrawLine(D2D1::Point2F(cx, baseY), D2D1::Point2F(cx, tipY), brush, stroke);
			pContext->DrawLine(D2D1::Point2F(cx - 8.0f * gAppScale, tipY - 8.0f * gAppScale), D2D1::Point2F(cx, tipY), brush, stroke);
			pContext->DrawLine(D2D1::Point2F(cx + 8.0f * gAppScale, tipY - 8.0f * gAppScale), D2D1::Point2F(cx, tipY), brush, stroke);
		}
		};

	ID2D1CommandList* pTempCanvas = NULL;
	pContext->CreateCommandList(&pTempCanvas);
	ID2D1Image* pMainScreen = NULL;
	pContext->GetTarget(&pMainScreen);
	pContext->SetTarget(pTempCanvas);
	pContext->Clear(D2D1::ColorF(0, 0, 0, 0));

	D2D1_MATRIX_3X2_F transform = D2D1::Matrix3x2F::Scale(scale * gAppScale, scale * gAppScale) * D2D1::Matrix3x2F::Translation(xPos, yPos);

	// 3. DRAW TO STENCIL (Wireframe OR Svg)
	if (pDoc != NULL) {
		if (g_useWireframe) {
			if (isMouse) DrawMouseWireframe(pGlowBrush, 4.0f * gAppScale);
			else pContext->DrawRoundedRectangle(wireframeBox, pGlowBrush, 4.0f * gAppScale);
		}
		else {
			pContext->SetTransform(transform);
			pContext->DrawSvgDocument(pDoc);
			pContext->SetTransform(D2D1::Matrix3x2F::Identity());
		}
	}

	pTempCanvas->Close();
	pContext->SetTarget(pMainScreen);
	pMainScreen->Release();

	// 4. APPLY THE OUTLINE
	float svgMultiplier = 4.0f; // Change this value for thinner/thicker outline for the svgs
	float finalThickness = g_useWireframe ? g_outlineThickness : (g_outlineThickness * svgMultiplier);

	pDilateEffect->SetInput(0, pTempCanvas);
	pDilateEffect->SetValue(D2D1_MORPHOLOGY_PROP_WIDTH, (UINT32)finalThickness);
	pDilateEffect->SetValue(D2D1_MORPHOLOGY_PROP_HEIGHT, (UINT32)finalThickness);

	D2D1_MATRIX_5X4_F matrix = D2D1::Matrix5x4F(
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 1,
		g_outlineColor.r, g_outlineColor.g, g_outlineColor.b, 0
	);
	pColorEffect->SetInputEffect(0, pDilateEffect);
	pColorEffect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);

	pContext->SetTransform(D2D1::Matrix3x2F::Identity());
	pContext->DrawImage(pColorEffect);

	// 5. DRAW CRISP INSIDES (Wireframe OR Svg)
	if (pDoc != NULL) {
		if (!g_useWireframe) {
			pContext->SetTransform(transform);
			pContext->DrawSvgDocument(pDoc);
			pContext->SetTransform(D2D1::Matrix3x2F::Identity());
		}
	}

	// 6. DRAW THE TEXT ON TOP
	if (isNumpad) {
		pContext->DrawTextW(topPart.c_str(), topPart.length(), pSmallFormat, topBox, pTextBrush);
		pContext->DrawTextW(bottomPart.c_str(), bottomPart.length(), pMainFormat, bottomBox, pTextBrush);
		pSmallFormat->Release();
	}
	else if (!textToDraw.empty()) {
		pContext->DrawTextW(wideText.c_str(), wideText.length(), pMainFormat, textBox, pTextBrush);
	}

	pMainFormat->Release();
	pTempCanvas->Release();
}


/* ---------------- FUNCTION TO RENDER THE BUTTON ON THE SCREEN ---------------- */
void RenderGraphics() {
	if (pContext == NULL) return;
	pContext->BeginDraw();
	pContext->Clear(g_bgColor);

	if (!gBtnLabel.empty() && !g_isResizing) {
		std::vector<std::string> keys = SplitCombo(gBtnLabel);

		float PosX = 50.0f;
		float PosY = 50.0f;

		for (size_t i = 0; i < keys.size(); i++) {
			std::string currentKey = keys[i];
			float keyWidth = 90.0f;
			float keyHeight = 90.0f;
			float drawY = PosY; // FIX: Create a temporary Y so we don't break the row!

			if (currentKey == "SPACE") {
				keyWidth = 400.0f;
				DrawSVGKey(pSvgSpace, currentKey, PosX, drawY, keyWidth, keyHeight, 42.0f, 1.0f);
			}
			else if (currentKey == "ENTER") {
				keyWidth = 200.0f;
				DrawSVGKey(pSvgEnter, currentKey, PosX, drawY, keyWidth, keyHeight, 42.0f, 1.0f);
			}
			else if (currentKey == "←") {
				keyWidth = 170.0f;
				DrawSVGKey(pSvgBackSpace, currentKey, PosX, drawY, keyWidth, keyHeight, 60.0f, 1.0f);
			}
			else if (currentKey == "CAPS") {
				keyWidth = 140.0f;
				DrawSVGKey(pSvgCaps, currentKey, PosX, drawY, keyWidth, keyHeight, 38.0f, 1.0f);
			}
			else if (currentKey == "TAB") {
				keyWidth = 120.0f;
				DrawSVGKey(pSvgTab, currentKey, PosX, drawY, keyWidth, keyHeight, 42.0f, 1.0f);
			}
			else if (currentKey == "SHIFT" || currentKey == "RIGHT\nSHIFT" || currentKey == "LEFT\nSHIFT") {
				keyWidth = 210.0f;
				DrawSVGKey(pSvgShift, currentKey, PosX, drawY, keyWidth, keyHeight, 28.0f, 1.0f);
			}
			else if (currentKey == "CTRL" || currentKey == "RIGHT\nCTRL" || currentKey == "LEFT\nCTRL") {
				keyWidth = 90.0f;
				DrawSVGKey(pSvgNormal, currentKey, PosX, drawY, keyWidth, keyHeight, 24.0f, 1.0f);
			}
			else if (currentKey == "ALT" || currentKey == "RIGHT\nALT" || currentKey == "LEFT\nALT") {
				keyWidth = 90.0f;
				DrawSVGKey(pSvgNormal, currentKey, PosX, drawY, keyWidth, keyHeight, 24.0f, 1.0f);
			}
			else if (currentKey == "PAGE\nUP" || currentKey == "PAGE\nDOWN") {
				DrawSVGKey(pSvgNormal, currentKey, PosX, drawY, keyWidth, keyHeight, 18.0f, 1.0f);
			}
			else if (currentKey == "PAUSE" || currentKey == "HOME") {
				DrawSVGKey(pSvgNormal, currentKey, PosX, drawY, keyWidth, keyHeight, 20.0f, 1.0f);
			}
			else if (currentKey == "ESC" || currentKey == "END" || currentKey == "INS" || currentKey == "DEL") {
				DrawSVGKey(pSvgNormal, currentKey, PosX, drawY, keyWidth, keyHeight, 28.0f, 1.0f);
			}
			else if (currentKey == "PRT\nSCR" || currentKey == "SCR\nLK") {
				DrawSVGKey(pSvgNormal, currentKey, PosX, drawY, keyWidth, keyHeight, 24.0f, 1.0f);
			}
			else if (currentKey.length() > 1 && currentKey[0] == 'F' && isdigit(currentKey[1])) {
				DrawSVGKey(pSvgNormal, currentKey, PosX, drawY, keyWidth, keyHeight, 38.0f, 1.0f);
			}
			else if (currentKey.starts_with("NUMPAD")) {
				DrawSVGKey(pSvgNormal, currentKey, PosX, drawY, keyWidth, keyHeight, 42.0f, 1.0f);
			}
			// --- MOUSE BUTTONS FIX ---
			else if (currentKey == "RIGHT CLICK") {
				keyWidth = g_useWireframe ? 80.0f : 180.0f; // Shrink the box if text is hidden
				if (!g_useWireframe) drawY = 50.0f; // Only shift SVGs up!
				DrawSVGKey(pSvgRClick, "", PosX, drawY, keyWidth, keyHeight, 42.0f, 1.0f);
			}
			else if (currentKey == "LEFT CLICK") {
				keyWidth = g_useWireframe ? 80.0f : 180.0f;
				if (!g_useWireframe) drawY = 50.0f;
				DrawSVGKey(pSvgLClick, "", PosX, drawY, keyWidth, 90.0f, 42.0f, 1.0f);
			}
			else if (currentKey == "MIDDLE CLICK") {
				keyWidth = g_useWireframe ? 80.0f : 180.0f;
				if (!g_useWireframe) drawY = 50.0f;
				DrawSVGKey(pSvgMClick, "", PosX, drawY, keyWidth, 90.0f, 42.0f, 1.0f);
			}
			else if (currentKey == "SCROLL UP") {
				keyWidth = g_useWireframe ? 80.0f : 180.0f;
				if (!g_useWireframe) drawY = 50.0f;
				DrawSVGKey(pSvgScrUP, "", PosX, drawY, keyWidth, keyHeight, 42.0f, 1.0f);
			}
			else if (currentKey == "SCROLL DOWN") {
				keyWidth = g_useWireframe ? 80.0f : 180.0f;
				if (!g_useWireframe) drawY = 50.0f;
				DrawSVGKey(pSvgScrDwn, "", PosX, drawY, keyWidth, keyHeight, 42.0f, 1.0f);
			}
			else {
				DrawSVGKey(pSvgNormal, currentKey, PosX, drawY, keyWidth, keyHeight, 42.0f, 1.0f);
			}

			PosX += keyWidth + 15.0f;

			if (i < keys.size() - 1) {
				keyWidth = 30.0f;
				// Always pass the base PosY so the plus sign stays centered vertically
				DrawSVGKey(NULL, "+", PosX, PosY, keyWidth, keyHeight, 42.0f, 1.0f);
				PosX += 30.0f + 15.0f;
			}
		}
	}

	pContext->SetTransform(D2D1::Matrix3x2F::Identity());
	pContext->EndDraw();
	pSwapChain->Present(1, 0);
}
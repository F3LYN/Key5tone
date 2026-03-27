/* OverlayWindow.cpp */
#include <algorithm>
#include <iostream>  // library to print text to the screen
#define NOMINMAX
#include <windows.h> // windows API
#include <string> // string library
#include <vector>
#include <dwrite.h>
#include <d2d1effects.h>
#include <winuser.h>
#include "OverlayWindow.h"
#include "KeyProcess.h"

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dwrite.lib")


ID2D1Factory1* pFactory = NULL;
ID3D11Device* pD3DDevice = NULL; // 3D Hardware Engine
IDXGISwapChain1* pSwapChain = NULL; // Allows to push the frames to the window
ID2D1DeviceContext5* pContext = NULL;
IDWriteFactory* pDWriteFactory = NULL;
ID2D1SolidColorBrush* pTextBrush = NULL;
ID2D1SolidColorBrush* pGlowBrush = NULL; // the glowing effect var
ID2D1Effect* pShapeGlowEffect = NULL;

// --- GLOBAL VARIABLE FOR RESIZING
float gAppScale = 1.0f; // tracks the windows scale (1.0 = normal)
bool g_isResizing = false; // tracks if the user is currently resizing the window


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
	// 1. Create the D2D Factory (Upgraded to Factory1)
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);

	// 2. Create the 3D Hardware Engine
	D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, NULL, 0, D3D11_SDK_VERSION, &pD3DDevice, NULL, NULL);

	// 3. Connect the 3D Engine to the 2D Factory to create our Context (Canvas)
	IDXGIDevice* pDXGIDevice;
	pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
	ID2D1Device* pD2DDevice;
	pFactory->CreateDevice(pDXGIDevice, &pD2DDevice);

	// create basic canvas
	ID2D1DeviceContext* pTempContext = NULL;
	pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pTempContext);

	// Upgrade into the global pContext variable
	pTempContext->QueryInterface(__uuidof(ID2D1DeviceContext5), (void**)&pContext);
	pTempContext->Release();

	// 4. Create the SwapChain (The delivery truck to the window)
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

	// 5. Connect the Canvas to the SwapChain
	IDXGISurface* pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&pBackBuffer);

	ID2D1Bitmap1* pTargetBitmap;
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);
	pContext->CreateBitmapFromDxgiSurface(pBackBuffer, &bitmapProperties, &pTargetBitmap);
	pContext->SetTarget(pTargetBitmap);

	// clean up pointers
	pTargetBitmap->Release();
	pBackBuffer->Release();

	pSvgNormal = LoadSVG(L"svg_files\\reg_key_btn.svg"); // Dimensions: 90 x 90
	pSvgSpace = LoadSVG(L"svg_files\\space_key_btn.svg"); // Dimensions: 400 x 90 
	pSvgEnter = LoadSVG(L"svg_files\\enter_key_btn.svg"); // Dimensions: 200 x 90
	pSvgShift = LoadSVG(L"svg_files\\shift_key_btn.svg"); // Dimensions: 220 x 90
	pSvgBackSpace= LoadSVG(L"svg_files\\bcksp_key_btn.svg"); // Dimensions: 180 x 90
	pSvgCaps = LoadSVG(L"svg_files\\caps_key_btn.svg"); // Dimensions: 140 x 90
	pSvgTab = LoadSVG(L"svg_files\\tab_key_btn.svg"); // Dimensions: 120 x 90
	pSvgNumpadTall = LoadSVG(L"svg_files\\nmpTL_key_btn.svg"); // Dimensions: 90 x 200
	pSvgNumpadWide = LoadSVG(L"svg_files\\nmpWD_key_btn.svg"); // Dimensions: 200 x 90
	pSvgNumpadWide = LoadSVG(L"svg_files\\nmpWD_key_btn.svg"); // Dimensions: 200 x 90
	pSvgNumpadWide = LoadSVG(L"svg_files\\nmpWD_key_btn.svg"); // Dimensions: 200 x 90

	pSvgRClick = LoadSVG(L"svg_files\\mouse_RClick.svg"); // Dimensions: 200 x 90
	pSvgLClick = LoadSVG(L"svg_files\\mouse_LClick.svg"); // Dimensions: 200 x 90
	pSvgMClick = LoadSVG(L"svg_files\\mouse_MClick.svg"); // Dimensions: 200 x 90
	pSvgScrUP = LoadSVG(L"svg_files\\mouse_ScrollUP.svg"); // Dimensions: 200 x 90
	pSvgScrDwn = LoadSVG(L"svg_files\\mouse_ScrollDOWN.svg"); // Dimensions: 200 x 90

	// 1. Create the Text Factory
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));


	// 4. Create a pure white brush for the text paint
	pContext->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 1.0f, 1.0f), &pTextBrush);

	pContext->CreateSolidColorBrush(D2D1::ColorF(1.f, 1.0f, 1.0f, 1.0f), &pGlowBrush);

	// Create the Shadow effect and configure it as an outline!
	pContext->CreateEffect(CLSID_D2D1Shadow, &pShapeGlowEffect);
	pShapeGlowEffect->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, 2.5f); // How wide the outline spreads
	pShapeGlowEffect->SetValue(D2D1_SHADOW_PROP_COLOR, D2D1::Vector4F(1.0f, 1.0f, 1.0f, 80.0f)); // white

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		RenderGraphics();
		ValidateRect(hwnd, NULL); // Critical: Tells Windows the "dirt" is gone
		return 0;
	case WM_ERASEBKGND:
		return 1; // Tell Windows: "Don't erase! I'm handling it with Direct2D."

	case WM_ENTERSIZEMOVE: // triggered when the user clicks the window  border
		g_isResizing = true;
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;

	case WM_EXITSIZEMOVE: // triggered when the user lets go off the window border
		g_isResizing = false;
		if (pContext != NULL) {
			ResizeGraphics(hwnd);
			InvalidateRect(hwnd, NULL, FALSE); // force the screen to redraw
		}
		return 0;

	case WM_SIZE:
		if (pContext != NULL && g_isResizing) {
			ResizeGraphics(hwnd);
			InvalidateRect(hwnd, NULL, FALSE);
		}

	case WM_TIMER:
		gBtnLabel = ""; // Clear the text
		InvalidateRect(hwnd, NULL, FALSE); // Redraw the empty green screen
		KillTimer(hwnd, 1); // Stop the timer so it doesn't loop
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


/* ---------------- FUNCTION TO REBUILD GRAPHICS WHEN WINDOW STRETCHES ---------------- */
void ResizeGraphics(HWND hwnd) {
	if (pContext == NULL || pSwapChain == NULL) return; // If the graphics isn't built yet, ignore and exit.

	pContext->SetTarget(NULL); // Tells the graphic Engine to drop the current canvas

	// Get the new physical size of the window
	RECT rc;
	GetClientRect(hwnd, &rc);
	UINT newWidth = rc.right - rc.left;
	UINT newHeight = rc.bottom - rc.top;
	std::cout << "hello";

	if (newWidth == 0 || newHeight == 0) return; // prevents crashing if the windows is 0

	float scaleX = (float)newWidth / 800.0f;
	float scaleY = (float)newHeight / 250.0f;

	gAppScale = std::min(scaleX, scaleY); // calculate new scale. 800 pixels is the default baseline.

	pSwapChain->ResizeBuffers(0, newWidth, newHeight, DXGI_FORMAT_UNKNOWN, 0); // resize the SwapChain to new pixel dimensions!

	// create new canvas at the exact pixel scale
	IDXGISurface* pBackBuffer = NULL;
	pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void**)&pBackBuffer);

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	ID2D1Bitmap1* pNewTargetBitmap = NULL;
	pContext->CreateBitmapFromDxgiSurface(pBackBuffer, &bitmapProperties, &pNewTargetBitmap);
	
	// Hook the canvas to the paint brush context
	pContext->SetTarget(pNewTargetBitmap);

	// Cleanup temporary pointers to avoid memory leaks
	pNewTargetBitmap->Release();
	pBackBuffer->Release();


}

HWND CreateOverlayWindow() {
	WNDCLASSEX wc = { 0 }; // Fills the blueprint with empty zeros to start
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WindowProc; // Connects the mailbox
	wc.hInstance = GetModuleHandle(NULL); // Gets the ID of our .e
	wc.lpszClassName = L"OverlayClass"; // The arbitrary name of our blueprint
	RegisterClassEx(&wc); // Hands the blueprint to Windows

	HWND hwnd = CreateWindowEx(
		0,        // Optional extension: Keeps the window always on top of other apps!
		L"OverlayClass",      // MUST exactly match your blueprint name
		L"Input Overlay",     // The text that will appear in the title bar
		WS_OVERLAPPEDWINDOW,  // The style: This gives you the standard title bar, borders, and X button to drag and close!
		CW_USEDEFAULT, CW_USEDEFAULT, // Starting X and Y position on the screen
		800, 250,             // Width and Height of the window
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

	// 1. Ask Windows how much space we need to translate this string
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);

	// 2. Create a blank wide string of that exact size
	std::wstring wstrTo(size_needed, 0);

	// 3. Tell Windows to actually do the translation into our blank string
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
	// Scale the buttons to the window context
	xPos *= gAppScale;
	yPos *= gAppScale;
	width *= gAppScale;
	height *= gAppScale;
	fontSize *= gAppScale;
	
	// 1. Draw SVG (Only if one was provided!)
	if (pDoc != NULL) {
		// A. Create an invisible temporary canvas
		ID2D1CommandList* pTempCanvas = NULL;
		pContext->CreateCommandList(&pTempCanvas);

		// B. Save the main screen, and switch our paintbrush to the temporary canvas
		ID2D1Image* pMainScreen = NULL;
		pContext->GetTarget(&pMainScreen);
		pContext->SetTarget(pTempCanvas);
		pContext->Clear(D2D1::ColorF(0, 0, 0, 0)); // Make the background fully transparent

		// C. Draw the SVG onto the temporary canvas
		D2D1_MATRIX_3X2_F transform = D2D1::Matrix3x2F::Scale(scale * gAppScale, scale * gAppScale) * D2D1::Matrix3x2F::Translation(xPos, yPos);
		pContext->SetTransform(transform);
		pContext->DrawSvgDocument(pDoc);

		// D. Close the canvas and switch back to the main screen
		pTempCanvas->Close();
		pContext->SetTarget(pMainScreen);
		pMainScreen->Release();

		// E. Feed our invisible canvas into the Shader, and draw the glow!
		pShapeGlowEffect->SetInput(0, pTempCanvas);
		pContext->SetTransform(D2D1::Matrix3x2F::Identity());
		pContext->DrawImage(pShapeGlowEffect);

		// F. Finally, draw the crisp, original SVG perfectly on top of the glow
		pContext->SetTransform(transform);
		pContext->DrawSvgDocument(pDoc);
		pContext->SetTransform(D2D1::Matrix3x2F::Identity());

		// G. Clean up the temporary canvas so we don't leak memory
		pTempCanvas->Release();
	}

	// 2. Setup Fonts
	IDWriteTextFormat* pMainFormat = NULL;
	pDWriteFactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", &pMainFormat);
	pMainFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pMainFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	size_t splitPos = textToDraw.find('\n');

	if (textToDraw.starts_with("NUMPAD") && splitPos != std::string::npos) {
		// --- DUAL TEXT MODE ---
		std::wstring topPart = ConvertUtf8ToWide(textToDraw.substr(0, splitPos));
		std::wstring bottomPart = ConvertUtf8ToWide(textToDraw.substr(splitPos + 1));

		IDWriteTextFormat* pSmallFormat = NULL;
		pDWriteFactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL, fontSize * 0.4f, L"en-us", &pSmallFormat);
		pSmallFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		pSmallFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		D2D1_RECT_F topBox = D2D1::RectF(xPos, yPos, xPos + (width * scale), yPos + (height * scale * 0.45f));
		D2D1_RECT_F bottomBox = D2D1::RectF(xPos, yPos - 10.0f + (height * scale * 0.35f), xPos + (width * scale), yPos + (height * scale));

		float glowSpread = 1.5f;
		for (int x = -2; x <= 2; x++) {
			for (int y = -2; y <= 2; y++) {
				if (x == 0 && y == 0) continue;
				D2D1_RECT_F gTopBox = D2D1::RectF(topBox.left + (x * glowSpread), topBox.top + (y * glowSpread), topBox.right + (x * glowSpread), topBox.bottom + (y * glowSpread));
				D2D1_RECT_F gBotBox = D2D1::RectF(bottomBox.left + (x * glowSpread), bottomBox.top + (y * glowSpread), bottomBox.right + (x * glowSpread), bottomBox.bottom + (y * glowSpread));

				pContext->DrawTextW(topPart.c_str(), topPart.length(), pSmallFormat, gTopBox, pGlowBrush);
				pContext->DrawTextW(bottomPart.c_str(), bottomPart.length(), pMainFormat, gBotBox, pGlowBrush);
			}
		}

		pContext->DrawTextW(topPart.c_str(), topPart.length(), pSmallFormat, topBox, pTextBrush);
		pContext->DrawTextW(bottomPart.c_str(), bottomPart.length(), pMainFormat, bottomBox, pTextBrush);

		pSmallFormat->Release();
	}
	else if (!textToDraw.empty()) {
		// --- NORMAL TEXT MODE ---
		std::wstring wideText = ConvertUtf8ToWide(textToDraw);
		D2D1_RECT_F textBox = D2D1::RectF(xPos, yPos, xPos + (width * scale), yPos + (height * scale));

		float glowSpread = 1.5f;
		for (int x = -2; x <= 2; x++) {
			for (int y = -2; y <= 2; y++) {
				if (x == 0 && y == 0) continue;
				D2D1_RECT_F glowBox = D2D1::RectF(
					textBox.left + (x * glowSpread), textBox.top + (y * glowSpread),
					textBox.right + (x * glowSpread), textBox.bottom + (y * glowSpread)
				);

				pContext->DrawTextW(wideText.c_str(), wideText.length(), pMainFormat, glowBox, pGlowBrush);
			}
		}
		pContext->DrawTextW(wideText.c_str(), wideText.length(), pMainFormat, textBox, pTextBrush);
	}

	pMainFormat->Release();
}


/* ---------------- FUNCTION TO RENDER THE BUTTON ON THE SCREEN ---------------- */
void RenderGraphics() {
	if (pContext == NULL) return;
	pContext->BeginDraw();
	pContext->Clear(D2D1::ColorF(0.1f, 1.0f, 0.1f));


	if (!gBtnLabel.empty() && !g_isResizing) { // only draw the keys if the buttons is pressed AND we are not currently dragging the window
		// 1. Slice the label into pieces!
		std::vector<std::string> keys = SplitCombo(gBtnLabel);

		// 2. Set our starting position
		float PosX = 50.0f;
		float PosY = 50.0f;

		// 3. Loop through each piece
		for (size_t i = 0; i < keys.size(); i++) {
			std::string currentKey = keys[i];
			float keyWidth = 90.0f; // default width assumption
			float keyHeight = 90.0f; // default height assumption

			if (currentKey == "SPACE") {
				keyWidth = 400.0f;
				DrawSVGKey(pSvgSpace, currentKey, PosX, PosY, keyWidth, keyHeight, 42.0f, 1.0f);
			}
			else if (currentKey == "ENTER") {
				keyWidth = 200.0f;
				DrawSVGKey(pSvgEnter, currentKey, PosX, PosY, keyWidth, keyHeight, 42.0f, 1.0f);
			}
			else if (currentKey == "←") {
				keyWidth = 170.0f;
				DrawSVGKey(pSvgBackSpace, currentKey, PosX, PosY, keyWidth, keyHeight, 60.0f, 1.0f);
			}
			else if (currentKey == "CAPS") {
				keyWidth = 140.0f;
				DrawSVGKey(pSvgCaps, currentKey, PosX, PosY, keyWidth, keyHeight, 38.0f, 1.0f);
			}
			else if (currentKey == "TAB") {
				keyWidth = 120.0f;
				DrawSVGKey(pSvgTab, currentKey, PosX, PosY, keyWidth, keyHeight, 42.0f, 1.0f);
			}
			else if (currentKey == "SHIFT" || currentKey == "RIGHT\nSHIFT" || currentKey == "LEFT\nSHIFT") {
				keyWidth = 210.0f;
				DrawSVGKey(pSvgShift, currentKey, PosX, PosY, keyWidth, keyHeight, 28.0f, 1.0f);
			}
			else if (currentKey == "CTRL" || currentKey == "RIGHT\nCTRL" || currentKey == "LEFT\nCTRL") {
				keyWidth = 90.0f;
				DrawSVGKey(pSvgNormal, currentKey, PosX, PosY, keyWidth, keyHeight, 24.0f, 1.0f);
			}
			else if (currentKey == "ALT" || currentKey == "RIGHT\nALT" || currentKey == "LEFT\nALT") {
				keyWidth = 90.0f;
				DrawSVGKey(pSvgNormal, currentKey, PosX, PosY, keyWidth, keyHeight, 24.0f, 1.0f);
			}
			else if (currentKey == "PAGE\nUP" || currentKey == "PAGE\nDOWN") {
				DrawSVGKey(pSvgNormal, currentKey, PosX, PosY, keyWidth, keyHeight, 18.0f, 1.0f);
			}
			else if (currentKey == "PAUSE" || currentKey == "HOME") {
				DrawSVGKey(pSvgNormal, currentKey, PosX, PosY, keyWidth, keyHeight, 20.0f, 1.0f);
			}
			else if (currentKey == "ESC" || currentKey == "END" || currentKey == "INS" || currentKey == "DEL") {
				DrawSVGKey(pSvgNormal, currentKey, PosX, PosY, keyWidth, keyHeight, 28.0f, 1.0f);
			}
			else if (currentKey == "PRT\nSCR" || currentKey == "SCR\nLK") {
				DrawSVGKey(pSvgNormal, currentKey, PosX, PosY, keyWidth, keyHeight, 24.0f, 1.0f);
			}
			else if (currentKey.length() > 1 && currentKey[0] == 'F' && isdigit(currentKey[1])) {
				DrawSVGKey(pSvgNormal, currentKey, PosX, PosY, keyWidth, keyHeight, 38.0f, 1.0f);
			}
			else if (currentKey.starts_with("NUMPAD")) {
				DrawSVGKey(pSvgNormal, currentKey, PosX, PosY, keyWidth, keyHeight, 42.0f, 1.0f);
			}
			else if (currentKey == "RIGHT CLICK") {
				keyWidth = 180.0f;
				PosY = 5.0f;
				DrawSVGKey(pSvgRClick, "", PosX, PosY, keyWidth, keyHeight, 42.0f, 2.0f);
			}
			else if (currentKey == "LEFT CLICK") {
				keyWidth = 180.0f;
				PosY = 5.0f;
				// Pass an empty string "" so no text is drawn!
				DrawSVGKey(pSvgLClick, "", PosX, PosY, keyWidth, 90.0f, 42.0f, 2.0f);
			}
			else if (currentKey == "SHIFT" || currentKey == "RIGHT\nSHIFT" || currentKey == "LEFT\nSHIFT" || currentKey == "CTRL" || currentKey == "MIDDLE CLICK") {
				keyWidth = 180.0f;
				PosY = 5.0f;
				DrawSVGKey(pSvgMClick, "", PosX, PosY, keyWidth, 90.0f, 42.0f, 2.0f);
			}
			else if (currentKey == "SCROLL UP") {
				keyWidth = 180.0f;
				PosY = 5.0f;
				DrawSVGKey(pSvgScrUP, "", PosX, PosY, keyWidth, keyHeight, 42.0f, 2.0f);
			}
			else if (currentKey == "SCROLL DOWN") {
				keyWidth = 180.0f;
				PosY = 5.0f;
				DrawSVGKey(pSvgScrDwn, "", PosX, PosY, keyWidth, keyHeight, 42.0f, 2.0f);
			}
			else {
				// Normal letters and default
				DrawSVGKey(pSvgNormal, currentKey, PosX, PosY, keyWidth, keyHeight, 42.0f, 1.0f);
			}

			// Advance the cursor to the right!
			PosX += keyWidth + 15.0f;

			if (i < keys.size() - 1) {
				keyWidth = 30.0f;
				// Pass NULL for the SVG so it only draws text!
				DrawSVGKey(NULL, "+", PosX, PosY, keyWidth, keyHeight, 42.0f, 1.0f);

				// Advance the cursor past the "+" sign
				PosX += 30.0f + 15.0f;
			}
		}
	}
	
	pContext->SetTransform(D2D1::Matrix3x2F::Identity());

	pContext->EndDraw();
	pSwapChain->Present(1, 0);
}
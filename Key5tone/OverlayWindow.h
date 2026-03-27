/* OverlayWindow.h */
#pragma once
#include <windows.h>
#include <d2d1_3.h> // direct 2D toolbox
#include <d3d11.h>
#include <dxgi1_2.h>
#include "OverlayWindow.h"
#include "shlwapi.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
# pragma comment(lib, "d2d1.lib") // link the Windows2D library file to the project

void DrawSVGKey(ID2D1SvgDocument* pDoc, float xPos, float yPos, float width, float height, float fontSize, float scale);
HWND CreateOverlayWindow();
void InitGraphics(HWND hwnd);
ID2D1CommandList* RecordSVGRendering(ID2D1SvgDocument* pDoc, float width, float height);
void RenderGraphics();
void ResizeGraphics(HWND hwnd); // function to resize the window
ID2D1SvgDocument* LoadSVG(LPCWSTR filepath);
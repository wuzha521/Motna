#pragma once
#include "‘§±‡“Î.h"
#include "Engine.h"
#include "Table.h"


#include "ImGui/imgui.h"
#include "ImGui//imgui_impl_win32.h"
#include "ImGui//imgui_impl_dx11.h"
#include <d3d11.h>



static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
typedef HRESULT (STDMETHODCALLTYPE* PPresent)(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
typedef LRESULT (WINAPI* PWndProc)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

float Lerp(float a, float b, float t);
void HookBegin(LPVOID* oFuncAddr, LPVOID pDetour);
void Dx11Hook(void);
void RenderClickableLink(const char* label, const char* url);

namespace Render
{
	void Line(Vector2 Begine, Vector2 End,ImColor Color ,float Width);
	void Text(Vector2 Screen, ImColor Color, const char* fmt, ...);
	void Rect(Vector2 Begine, Vector2 End, ImColor Color);
}

namespace Cheat
{

	void RenderPlayer(void);
	void RenderGoods(void);

};
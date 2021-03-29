#include <Windows.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <stdio.h>
#include "Weapons.h"

#include <fcntl.h> 

#include <io.h>
#include <fcntl.h> 
#include <cstdio>
#include <d3d9.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"


#include "termcolor.hpp"


#define A 220.00
#define As 233.0819
#define B 246.9417
#define C 261.6256
#define Cis 277.1826
#define D 293.6648
#define Dis 311.1270
#define E 329.6276
#define E2 164.81
#define F 174.61	
#define Fis 369.9944
#define G 392.00
#define G2 196.00
#define Gis 415.30
#define Gis2 207.65
#define H 246.94


#define FG_RED "\033[31m"
#define FG_PURPLE "\033[35m"
#define FG_GREEN "\033[32m"
#define FG_YELLOW "\033[33m"
#define FG_WHITE "\033[0m"
#define FG_GREY "\033[1;33m"
#define FG_WHITE "\033[1;37m"
#define FG_LTBL "\033[1;34m"


//int currentwep = 7;
bool ak47 = false;
bool lr300 = false;
bool mp5 = false;
bool thompson = false;
bool sar = false;
bool custom = false;
bool m249 = false;
int scope = 0;
int barrel = 0;
int randomizer = 70;
int playerfov = 90;
float playersens = 0.5;
bool enabled = false;
const float cala_nuta = 2000;
float deep = cala_nuta / 16;
float osemka = cala_nuta / 8;

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;

	return true;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

//styles randomize fxn

float Randomize(float val, int perc)
{
	LARGE_INTEGER time_mayne;
	QueryPerformanceCounter((LARGE_INTEGER*)&time_mayne);

	srand((unsigned long)(time_mayne.LowPart));
	float range = val * ((float)(perc) / 100);

	if (range <= 0.5) return val;
	if (range > 0.5) range = 1;

	int result = 1 + (rand() % (int)range);
	//beast = val + result;

	if ((1 + (rand() % 1) > 0)) return val + result;
	else return val + (result * -1);

}

void QuerySleep(int ms) // Sleep / Delay
{
	LONGLONG timerResolution;
	LONGLONG wantedTime;
	LONGLONG currentTime;

	QueryPerformanceFrequency((LARGE_INTEGER*)&timerResolution);
	timerResolution /= 1000;

	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	wantedTime = currentTime / timerResolution + ms;
	currentTime = 0;
	while (currentTime < wantedTime)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
		currentTime /= timerResolution;
	}
}


void Smoothing(double delay, double control_time, float x, float y) 
{
	int x_ = 1, y_ = 1, t_ = 0;
	for (int i = 1; i <= (int)control_time; ++i) 
	{
		int xI = i * x / (int)control_time;
		int yI = i * y / (int)control_time;
		int tI = i * (int)control_time / (int)control_time;
		mouse_event(1, (int)xI - (int)x_, (int)yI - (int)y_, 0, 0);
		QuerySleep((int)tI - (int)t_);
		x_ = xI; y_ = yI; t_ = tI;
	}
	QuerySleep((int)delay - (int)control_time);
}

//  another pasted smoothing solution

//void Smoothing(double delay, double control_time, int x, int y) {
//	int x_ = 1, y_ = 1, t_ = 0;
//	for (int i = 1; i <= (int)control_time; ++i) {
//		int xI = i * x / (int)control_time;
//		int yI = i * y / (int)control_time;
//		int tI = i * (int)control_time / (int)control_time;
//		mouse_event(1, (int)xI - (int)x_, (int)yI - (int)y_, 0, 0);
//		QuerySleep((int)tI - (int)t_);
//		x_ = xI; y_ = yI; t_ = tI;
//	}
//	QuerySleep((int)delay - (int)control_time);
//}

float getScope(float val)
{
	//none
	if (scope == 0)
		return val * 1.0;
	//holo
	if (scope == 1)
		return val * 1.2;
	//8x
	if (scope == 2)
		return val * 3.84;
	//16x
	if (scope == 3)
		return val * 7.68;
	//simple
	if (scope == 4)
		return val * 0.8;
	return val;
}

float getBarrel(float barval)
{
	if (barrel == 0)
		return barval * 1.0;
	//suppressor
	if (barrel == 1)
		return barval * 0.8;
	//boost **(WIP)**
	if (barrel == 2)
		return barval * 1.0;
	//break
	if (barrel == 3)
		return barval * 0.5;
}

float tofovandsens(float sens, int fov, float val)
{
	float a = (0.5 * fov * val) / ( sens * 90);
	
	return getScope(a);

}

// Main code
int main(int, char**)
{
	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"testing", NULL };
	::RegisterClassExW(&wc);
	HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"vera test", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	bool done = false;
	while (!done)
	{

		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		// Start the Dear ImGui frame
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();



		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("vera nignog");       


			
			ImGui::Text("Enable or disable script.");               // Display some text (you can use a format strings too)
			if (ImGui::Checkbox("Toggle on/off", &enabled))
			{
				if (enabled == false)
				{
					enabled == true;
					Beep(C, 80);
					Beep(E2, 80);
					Beep(G, 80);
				}
				else
				{

					Beep(G, 80);
					Beep(E2, 80);
					Beep(C, 80);
				}
			}
			ImGui::Text("scripts");
			if (ImGui::Checkbox("ak47", &ak47))
			{
				if (ak47 != true)
				{
					ak47 == true;
					Beep(E, 80);
				}
				else
				{
					ak47 == false;
					Beep(C, 80);
				}
			}

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::EndFrame();
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 0.0f), (int)(clear_color.y * clear_color.w * 0.0f), (int)(clear_color.z * clear_color.w * 0.0f), (int)(clear_color.w * 0.0f));
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 0.0f, 0);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}
		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

		// Handle loss of D3D9 device
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return 0;
}


int weapons()
{
	int count = 0;

	while (true)
	{
		//if (GetKeyState(VK_F2) & 0x8000)
		{
			if (ak47 != true)
			{
				ak47 = true;
			}
		}
		//if (GetKeyState(VK_F3) & 0x8000)
		//{
		//	if (currentwep != 1)
		//	{
		//		currentwep = 1;
		//	}
		//}
		//if (GetKeyState(VK_F4) & 0x8000)
		//{
		//	if (currentwep != 2)
		//	{
		//		currentwep = 2;
		//	}
		//}
		//if (GetKeyState(VK_F5) & 0x8000)
		//{
		//	if (currentwep != 3)
		//	{
		//		currentwep = 3;
		//	}
		//}
		//if (GetKeyState(VK_F6) & 0x8000)
		//{
		//	if (currentwep != 4)
		//	{
		//		currentwep = 4;

		//	}
		//}
		//if (GetKeyState(VK_F7) & 0x8000)
		//{
		//	if (currentwep != 5)
		//	{
		//		currentwep = 5;
		//	}
		//}
		//if (GetKeyState(VK_F8) & 0x8000)
		//{
		//	if (currentwep != 6)
		//	{
		//		currentwep = 6;
		//	}
		//}
		//if (GetAsyncKeyState(VK_INSERT) == -32767)
		//{
		//	enabled = !enabled;
		//}
		//if (GetAsyncKeyState(VK_LEFT) == -32767)
		//{
		//	scope = 0;
		//}
		//if (GetAsyncKeyState(VK_UP) == -32767)
		//{
		//	scope = 1;
		//}
		//if (GetAsyncKeyState(VK_DOWN) == -32767)
		//{
		//	barrel = 1;
		//}
		//if (GetAsyncKeyState(VK_RIGHT) == -32767)
		//{
		//	barrel = 0;
		//}
		//if (GetAsyncKeyState(VK_PRIOR) == -32767)
		//{
		//	scope = 2;
		//}
		if (enabled == true)
		{
			if (GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON))
			{

				switch (ak47)
				{
					if (count < Weapons::ak::pattern.size())
					{
						Smoothing(Weapons::ak::delay, Weapons::ak::controltime.at(count), Randomize(tofovandsens(playersens, playerfov, Weapons::ak::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::ak::pattern.at(count).y), randomizer));
						count++;
					}
					break;
				}
			}
		}
	}
}
				//case 1:
				//	if (count < Weapons::thompson::pattern.size())
				//	{
				//		Smoothing(Weapons::thompson::delay, Weapons::thompson::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::thompson::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::thompson::pattern.at(count).y), randomizer));
				//		count++;
				//	}
				//	break;
				//case 2:
				//	if (count < Weapons::smg::pattern.size())
				//	{
				//		Smoothing(Weapons::smg::delay, Weapons::smg::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::smg::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::smg::pattern.at(count).y), randomizer));
				//		count++;
				//	}
				//	break;
				//case 3:
				//	if (count < Weapons::lr::pattern.size())
				//	{
				//		Smoothing(Weapons::lr::delay, Weapons::lr::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::lr::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::lr::pattern.at(count).y), randomizer));
				//		count++;
				//	}
				//	break;
				//case 4:
				//	if (count < Weapons::mp5::pattern.size())
				//	{
				//		Smoothing(Weapons::mp5::delay, Weapons::mp5::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::mp5::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::mp5::pattern.at(count).y), randomizer));
				//		count++;
				//	}
				//	break;
				//case 5:
				//	if (count < Weapons::semi::pattern.size())
				//	{
				//		Smoothing(Weapons::semi::delay, Weapons::semi::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::semi::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::semi::pattern.at(count).y), randomizer));
				//	}
				//	break;
				//case 6:
				//	Smoothing(Weapons::m249::delay, Weapons::m249::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::m249::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::m249::pattern.at(count).y), randomizer));
				//	break;
				//default:
				//	break;
				//}

		//	}
		//	else
		//		count = 0;
		//}

//	}
//
//}
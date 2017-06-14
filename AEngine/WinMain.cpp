﻿#include"onwind.h"
#include"frameBuffering.h"
#include"NBody.h"
#include"ExecuteIndirect.h"
#include"DrawLine.h"
#include"Input.h"
#include"Screen.h"
#include"DrawCube.h"
using namespace std;

WNDCLASSEX wnd;
HWND window;
int state;
int screenw;
int screenh;

LRESULT WINAPI WinProc(HWND hwnd, unsigned int msg, WPARAM wParam, LPARAM lParam)
{
	D3D12AppBase* pD3dApp = reinterpret_cast<D3D12AppBase*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	switch (msg)
	{
	case(WM_CREATE):
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		break;
	}
	case(WM_DESTROY):
	{
		PostQuitMessage(0);
		break;
	}
	case(WM_SETFOCUS):
	{
		BaseInput::SetAcquire();
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		BaseInput::SetMousePosition(p);
		break;
	}
	case(WM_KILLFOCUS):
	{
		BaseInput::SetUnacquire();
		break;
	}
	case(WM_MOUSEMOVE):
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		BaseInput::SetMousePosition(p);
		break; 
	}
	case(WM_PAINT):
	{
		BaseInput::Update();
		if (pD3dApp)
		{
			pD3dApp->OnUpdate();
			pD3dApp->OnRender();
		}
		break;
	}
	case(WM_USER):
	{
		var cc = LOWORD(lParam);
		char* l = new char[1];
		l[0] = { (char)(cc + '0') };
		SetWindowText(hwnd, ToLPCWSTR(l));
		break;
	}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DeleteFile(_T("log.txt"));
	wstring windowTitle(_T("AEngine"));
	wstring windowClassName(_T("AWindow"));
	screenw = 1280;
	screenh = 720;

	D3D12AppBase* d3dApp = new DrawLine(window, screenw, screenh);
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	d3dApp->ParseCommandLineArgs(argv, argc);
	LocalFree(argv);

	wnd = { 0 };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.style = CS_HREDRAW | CS_VREDRAW;
	wnd.lpfnWndProc = WinProc;
	wnd.hInstance = hInstance;
	wnd.hIcon = NULL;
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.lpszClassName = windowClassName.c_str();

	RegisterClassEx(&wnd);

	RECT windowRect = { 0, 0, static_cast<LONG>(screenw), static_cast<LONG>(screenh) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	window = CreateWindow(
		windowClassName.c_str(),
		windowTitle.c_str(),
		WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_BORDER ^ WS_SIZEBOX,
		//WS_POPUPWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hInstance,
		d3dApp);

	if (window == NULL)
	{
		MessageBox(NULL, _T("Error"), _T("Error1"), 0);
		state = 1;
		return 0;
	}
	randomize();

	d3dApp->SetHwnd(window);
	d3dApp->OnInit();
	//POINT curPos;
	//GetCursorPos(&curPos);
	//ScreenToClient(window, &curPos);
	//BaseInput::Initialize(window, hInstance);
	//BaseInput::SetMousePosition(curPos);
	BaseInput::Initialize(window, hInstance);
	Screen::InitializeScreen(screenw, screenh);

	ShowWindow(window, nCmdShow);

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	BaseInput::Release();
	d3dApp->OnRelease();

	UnregisterClass(wnd.lpszClassName, hInstance);
	return 0;
}
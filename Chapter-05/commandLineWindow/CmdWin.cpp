#include<Windows.h>
#include<stdio.h>
#include<stdlib.h>

#define WIN_WIDTH	800
#define WIN_HEIGHT	600
#define LOG_FILE_NAME "AppLogFile.txt"
#define WRITE_IN_LOG_FILE(fmt,...){FILE* pF=NULL;if ((fopen_s(&pF, LOG_FILE_NAME, "a+")) == 0){fprintf_s(pF,fmt,##__VA_ARGS__);fclose(pF);pF=NULL;}}  

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

bool g_bWindowActive = false;
HWND g_hwnd = NULL;
HDC  g_hdc = NULL;

WINDOWPLACEMENT wpPrev;
DWORD dwStyle;
bool g_bFullScreen = false;

FILE* g_pFile = NULL;

int main(int argc, char** argv, char** envp)
{

	int Initialize(void);
	void Update(void);
	void Render(void);

	// Windowing Elelments
	WNDCLASSEX wndclass;
	MSG msg;
	HWND hwnd = NULL;
	TCHAR szClassName[] = TEXT("Sam_CMD");
	RECT windowRect;

	// Game Loop Control
	bool bDone = false;

	// Initialization Status
	int iInitRet = 0;

	HINSTANCE hInstance = GetModuleHandle(NULL);
	SecureZeroMemory((void*)&wndclass, sizeof(wndclass));
	wndclass.cbSize = sizeof(wndclass);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc = MainWndProc;
	wndclass.lpszClassName = szClassName;
	wndclass.lpszMenuName = NULL;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(hInstance, IDC_ARROW);

	if (!RegisterClassEx(&wndclass))
	{
		MessageBox(NULL, TEXT("Issue...!!!"), TEXT("Could Not RegisterClass() "), MB_OK | MB_ICONERROR);
		exit(EXIT_FAILURE);
	}

	if ((fopen_s(&g_pFile, LOG_FILE_NAME, "w+")) == 0)
	{
		fprintf_s(g_pFile, "File Opened Successfully. \n");
		fclose(g_pFile);
		g_pFile = NULL;
	}
	else
	{
		MessageBox(NULL, TEXT("Issue...!!!"), TEXT("Could not open File"), MB_OK | MB_ICONERROR);
		exit(EXIT_FAILURE);
	}


	// Adjust the client area with actual value WIN_WIDTH and WIN_HEIGHT as frambuffers will be made with those values.
	SecureZeroMemory((void*)&windowRect, sizeof(windowRect));
	windowRect.left = 0;
	windowRect.top = 0;
	windowRect.bottom = WIN_HEIGHT;
	windowRect.right = WIN_WIDTH;
	AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW);
	int x = GetSystemMetrics(SM_CXFULLSCREEN);
	int y = GetSystemMetrics(SM_CYFULLSCREEN);

	// Window at center of screen
	int iWinStartX = (x - windowRect.right) / 2;
	int iWinStartY = (y - windowRect.bottom) / 2;

	hwnd = CreateWindowEx(WS_EX_APPWINDOW, szClassName,
		TEXT("First_CMD_Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		iWinStartX, iWinStartY,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, TEXT("Issue...!!!"), TEXT("Could Not CreateWindow() "), MB_OK | MB_ICONERROR);
		exit(EXIT_FAILURE);
	}
	else
	{
		WRITE_IN_LOG_FILE("Window Created\n");
	}

	g_hwnd = hwnd;

	iInitRet = Initialize();
	if (iInitRet == EXIT_FAILURE)
	{
		MessageBox(NULL, TEXT("Issue...!!!"), TEXT("Could Not Initialize() "), MB_OK | MB_ICONERROR);
		WRITE_IN_LOG_FILE("Initialize Failed. RetVal => %d\n", iInitRet);
		exit(EXIT_FAILURE);
	}
	else
	{
		WRITE_IN_LOG_FILE("Initialize Complete\n");
	}


	ShowWindow(hwnd, SW_SHOWNORMAL);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (g_bWindowActive)
			{
				Update();
			}
			// Show all Animations
			Render();

		}
	}

	return ((int)msg.wParam);
}


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int UnInitialize(void);
	void FullScreen(void);
	bool Resize(int, int);

	switch (iMsg)
	{
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		{
			switch (LOWORD(wParam))
			{
			case VK_ESCAPE:
				DestroyWindow(hwnd);
				break;

			case 0x46: // 'f' or 'F'
			//MessageBox(hwnd, TEXT("F is pressed"), TEXT("Status"), MB_OK);
				FullScreen();
				break;
			}
		}
		break;

	case WM_MOUSEMOVE:
		break;
	case WM_SIZE:
		//g_fCurrrentWidth = LOWORD(lParam);
		//g_fCurrrentHeight = HIWORD(lParam);
		//WRITE_IN_LOG_FILE("In WM_SIZE()\n");

		Resize(LOWORD(lParam), HIWORD(lParam));
		break;
	//case WM_ERASEBKGND:
		//return(0);
		//break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		UnInitialize();
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

bool Resize(int iWidth, int iHeight)
{
	if (iHeight <= 0)
	{
		iHeight = 1;
	}

	return true;
}

void FullScreen(void)
{
	MONITORINFO mi = { sizeof(mi) };
	dwStyle = GetWindowLong(g_hwnd, GWL_STYLE);
	if (g_bFullScreen == false)
	{
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(g_hwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(g_hwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(g_hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(g_hwnd, HWND_TOP,
					mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
		g_bFullScreen = true;
	}
	else
	{
		SetWindowLong(g_hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(g_hwnd, &wpPrev);
		SetWindowPos(g_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
		ShowCursor(TRUE);
		g_bFullScreen = false;
	}
}

void Update(void)
{

}

void Render(void)
{

}

int Initialize(void)
{
	
	return EXIT_SUCCESS;
}

int UnInitialize(void)
{

	if (g_bFullScreen == true)
	{
		SetWindowLong(g_hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(g_hwnd, &wpPrev);
		SetWindowPos(g_hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
		ShowCursor(TRUE);
		g_bFullScreen = false;
	}

	WRITE_IN_LOG_FILE("Closing the Log File\n");

	return EXIT_SUCCESS;
}

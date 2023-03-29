#include <Windows.h>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define LOG_FILE_NAME "AppLogFile.txt"
#define WRITE_IN_LOG_FILE(fmt, ...)                 \
	{                                               \
		FILE *pF = NULL;                            \
		if (fopen_s(&pF, LOG_FILE_NAME, "a+") == 0) \
		{                                           \
			fprintf_s(pF, fmt, ##__VA_ARGS__);      \
			fclose(pF);                             \
			pF = NULL;                              \
		}                                           \
	}

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "vulkan-1.lib")

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

bool g_bWindowActive = false;
HWND g_hwnd = NULL;
HDC g_hdc = NULL;

WINDOWPLACEMENT wpPrev;
DWORD dwStyle;
bool g_bFullScreen = false;

FILE *g_pFile = NULL;

#pragma region VulkanVariables
VkInstance g_vlkInstance = VK_NULL_HANDLE;
VkPhysicalDevice g_vlkReqPhysicalDevice = VK_NULL_HANDLE;
VkSurfaceKHR g_vlkSurface = VK_NULL_HANDLE;
VkDevice g_vlkDevice = VK_NULL_HANDLE;

uint32_t g_iPresentationQueueID = 0;
#pragma endregion

int main(int argc, char **argv, char **envp)
{

	int Initialize(HINSTANCE, HWND);
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
	SecureZeroMemory((void *)&wndclass, sizeof(wndclass));
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
	SecureZeroMemory((void *)&windowRect, sizeof(windowRect));
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

	iInitRet = Initialize(hInstance, hwnd);
	if (iInitRet == EXIT_FAILURE)
	{
		MessageBox(NULL, TEXT("Issue...!!!"), TEXT("Could Not Initialize() "), MB_OK | MB_ICONERROR);
		WRITE_IN_LOG_FILE("Initialize Failed. RetVal => %d\n", iInitRet);
		DestroyWindow(hwnd);
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
	bool Resize(HWND, int, int);

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
				   // MessageBox(hwnd, TEXT("F is pressed"), TEXT("Status"), MB_OK);
			FullScreen();
			break;
		}
	}
	break;

	case WM_MOUSEMOVE:
		break;
	case WM_SIZE:
		// g_fCurrrentWidth = LOWORD(lParam);
		// g_fCurrrentHeight = HIWORD(lParam);
		WRITE_IN_LOG_FILE("In WM_SIZE()\n");

		if (!Resize(hwnd, LOWORD(lParam), HIWORD(lParam)))
		{
			WRITE_IN_LOG_FILE("Something Failed in Resize()\n");
			DestroyWindow(hwnd);
		}
		break;
	// case WM_ERASEBKGND:
	// return(0);	// Screen will be painted by Vulkan not by Win32 SDK.
	// break;
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

bool Resize(HWND hWnd, int iWidth, int iHeight)
{
	if (iHeight <= 0)
	{
		iHeight = 1;
	}

	if (g_vlkDevice)
	{
		if (g_vlkSurface)
		{
			vkDestroySurfaceKHR(g_vlkInstance, g_vlkSurface, nullptr);
			g_vlkSurface = VK_NULL_HANDLE;
		}

		VkResult vlkRes;
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.pNext = nullptr;
		surfaceCreateInfo.flags = 0;
		surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
		surfaceCreateInfo.hwnd = hWnd;
		vlkRes = vkCreateWin32SurfaceKHR(g_vlkInstance, &surfaceCreateInfo, nullptr, &g_vlkSurface);
		if (vlkRes != VK_SUCCESS)
		{
			WRITE_IN_LOG_FILE("Failed to Create Win32 Surface\n");
			return false;
		}
		else
		{
			WRITE_IN_LOG_FILE("Win32 Surface Createion Complete\n");
		}
	}

	return true;
}

void FullScreen(void)
{
	MONITORINFO mi = {sizeof(mi)};
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

int Initialize(HINSTANCE hInstance, HWND hWnd)
{
	VkResult vlkRes{};

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pApplicationName = "NoApp";
	appInfo.pEngineName = "NoEngine";

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	uint32_t iInstanceLayerCount = 0;
	std::vector<VkLayerProperties> instanceLayerList;
	std::vector<char *> instanceLayerNames;

	vkEnumerateInstanceLayerProperties(&iInstanceLayerCount, nullptr);
	instanceLayerList.resize(iInstanceLayerCount);
	vkEnumerateInstanceLayerProperties(&iInstanceLayerCount, instanceLayerList.data());
	WRITE_IN_LOG_FILE("Instance Layer Names: \n");
	for (size_t iIdx = 0; iIdx < instanceLayerList.size(); iIdx++)
	{
		WRITE_IN_LOG_FILE("%s\n", instanceLayerList[iIdx].layerName);
		if (strcmp(instanceLayerList[iIdx].layerName, "VK_LAYER_KHRONOS_validation") == 0)
		{
			instanceLayerNames.push_back(instanceLayerList[iIdx].layerName);
		}

		// Suppourt for debug messages
		/*if (strcmp(instanceLayerList[iIdx].layerName, "VK_LAYER_LUNARG_api_dump") == 0)
		{
		   instanceLayerNames.push_back(instanceLayerList[iIdx].layerName);
		}*/
	}
	instanceLayerList.shrink_to_fit();
	instanceLayerNames.shrink_to_fit();
	WRITE_IN_LOG_FILE("\n");

	instanceCreateInfo.enabledLayerCount = (uint32_t)instanceLayerNames.size();
	instanceCreateInfo.ppEnabledLayerNames = instanceLayerNames.data();

	uint32_t iInstanceExtensionCount = 0;
	std::vector<VkExtensionProperties> instanceExtensionList;
	std::vector<char *> instanceExtensionNames;

	vkEnumerateInstanceExtensionProperties(nullptr, &iInstanceExtensionCount, nullptr);
	instanceExtensionList.resize(iInstanceExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &iInstanceExtensionCount, instanceExtensionList.data());

	WRITE_IN_LOG_FILE("Instance Extension Names: \n");
	for (size_t i = 0; i < instanceExtensionList.size(); i++)
	{
		WRITE_IN_LOG_FILE("%s\n", instanceExtensionList[i].extensionName);
		instanceExtensionNames.push_back(instanceExtensionList[i].extensionName);
	}
	instanceExtensionList.shrink_to_fit();
	instanceExtensionNames.shrink_to_fit();
	WRITE_IN_LOG_FILE("\n");

	instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionNames.data();

	vlkRes = vkCreateInstance(&instanceCreateInfo, nullptr, &g_vlkInstance);
	if (vlkRes != VK_SUCCESS)
	{
		WRITE_IN_LOG_FILE("Failed to Create Instance\n");
		return EXIT_FAILURE;
	}
	else
	{
		WRITE_IN_LOG_FILE("Create Instance Complete\n");
	}

	std::vector<VkPhysicalDevice> physicalDeviceList;
	uint32_t iPhysicalDevices = 0;
	vkEnumeratePhysicalDevices(g_vlkInstance, &iPhysicalDevices, nullptr);
	physicalDeviceList.resize(iPhysicalDevices);
	vkEnumeratePhysicalDevices(g_vlkInstance, &iPhysicalDevices, physicalDeviceList.data());

	physicalDeviceList.shrink_to_fit();

	for (size_t iDeviceId = 0; iDeviceId < physicalDeviceList.size(); iDeviceId++)
	{
		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(physicalDeviceList[iDeviceId], &deviceProperties);

		// Add further properties to modify the device selection.

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			// Check if physical Device Suppourts Presentation.
			// At least 1 Queue must be there for Presentation.
			uint32_t iQueuFamilyPropertiesCount = 0;
			std::vector<VkQueueFamilyProperties> queueFamilyPropList;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDeviceList[iDeviceId], &iQueuFamilyPropertiesCount, nullptr);
			queueFamilyPropList.resize(iQueuFamilyPropertiesCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDeviceList[iDeviceId], &iQueuFamilyPropertiesCount, queueFamilyPropList.data());

			VkBool32 bPresentationSuppourt = VK_FALSE;

			for (size_t iIdx = 0; iIdx < queueFamilyPropList.size(); iIdx++)
			{
				// vkGetPhysicalDeviceSurfaceSupportKHR(physicalDeviceList[iDeviceId], iIdx, g_vlkSurface, &bPresentationSuppourt);
				bPresentationSuppourt = vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDeviceList[iDeviceId], iIdx);
				WRITE_IN_LOG_FILE("Queue family Index = %zd, %s presentation\n", iIdx, bPresentationSuppourt ? "Supports" : "Does not Support");
				if (bPresentationSuppourt)
				{
					g_iPresentationQueueID = iIdx;
					g_vlkReqPhysicalDevice = physicalDeviceList[iDeviceId];
					break; // 1st device to suppourt all requirements.
				}
			}
		}
	}

	if (g_vlkReqPhysicalDevice == VK_NULL_HANDLE)
	{
		WRITE_IN_LOG_FILE("Could not get desired physical device\n");
		return EXIT_FAILURE;
	}
	else
	{
		WRITE_IN_LOG_FILE("Obtained desired physical device\n");
		WRITE_IN_LOG_FILE("Device ID = %lld\n", (int64_t)g_vlkReqPhysicalDevice);
	}

	uint32_t iQueuFamilyPropertiesCount = 0;
	std::vector<VkQueueFamilyProperties> queueFamilyPropList;
	vkGetPhysicalDeviceQueueFamilyProperties(g_vlkReqPhysicalDevice, &iQueuFamilyPropertiesCount, nullptr);
	queueFamilyPropList.resize(iQueuFamilyPropertiesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(g_vlkReqPhysicalDevice, &iQueuFamilyPropertiesCount, queueFamilyPropList.data());

	for (uint32_t i = 0; i < queueFamilyPropList.size(); i++)
	{
	}

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

	if (g_vlkDevice)
	{
		vkDeviceWaitIdle(g_vlkDevice);

		vkDestroyDevice(g_vlkDevice, nullptr); // Destroyes queues as well...!!
		g_vlkDevice = VK_NULL_HANDLE;
		WRITE_IN_LOG_FILE("Destroyed Device\n");
	}

	if (g_vlkInstance)
	{
		if (g_vlkSurface)
		{
			vkDestroySurfaceKHR(g_vlkInstance, g_vlkSurface, nullptr);
			g_vlkSurface = VK_NULL_HANDLE;
			WRITE_IN_LOG_FILE("Destroyed Surface\n");
		}

		vkDestroyInstance(g_vlkInstance, nullptr);
		g_vlkInstance = VK_NULL_HANDLE;
		WRITE_IN_LOG_FILE("Destroyed Instance\n");
	}

	WRITE_IN_LOG_FILE("Closing the Log File\n");

	return EXIT_SUCCESS;
}

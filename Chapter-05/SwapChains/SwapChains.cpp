#include <Windows.h>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#define NUM_FRAMES 3
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
VkDevice g_vlkDevice = VK_NULL_HANDLE;
VkSurfaceKHR g_vlkSurface = VK_NULL_HANDLE;

VkQueue g_vlkCommandQueue = VK_NULL_HANDLE;
VkQueue g_vlkPresentationQueue = VK_NULL_HANDLE;

VkCommandPool g_vlkCmdCommandPool = VK_NULL_HANDLE;
VkCommandBuffer g_vlkCmdCommandBuffer = VK_NULL_HANDLE;

VkCommandPool g_vlkPresentCommandPool = VK_NULL_HANDLE;
VkCommandBuffer g_vlkPresentCommandBuffer = VK_NULL_HANDLE;

uint32_t g_PresentationQFamilyIdx = 0;
uint32_t g_CommandQFamilyIdx = 0;

VkSwapchainKHR g_vlkSwapChain = VK_NULL_HANDLE;
std::vector<VkImage> g_vlkSwapChainImages;
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

	printf_s("In Resize()\n");
	if (iHeight <= 0)
	{
		iHeight = 1;
	}

	if (g_vlkDevice)
	{
		VkResult vlkRes{};

		// vkDeviceWaitIdle(g_vlkDevice);

		vkQueueWaitIdle(g_vlkPresentationQueue);

		if ((g_vlkPresentCommandBuffer != VK_NULL_HANDLE) && (g_vlkPresentCommandPool != VK_NULL_HANDLE))
		{
			vkFreeCommandBuffers(g_vlkDevice, g_vlkPresentCommandPool, 1, &g_vlkPresentCommandBuffer);
			g_vlkPresentCommandBuffer = VK_NULL_HANDLE;

			vkDestroyCommandPool(g_vlkDevice, g_vlkPresentCommandPool, nullptr);
			g_vlkPresentCommandPool = VK_NULL_HANDLE;
		}
		if (g_vlkSwapChainImages.size())
		{
			g_vlkSwapChainImages.clear();
			g_vlkSwapChainImages.shrink_to_fit();
		}

		if (g_vlkSwapChain)
		{
			vkDestroySwapchainKHR(g_vlkDevice, g_vlkSwapChain, nullptr);
			g_vlkSwapChain = VK_NULL_HANDLE;
		}

		if (g_vlkSurface)
		{
			vkDestroySurfaceKHR(g_vlkInstance, g_vlkSurface, nullptr);
			g_vlkSurface = VK_NULL_HANDLE;
		}

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

		VkSurfaceCapabilitiesKHR surfaceCapabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_vlkReqPhysicalDevice, g_vlkSurface, &surfaceCapabilities);

		// Find suppourted surface format
		uint32_t iSurfaceFormatCount = 0;
		std::vector<VkSurfaceFormatKHR> surfaceFormatList;
		VkSurfaceFormatKHR desiredSurfaceFormat{};
		vkGetPhysicalDeviceSurfaceFormatsKHR(g_vlkReqPhysicalDevice, g_vlkSurface, &iSurfaceFormatCount, nullptr);
		surfaceFormatList.resize(iSurfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(g_vlkReqPhysicalDevice, g_vlkSurface, &iSurfaceFormatCount, surfaceFormatList.data());

		for (size_t iIdx = 0; iIdx < surfaceFormatList.size(); iIdx++)
		{
			if (surfaceFormatList[iIdx].format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				desiredSurfaceFormat = surfaceFormatList[iIdx];
			}
		}

		// Find Supported Present Mode
		uint32_t iPresentModeCount = 0;
		std::vector<VkPresentModeKHR> presentModesList;
		VkPresentModeKHR desiredPresentMode{};
		vkGetPhysicalDeviceSurfacePresentModesKHR(g_vlkReqPhysicalDevice, g_vlkSurface, &iPresentModeCount, nullptr);
		presentModesList.resize(iPresentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(g_vlkReqPhysicalDevice, g_vlkSurface, &iPresentModeCount, presentModesList.data());

		for (size_t iIdx = 0; iIdx < presentModesList.size(); iIdx++)
		{
			if (presentModesList[iIdx] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				desiredPresentMode = presentModesList[iIdx];
				break;
			}
		}

		if (!(surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
			WRITE_IN_LOG_FILE("swap chain image does not support VK_IMAGE_TRANSFER_DST usage\n");
		}

		// Surface Transformation
		VkSurfaceTransformFlagBitsKHR surfaceTraansformBit{};
		if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			surfaceTraansformBit = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			surfaceTraansformBit = surfaceCapabilities.currentTransform;
		}

		VkSwapchainCreateInfoKHR swapChainCreateInfo{};
		swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainCreateInfo.flags = 0;
		swapChainCreateInfo.pNext = nullptr;
		swapChainCreateInfo.surface = g_vlkSurface;
		swapChainCreateInfo.minImageCount = ((NUM_FRAMES < surfaceCapabilities.maxImageCount) && (NUM_FRAMES >= surfaceCapabilities.minImageCount)) ? NUM_FRAMES : surfaceCapabilities.minImageCount;
		swapChainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
		swapChainCreateInfo.imageFormat = desiredSurfaceFormat.format;
		swapChainCreateInfo.imageColorSpace = desiredSurfaceFormat.colorSpace;
		swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.imageArrayLayers = 1;
		swapChainCreateInfo.preTransform = surfaceTraansformBit;
		swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainCreateInfo.presentMode = desiredPresentMode;
		swapChainCreateInfo.clipped = VK_TRUE;

		vlkRes = vkCreateSwapchainKHR(g_vlkDevice, &swapChainCreateInfo, nullptr, &g_vlkSwapChain);
		if (vlkRes != VK_SUCCESS)
		{
			WRITE_IN_LOG_FILE("Failed to Create Swapchain\n");
			return false;
		}
		else
		{
			WRITE_IN_LOG_FILE("Swapchain Createion Complete\n");
		}

		// Get Images from SwapChain
		uint32_t iSwapchainImagesCount = 0;
		vkGetSwapchainImagesKHR(g_vlkDevice, g_vlkSwapChain, &iSwapchainImagesCount, nullptr);
		g_vlkSwapChainImages.resize(iSwapchainImagesCount);
		vkGetSwapchainImagesKHR(g_vlkDevice, g_vlkSwapChain, &iSwapchainImagesCount, g_vlkSwapChainImages.data());

		if (g_vlkSwapChainImages.data())
		{
			WRITE_IN_LOG_FILE("Obtained Swapchain Images\n");
		}

		VkCommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext = nullptr;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		commandPoolCreateInfo.queueFamilyIndex = g_PresentationQFamilyIdx;
		vlkRes = vkCreateCommandPool(g_vlkDevice, &commandPoolCreateInfo, nullptr, &g_vlkPresentCommandPool);
		if (vlkRes != VK_SUCCESS)
		{
			WRITE_IN_LOG_FILE("Failed to Create Present Command Pool\n");
			return EXIT_FAILURE;
		}

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = g_vlkPresentCommandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1;
		vlkRes = vkAllocateCommandBuffers(g_vlkDevice, &commandBufferAllocateInfo, &g_vlkPresentCommandBuffer);
		if (vlkRes != VK_SUCCESS)
		{
			WRITE_IN_LOG_FILE("Failed to Obtain Present Command Buffers");
			return EXIT_FAILURE;
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
	VkResult vlkRes;

	// Reset Command Buffers
	vlkRes = vkResetCommandBuffer(g_vlkCmdCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	vlkRes = vkResetCommandBuffer(g_vlkPresentCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	// Record Command buffers
	VkCommandBufferBeginInfo cmdCommandBufferBeginInfo{};
	cmdCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdCommandBufferBeginInfo.pNext = nullptr;
	cmdCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT re-use of command buffers
	cmdCommandBufferBeginInfo.pInheritanceInfo = nullptr;
	vlkRes = vkBeginCommandBuffer(g_vlkCmdCommandBuffer, &cmdCommandBufferBeginInfo);
	if (vlkRes != VK_SUCCESS)
	{
		std::cout << "Failed to Record Cmd Command Buffer" << std::endl;
	}

	vlkRes = vkEndCommandBuffer(g_vlkCmdCommandBuffer);
	if (vlkRes != VK_SUCCESS)
	{
		std::cout << "Failed to End Recording Cmd Command Buffer" << std::endl;
	}

	VkCommandBufferBeginInfo presentCommandBufferBeginInfo{};
	presentCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	presentCommandBufferBeginInfo.pNext = nullptr;
	presentCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT re-use of command buffers
	presentCommandBufferBeginInfo.pInheritanceInfo = nullptr;
	vlkRes = vkBeginCommandBuffer(g_vlkPresentCommandBuffer, &presentCommandBufferBeginInfo);
	if (vlkRes != VK_SUCCESS)
	{
		std::cout << "Failed to Record Present Command Buffer" << std::endl;
	}

	vlkRes = vkEndCommandBuffer(g_vlkPresentCommandBuffer);
	if (vlkRes != VK_SUCCESS)
	{
		std::cout << "Failed to End Recording Present Command Buffer" << std::endl;
	}

	// Submit command buffers
	VkSubmitInfo cmdCommandSubmitInfo{};
	cmdCommandSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	cmdCommandSubmitInfo.pNext = nullptr;
	cmdCommandSubmitInfo.commandBufferCount = 1;
	cmdCommandSubmitInfo.pCommandBuffers = &g_vlkCmdCommandBuffer;
	vlkRes = vkQueueSubmit(g_vlkCommandQueue, 1, &cmdCommandSubmitInfo, VK_NULL_HANDLE);
	if (vlkRes != VK_SUCCESS)
	{
		std::cout << "Failed to submit cmd Command Queue" << std::endl;
	}

	VkSubmitInfo presentCommandSubmitInfo{};
	presentCommandSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	presentCommandSubmitInfo.pNext = nullptr;
	presentCommandSubmitInfo.commandBufferCount = 1;
	presentCommandSubmitInfo.pCommandBuffers = &g_vlkPresentCommandBuffer;
	vlkRes = vkQueueSubmit(g_vlkPresentationQueue, 1, &presentCommandSubmitInfo, VK_NULL_HANDLE);
	if (vlkRes != VK_SUCCESS)
	{
		std::cout << "Failed to submit present Command Queue" << std::endl;
	}

	vkQueueWaitIdle(g_vlkCommandQueue);
	vkQueueWaitIdle(g_vlkPresentationQueue);
}

int Initialize(HINSTANCE hInstance, HWND hWnd)
{
	printf_s("In Initialize()\n");
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

			for (size_t iIdx = 0; iIdx < queueFamilyPropList.size(); iIdx++)
			{
				VkBool32 bPresentationSuppourt = VK_FALSE;
				// vkGetPhysicalDeviceSurfaceSupportKHR(physicalDeviceList[iDeviceId], iIdx, g_vlkSurface, &bPresentationSuppourt);
				bPresentationSuppourt = vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDeviceList[iDeviceId], iIdx);
				WRITE_IN_LOG_FILE("Queue family Index = %zd, %s presentation\n", iIdx, bPresentationSuppourt ? "Supports" : "Does not Support");
				if (bPresentationSuppourt)
				{
					g_PresentationQFamilyIdx = iIdx;
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
		if (queueFamilyPropList[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
		{
			g_CommandQFamilyIdx = i;
			break;
		}
	}

	float fQueuePriorities[] = {1.0f, 1.0}; // This is for each queue, we are asking
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pNext = nullptr;
	queueCreateInfo.flags = 0;
	queueCreateInfo.queueCount = 2;
	queueCreateInfo.queueFamilyIndex = g_CommandQFamilyIdx;
	queueCreateInfo.pQueuePriorities = fQueuePriorities;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

	VkPhysicalDeviceFeatures physicalDeviceFeatures{};
	vkGetPhysicalDeviceFeatures(g_vlkReqPhysicalDevice, &physicalDeviceFeatures);
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

	uint32_t iDeviceLayerCount = 0;
	std::vector<VkLayerProperties> deviceLayerList;
	std::vector<char *> devLayerListNames;

	vkEnumerateDeviceLayerProperties(g_vlkReqPhysicalDevice, &iDeviceLayerCount, nullptr);
	deviceLayerList.resize(iDeviceLayerCount);
	vkEnumerateDeviceLayerProperties(g_vlkReqPhysicalDevice, &iDeviceLayerCount, deviceLayerList.data());

	WRITE_IN_LOG_FILE("Device Layer Names: \n");
	for (size_t i = 0; i < deviceLayerList.size(); i++)
	{
		devLayerListNames.push_back(deviceLayerList[i].layerName);
		WRITE_IN_LOG_FILE("%s\n", deviceLayerList[i].layerName);
	}

	deviceCreateInfo.enabledLayerCount = (uint32_t)devLayerListNames.size();
	deviceCreateInfo.ppEnabledLayerNames = devLayerListNames.data();

	uint32_t iDeviceExtensionCount = 0;
	std::vector<VkExtensionProperties> deviceExtList;
	std::vector<char *> devExtNames;
	vkEnumerateDeviceExtensionProperties(g_vlkReqPhysicalDevice, nullptr, &iDeviceExtensionCount, nullptr);
	deviceExtList.resize(iDeviceExtensionCount);
	vkEnumerateDeviceExtensionProperties(g_vlkReqPhysicalDevice, nullptr, &iDeviceExtensionCount, deviceExtList.data());

	WRITE_IN_LOG_FILE("Device Extension Names: \n");
	for (size_t i = 0; i < deviceExtList.size(); i++)
	{
		if (strcmp("VK_EXT_buffer_device_address", deviceExtList[i].extensionName) == 0)
		{
			continue;
		}
		devExtNames.push_back(deviceExtList[i].extensionName);
		WRITE_IN_LOG_FILE("%s\n", deviceExtList[i].extensionName);
	}

	deviceCreateInfo.enabledExtensionCount = (uint32_t)devExtNames.size();
	deviceCreateInfo.ppEnabledExtensionNames = devExtNames.data();

	vlkRes = vkCreateDevice(g_vlkReqPhysicalDevice, &deviceCreateInfo, nullptr, &g_vlkDevice);
	if (vlkRes != VK_SUCCESS)
	{
		WRITE_IN_LOG_FILE("Failed to Create Device\n");
		return EXIT_FAILURE;
	}
	else
	{
		WRITE_IN_LOG_FILE("Create Device Complete\n");
	}

	vkGetDeviceQueue(g_vlkDevice, g_CommandQFamilyIdx, 0, &g_vlkCommandQueue);
	vkGetDeviceQueue(g_vlkDevice, g_PresentationQFamilyIdx, 1, &g_vlkPresentationQueue);

	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolCreateInfo.queueFamilyIndex = g_CommandQFamilyIdx;
	vlkRes = vkCreateCommandPool(g_vlkDevice, &commandPoolCreateInfo, nullptr, &g_vlkCmdCommandPool);
	if (vlkRes != VK_SUCCESS)
	{
		WRITE_IN_LOG_FILE("Failed to Create Cmd Command Pool\n");
		return EXIT_FAILURE;
	}

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = g_vlkCmdCommandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;
	vlkRes = vkAllocateCommandBuffers(g_vlkDevice, &commandBufferAllocateInfo, &g_vlkCmdCommandBuffer);
	if (vlkRes != VK_SUCCESS)
	{
		WRITE_IN_LOG_FILE("Failed to Obtain Cmd Command Buffers");
		return EXIT_FAILURE;
	}

	// Warm-up call
	bool bSuccess = Resize(hWnd, WIN_WIDTH, WIN_HEIGHT);
	if (!bSuccess)
	{
		WRITE_IN_LOG_FILE("War up Resize() Failed\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int UnInitialize(void)
{
	printf_s("In UnInitialize()\n");
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

		if ((g_vlkCmdCommandPool != VK_NULL_HANDLE) && (g_vlkCmdCommandBuffer != VK_NULL_HANDLE))
		{
			vkFreeCommandBuffers(g_vlkDevice, g_vlkCmdCommandPool, 1, &g_vlkCmdCommandBuffer);
			g_vlkCmdCommandBuffer = VK_NULL_HANDLE;
			vkDestroyCommandPool(g_vlkDevice, g_vlkCmdCommandPool, nullptr);
			g_vlkCmdCommandPool = VK_NULL_HANDLE;
		}

		if ((g_vlkPresentCommandBuffer != VK_NULL_HANDLE) && (g_vlkPresentCommandPool != VK_NULL_HANDLE))
		{
			vkFreeCommandBuffers(g_vlkDevice, g_vlkPresentCommandPool, 1, &g_vlkPresentCommandBuffer);
			g_vlkPresentCommandBuffer = VK_NULL_HANDLE;

			vkDestroyCommandPool(g_vlkDevice, g_vlkPresentCommandPool, nullptr);
			g_vlkPresentCommandPool = VK_NULL_HANDLE;
		}
		if (g_vlkSwapChainImages.size())
		{
			g_vlkSwapChainImages.clear();
			g_vlkSwapChainImages.shrink_to_fit();
		}

		if (g_vlkSwapChain)
		{
			vkDestroySwapchainKHR(g_vlkDevice, g_vlkSwapChain, nullptr);
			g_vlkSwapChain = VK_NULL_HANDLE;
		}

		if (g_vlkSwapChain)
		{
			vkDestroySwapchainKHR(g_vlkDevice, g_vlkSwapChain, nullptr);
			g_vlkSwapChain = VK_NULL_HANDLE;
		}

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

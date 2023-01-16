#include<Windows.h>
#include<iostream>
#include<vector>
#include<vulkan/vulkan.h>
#include<vulkan/vulkan_win32.h>

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"VkLayer_utils.lib")
#pragma comment(lib,"vulkan-1.lib")

#pragma warning(disable : 26812)


#define WIN_WIDTH		800
#define WIN_HEIGHT		600
#define LOG_FILE_NAME	"AppLogFile.txt"
#define GRAPHICS_INDEX	0
#define PRESENT_INDEX	1
#define NUMBER_OF_FRAMES	3

LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

FILE* g_pFile = NULL;

bool g_bWindowActive = false;
HWND g_hwnd = NULL;
HDC  g_hdc = NULL;

WINDOWPLACEMENT wpPrev;
DWORD dwStyle;
bool g_bFullScreen = false;

VkInstance g_vlkInstance = VK_NULL_HANDLE;
VkPhysicalDevice g_vlkPhysicalDevice = VK_NULL_HANDLE;
VkSurfaceKHR g_vlkSurface = VK_NULL_HANDLE;
VkDevice g_vlkDevice = VK_NULL_HANDLE;

uint32_t iGaphicsFamilyIndex = 0;
uint32_t iPresentFamilyIndex = 0;
VkQueue g_vlkQueue[2];

VkSemaphore g_vlkImageAvailableSemaphore[NUMBER_OF_FRAMES];
VkSemaphore g_vlkRenderingCompleteSemaphore[NUMBER_OF_FRAMES];

VkFence g_vlkCommandBufferFence[NUMBER_OF_FRAMES];
VkFence g_vlkPresentCommandBufferFence[NUMBER_OF_FRAMES];

VkCommandPool g_vlkCommandPool = VK_NULL_HANDLE;
std::vector<VkCommandBuffer> g_vlkCommandBuffers;

VkSwapchainKHR g_vlkSwapchain = VK_NULL_HANDLE;
std::vector<VkImage> g_vlkSwapChainImages;
VkCommandPool g_vlkPresentCommandPool = VK_NULL_HANDLE;
std::vector<VkCommandBuffer> g_vlkPresentCommandBuffers;

int main(int argc, char** argv, char** envp)
{
	VkResult Initialize(HINSTANCE, HWND);
	void Update(void);
	void Render(uint32_t);
	void UnInitialize();

	// Windowing Elelments
	WNDCLASSEX wndclass{};
	MSG msg{};
	HWND hwnd = NULL;
	TCHAR szClassName[] = TEXT("Sam_CMD");
	RECT windowRect;

	// Game Loop Control
	bool bDone = false;

	uint32_t iFramenNumber = 0;

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
		TEXT("VlkWindow"),
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

	g_hwnd = hwnd;

	VkResult vlkRes = Initialize(hInstance, hwnd);
	if (vlkRes != VK_SUCCESS)
	{
		MessageBox(NULL, TEXT("Issue...!!!"), TEXT("Initialize() Failed"), MB_OK | MB_ICONERROR);
		DestroyWindow(hwnd);
		exit(EXIT_FAILURE);
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
			if (iFramenNumber >= UINT32_MAX)
			{
				iFramenNumber = 0;
			}
			iFramenNumber++;
			Render(iFramenNumber);
		}
	}

	return ((int)msg.wParam);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void FullScreen(void);
	VkResult Resize(HWND hWnd, int iWidth, int iHeight);
	void UnInitialize();

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
	case WM_SIZE:
		if (g_vlkDevice)
		{
			VkResult vlkRes = Resize(hwnd,LOWORD(lParam), HIWORD(lParam));
			if (vlkRes != VK_SUCCESS)
			{
				printf_s("Something Failed While Resize()\n");
				PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, 0);
			}
		}
		
		break;
	case WM_QUIT:
		break;
	case WM_DESTROY:
		// Destroy Vulkan stuff here.
		UnInitialize();
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

VkResult Initialize(HINSTANCE hInstance, HWND hWnd)
{
	VkResult Resize(HWND hWnd, int iWidth, int iHeight);

	VkResult vlkRes{};

	VkApplicationInfo vlkApplicationInfo{};
	vlkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vlkApplicationInfo.pNext = NULL;
	vlkApplicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	vlkApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	vlkApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	vlkApplicationInfo.pApplicationName = "BlueWinVK";
	vlkApplicationInfo.pEngineName = "NoEngine";

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &vlkApplicationInfo;

	std::vector<VkExtensionProperties> insatnceExtensions{};
	uint32_t iInstanceExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &iInstanceExtensionCount, nullptr);
	insatnceExtensions.resize(iInstanceExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &iInstanceExtensionCount, insatnceExtensions.data());

	std::vector<const char*> instanceExtensionNameList;

	for (size_t iIdx = 0; iIdx < iInstanceExtensionCount; iIdx++)
	{
		instanceExtensionNameList.push_back(insatnceExtensions[iIdx].extensionName);
	}
	instanceExtensionNameList.shrink_to_fit();

	instanceCreateInfo.enabledExtensionCount = (uint32_t)insatnceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionNameList.data();

	std::vector<const char*> instanceLayerNameList{ "VK_LAYER_KHRONOS_validation" /*,"VK_LAYER_LUNARG_monitor"*/};
	instanceCreateInfo.enabledLayerCount = (uint32_t)instanceLayerNameList.size();
	instanceCreateInfo.ppEnabledLayerNames = instanceLayerNameList.data();

	vlkRes = vkCreateInstance(&instanceCreateInfo, nullptr, &g_vlkInstance);
	if (vlkRes != VK_SUCCESS)
	{
		printf_s("Could not vkCreateInstance()\n");
		return vlkRes;
	}
	else
	{
		printf_s("vkCreateInstance() SUCCESS\n");
	}

	// Create Surface 
	// This will be re-created in Re-size while creating framebuffers
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.hinstance = hInstance;
	surfaceCreateInfo.hwnd = hWnd;
	vlkRes = vkCreateWin32SurfaceKHR(g_vlkInstance, &surfaceCreateInfo, nullptr, &g_vlkSurface);
	if (vlkRes != VK_SUCCESS)
	{
		printf_s("Could not vkCreateWin32SurfaceKHR()\n");
		return vlkRes;
	}
	else
	{
		printf_s("vkCreateWin32SurfaceKHR() SUCCESS\n");
	}


	// Enumerate Devices
	std::vector<VkPhysicalDevice>physicalDeviceList;
	uint32_t iPhysicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(g_vlkInstance, &iPhysicalDeviceCount, nullptr);
	physicalDeviceList.resize(iPhysicalDeviceCount);
	vkEnumeratePhysicalDevices(g_vlkInstance, &iPhysicalDeviceCount, physicalDeviceList.data());

	for (size_t iIdx = 0; iIdx < physicalDeviceList.size(); iIdx++)
	{
		// Queues
		uint32_t iNumberOfQueues = 0;
		std::vector<VkQueueFamilyProperties> queueFamilyProprtiesList;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDeviceList[iIdx], &iNumberOfQueues, nullptr);
		queueFamilyProprtiesList.resize(iNumberOfQueues);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDeviceList[iIdx], &iNumberOfQueues, queueFamilyProprtiesList.data());

		// Extensions

		// Surface Capabilites
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		//VkBool32 bDisplatSupportd = VK_FALSE;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDeviceList[iIdx], g_vlkSurface, &surfaceCapabilities);
		//vkGetPhysicalDeviceSurfaceSupportKHR(g_vlkPhysicalDevice,0,g_vlkSurface,&bDisplatSupportd);

		//Mem
		VkPhysicalDeviceMemoryProperties memProperties{};
		vkGetPhysicalDeviceMemoryProperties(physicalDeviceList[iIdx], &memProperties);
		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(physicalDeviceList[iIdx], &deviceProperties);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
		{
			g_vlkPhysicalDevice = physicalDeviceList[iIdx];
		}
	}

	if (g_vlkPhysicalDevice == VK_NULL_HANDLE)
	{
		printf_s("Could not Choose desirable device\n");
		return VK_RESULT_MAX_ENUM;
	}

	// Create Logical Device and Queues.
	
	// Check Queue Families
	uint32_t iNumberOfQueues = 0;
	std::vector<VkQueueFamilyProperties> queueFamilyProprtiesList;
	vkGetPhysicalDeviceQueueFamilyProperties(g_vlkPhysicalDevice, &iNumberOfQueues, nullptr);
	queueFamilyProprtiesList.resize(iNumberOfQueues);
	vkGetPhysicalDeviceQueueFamilyProperties(g_vlkPhysicalDevice, &iNumberOfQueues, queueFamilyProprtiesList.data());

	
	for (uint32_t iIdx = 0; iIdx < iNumberOfQueues; iIdx++)
	{
		std::vector<VkQueueFamilyProperties> queueFamilies(iNumberOfQueues);
		vkGetPhysicalDeviceQueueFamilyProperties(g_vlkPhysicalDevice, &iNumberOfQueues, queueFamilies.data());

		// Graphics Support
		if (queueFamilies[iIdx].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			iGaphicsFamilyIndex = iIdx;
			printf_s("Idx = %d, Graphics Supprted,",iIdx);
		}

		// Present Support
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(g_vlkPhysicalDevice, iIdx, g_vlkSurface, &presentSupport);
		if (presentSupport && (iIdx != iGaphicsFamilyIndex))
		{
			iPresentFamilyIndex = iIdx;
			printf_s("Idx = %d Presentation Supported\n", iIdx);
		}
	}

	std::vector<uint32_t> queueIndexList = { iGaphicsFamilyIndex,iPresentFamilyIndex };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoList{};

	for (size_t iIdx = 0; iIdx < queueIndexList.size(); iIdx++)
	{
		float fQueuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.flags = 0;
		queueCreateInfo.pQueuePriorities = &fQueuePriority;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.queueFamilyIndex = queueIndexList[iIdx];

		queueCreateInfoList.push_back(queueCreateInfo);
	}


	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfoList.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfoList.data();

	uint32_t iDeviceLayerCount = 0;
	std::vector<VkLayerProperties> deviceLayerList;
	std::vector<const char*> deviceLayerNames;
	vkEnumerateDeviceLayerProperties(g_vlkPhysicalDevice, &iDeviceLayerCount, nullptr);
	deviceLayerList.resize(iDeviceLayerCount);
	vkEnumerateDeviceLayerProperties(g_vlkPhysicalDevice, &iDeviceLayerCount, deviceLayerList.data());

	for (size_t iIdx = 0; iIdx < iDeviceLayerCount; iIdx++)
	{
		deviceLayerNames.push_back(deviceLayerList[iIdx].layerName);
	}

	deviceCreateInfo.enabledLayerCount = iDeviceLayerCount;
	deviceCreateInfo.ppEnabledLayerNames = deviceLayerNames.data();

	uint32_t iDeviceExtensionCount = 0;
	std::vector<VkExtensionProperties> deviceExtensionPropertiesList;
	std::vector<const char*>deviceExtensionNameList;

	vkEnumerateDeviceExtensionProperties(g_vlkPhysicalDevice, NULL, &iDeviceExtensionCount, nullptr);
	deviceExtensionPropertiesList.resize(iDeviceExtensionCount);
	vkEnumerateDeviceExtensionProperties(g_vlkPhysicalDevice, NULL, &iDeviceExtensionCount, deviceExtensionPropertiesList.data());

	for (size_t iIdx = 0; iIdx < iDeviceExtensionCount; iIdx++)
	{
		if (strcmp("VK_EXT_buffer_device_address", deviceExtensionPropertiesList[iIdx].extensionName) == 0)
		{
			continue;
		}
		deviceExtensionNameList.push_back(deviceExtensionPropertiesList[iIdx].extensionName);
	}

	deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensionNameList.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNameList.data();

	VkPhysicalDeviceFeatures physicalDeviceFeatures{};
	vkGetPhysicalDeviceFeatures(g_vlkPhysicalDevice, &physicalDeviceFeatures);

	deviceCreateInfo.pEnabledFeatures= &physicalDeviceFeatures;


	vlkRes = vkCreateDevice(g_vlkPhysicalDevice, &deviceCreateInfo, nullptr, &g_vlkDevice); 
	if (vlkRes != VK_SUCCESS)
	{
		printf_s("Could not vkCreateDevice()\n");
		return vlkRes;
	}
	else
	{
		printf_s("vkCreateDevice() SUCCESS\n");
	}

	vkGetDeviceQueue(g_vlkDevice, iGaphicsFamilyIndex,0, &g_vlkQueue[GRAPHICS_INDEX]);
	vkGetDeviceQueue(g_vlkDevice, iPresentFamilyIndex,0, &g_vlkQueue[PRESENT_INDEX]);

	// CreateSemaphores

	for (size_t iIdx = 0; iIdx < NUMBER_OF_FRAMES; iIdx++)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vkCreateSemaphore(g_vlkDevice, &semaphoreCreateInfo, nullptr, &g_vlkImageAvailableSemaphore[iIdx]);
		vkCreateSemaphore(g_vlkDevice, &semaphoreCreateInfo, nullptr, &g_vlkRenderingCompleteSemaphore[iIdx]);
	}

	// Create Command Pool
	// To hold allocated Command buffers
	VkCommandPoolCreateInfo commandPoolCreateInfo{};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

	// this will alow our command buffer to reset implicitly while we call vkBeginCommandBuffer()
	// otherwise we need to call vkResetCommandBuffer()
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	// Create command buffer to send commands through Graphics queue
	commandPoolCreateInfo.queueFamilyIndex = iGaphicsFamilyIndex;
	vlkRes = vkCreateCommandPool(g_vlkDevice, &commandPoolCreateInfo, nullptr, &g_vlkCommandPool);
	if (vlkRes != VK_SUCCESS)
	{
		printf_s("Could not vkCreateCommandPool()\n");
		return vlkRes;
	}
	else
	{
		printf_s("vkCreateCommandPool() SUCCESS\n");
	}

	// Create Command Buffer
	g_vlkCommandBuffers.resize(NUMBER_OF_FRAMES);
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = NULL;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // These Command buffers will be Primary.
	commandBufferAllocateInfo.commandBufferCount = NUMBER_OF_FRAMES;
	commandBufferAllocateInfo.commandPool = g_vlkCommandPool;
	
	vlkRes = vkAllocateCommandBuffers(g_vlkDevice,&commandBufferAllocateInfo,g_vlkCommandBuffers.data());
	if (vlkRes != VK_SUCCESS)
	{
		printf_s("Could not vkAllocateCommandBuffers()\n");
		return vlkRes;
	}
	else
	{
		printf_s("vkAllocateCommandBuffers() SUCCESS\n");
	}

	// Create Command Buffer Fence
	for (size_t iIdx = 0; iIdx < NUMBER_OF_FRAMES; iIdx++)
	{
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = 0;
		vkCreateFence(g_vlkDevice, &fenceCreateInfo, nullptr, &g_vlkCommandBufferFence[iIdx]);
		if (vlkRes != VK_SUCCESS)
		{
			printf_s("Could not vkCreateFence(%zd)\n",iIdx);
			return vlkRes;
		}
		else
		{
			printf_s("vkCreateFence() SUCCESS\n");
		}
	}
	
	// Warm-up Call to set up all required things for rendering to a framebuffer.
	vlkRes = Resize(hWnd,WIN_WIDTH,WIN_HEIGHT);

	return vlkRes;
}

void Update(void)
{
}

void Render(uint32_t iFrameIdx)
{
	uint32_t iCurrentIndex = iFrameIdx % NUMBER_OF_FRAMES;
	uint32_t imageIndex = 0;

	// Record the command buffer for presentation.
	VkCommandBufferBeginInfo presenrCmdBufBeginInfo{};
	presenrCmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	presenrCmdBufBeginInfo.flags= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkClearColorValue clearColor = {{ 0.0f, 0.0f, 1.0f, 1.0f } };// R, G, B, A

	VkImageSubresourceRange imageSubResourceRange{};
	imageSubResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubResourceRange.baseMipLevel = 0;
	imageSubResourceRange.levelCount = 1;
	imageSubResourceRange.baseArrayLayer = 0;
	imageSubResourceRange.layerCount = 1;


	VkImageMemoryBarrier presentToClearBarrier = {};
	presentToClearBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	presentToClearBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	presentToClearBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	presentToClearBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	presentToClearBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	presentToClearBarrier.srcQueueFamilyIndex = iPresentFamilyIndex;
	presentToClearBarrier.dstQueueFamilyIndex = iPresentFamilyIndex;
	presentToClearBarrier.image = g_vlkSwapChainImages[iCurrentIndex];
	presentToClearBarrier.subresourceRange = imageSubResourceRange;

	// Change layout of image to be optimal for presenting
	VkImageMemoryBarrier clearToPresentBarrier = {};
	clearToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	clearToPresentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	clearToPresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	clearToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	clearToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	clearToPresentBarrier.srcQueueFamilyIndex = iPresentFamilyIndex;
	clearToPresentBarrier.dstQueueFamilyIndex = iPresentFamilyIndex;
	clearToPresentBarrier.image = g_vlkSwapChainImages[iCurrentIndex];
	clearToPresentBarrier.subresourceRange = imageSubResourceRange;


	vkBeginCommandBuffer(g_vlkPresentCommandBuffers[iCurrentIndex], &presenrCmdBufBeginInfo);

	vkCmdPipelineBarrier(g_vlkPresentCommandBuffers[iCurrentIndex],VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&presentToClearBarrier);

	vkCmdClearColorImage(g_vlkPresentCommandBuffers[iCurrentIndex], g_vlkSwapChainImages[iCurrentIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &imageSubResourceRange);

	vkCmdPipelineBarrier(g_vlkPresentCommandBuffers[iCurrentIndex], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &clearToPresentBarrier);

	vkEndCommandBuffer(g_vlkPresentCommandBuffers[iCurrentIndex]);

	// Submit the command buffer

	vkAcquireNextImageKHR(g_vlkDevice, g_vlkSwapchain, UINT64_MAX, g_vlkImageAvailableSemaphore[iCurrentIndex], VK_NULL_HANDLE, &imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &g_vlkImageAvailableSemaphore[iCurrentIndex];

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &g_vlkRenderingCompleteSemaphore[iCurrentIndex];

	// This is the stage where the queue should wait on the semaphore (it doesn't have to wait with drawing, for example)
	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	submitInfo.pWaitDstStageMask = &waitDstStageMask;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &g_vlkPresentCommandBuffers[imageIndex];

	vkQueueSubmit(g_vlkQueue[PRESENT_INDEX], 1, &submitInfo, g_vlkPresentCommandBufferFence[iCurrentIndex]);

	//vkWaitForFences(g_vlkDevice, 1, &g_vlkPresentCommandBufferFence[iCurrentIndex], VK_TRUE, UINT32_MAX);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &g_vlkRenderingCompleteSemaphore[iCurrentIndex];

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &g_vlkSwapchain;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(g_vlkQueue[PRESENT_INDEX], &presentInfo);

}

void UnInitialize()
{
	if (g_vlkDevice)
	{
		vkDeviceWaitIdle(g_vlkDevice);

		for (size_t iIdx = 0; iIdx < NUMBER_OF_FRAMES; iIdx++)
		{
			if (g_vlkImageAvailableSemaphore[iIdx])
			{
				vkDestroySemaphore(g_vlkDevice, g_vlkImageAvailableSemaphore[iIdx], nullptr);
				g_vlkImageAvailableSemaphore[iIdx] = VK_NULL_HANDLE;
			}
			if (g_vlkRenderingCompleteSemaphore[iIdx])
			{
				vkDestroySemaphore(g_vlkDevice, g_vlkRenderingCompleteSemaphore[iIdx], nullptr);
				g_vlkRenderingCompleteSemaphore[iIdx] = VK_NULL_HANDLE;
			}
			
		}
		for (size_t iIdx = 0; iIdx < NUMBER_OF_FRAMES; iIdx++)
		{
			vkDestroyFence(g_vlkDevice, g_vlkPresentCommandBufferFence[iIdx], nullptr);
			g_vlkPresentCommandBufferFence[iIdx] = VK_NULL_HANDLE;
		}

		for (size_t iIdx = 0; iIdx < NUMBER_OF_FRAMES; iIdx++)
		{
			vkDestroyFence(g_vlkDevice, g_vlkCommandBufferFence[iIdx], nullptr);
			g_vlkCommandBufferFence[iIdx] = VK_NULL_HANDLE;
		}

		if (g_vlkPresentCommandBuffers.size())
		{
			vkFreeCommandBuffers(g_vlkDevice, g_vlkPresentCommandPool, (uint32_t)g_vlkPresentCommandBuffers.size(), g_vlkPresentCommandBuffers.data());
			g_vlkPresentCommandBuffers.clear();
			g_vlkPresentCommandBuffers.shrink_to_fit();
		}
		if (g_vlkPresentCommandPool)
		{
			vkDestroyCommandPool(g_vlkDevice, g_vlkPresentCommandPool, nullptr);
			g_vlkPresentCommandPool = VK_NULL_HANDLE;
		}
		if (g_vlkSwapChainImages.size())
		{
			g_vlkSwapChainImages.clear();
			g_vlkSwapChainImages.shrink_to_fit();
		}
		if (g_vlkSwapchain)
		{
			vkDestroySwapchainKHR(g_vlkDevice, g_vlkSwapchain, nullptr);
			g_vlkSwapchain = VK_NULL_HANDLE;
		}
		
		if (g_vlkCommandBuffers.size())
		{
			vkFreeCommandBuffers(g_vlkDevice, g_vlkCommandPool, NUMBER_OF_FRAMES, g_vlkCommandBuffers.data());
		}

		if (g_vlkCommandPool)
		{
			vkDestroyCommandPool(g_vlkDevice, g_vlkCommandPool,nullptr);
			g_vlkCommandPool = VK_NULL_HANDLE;
		}		

		vkDestroyDevice(g_vlkDevice, nullptr);
		g_vlkDevice = VK_NULL_HANDLE;
	}

	if (g_vlkInstance)
	{
		if (g_vlkSurface)
		{
			vkDestroySurfaceKHR(g_vlkInstance, g_vlkSurface, nullptr);
			g_vlkSurface = VK_NULL_HANDLE;
		}

		vkDestroyInstance(g_vlkInstance, nullptr);
		g_vlkInstance = VK_NULL_HANDLE;
	}
}

/*
* Create Swapchain and Famebuffer to show things on the screen.
*/
VkResult Resize(HWND hWnd,int iWidth,int iHeight)
{
	// Clear Resources whiuch were created for presentation before
	// Re-Create everything as per new size.
	vkQueueWaitIdle(g_vlkQueue[PRESENT_INDEX]);

	for (size_t iIdx = 0; iIdx < NUMBER_OF_FRAMES; iIdx++)
	{
		vkDestroyFence(g_vlkDevice, g_vlkPresentCommandBufferFence[iIdx], nullptr);
		g_vlkPresentCommandBufferFence[iIdx] = VK_NULL_HANDLE;
	}

	if (g_vlkPresentCommandBuffers.size())
	{
		vkFreeCommandBuffers(g_vlkDevice, g_vlkPresentCommandPool, (uint32_t)g_vlkPresentCommandBuffers.size(), g_vlkPresentCommandBuffers.data());
		g_vlkPresentCommandBuffers.clear();
		g_vlkPresentCommandBuffers.shrink_to_fit();
	}
	if (g_vlkPresentCommandPool)
	{
		vkDestroyCommandPool(g_vlkDevice,g_vlkPresentCommandPool,nullptr);
		g_vlkPresentCommandPool = VK_NULL_HANDLE;
	}
	if (g_vlkSwapChainImages.size())
	{
		g_vlkSwapChainImages.clear();
		g_vlkSwapChainImages.shrink_to_fit();
	}
	if (g_vlkSwapchain)
	{
		vkDestroySwapchainKHR(g_vlkDevice, g_vlkSwapchain, nullptr);
		g_vlkSwapchain = VK_NULL_HANDLE;
	}
	if (g_vlkSurface)
	{
		vkDestroySurfaceKHR(g_vlkInstance, g_vlkSurface, nullptr);
		g_vlkSurface = VK_NULL_HANDLE;
	}
	
	VkResult vlkRes = VK_SUCCESS;

	// Create New Surface with New Dimensions
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
	surfaceCreateInfo.hwnd = hWnd;
	vlkRes = vkCreateWin32SurfaceKHR(g_vlkInstance, &surfaceCreateInfo, nullptr, &g_vlkSurface);
	if (vlkRes != VK_SUCCESS)
	{
		printf_s("In Resize() : Could not vkCreateWin32SurfaceKHR()\n");
		return vlkRes;
	}
	else
	{
		printf_s("In Resize() : vkCreateWin32SurfaceKHR() SUCCESS\n");
	}

	// Find Surface Capabilites
	VkSurfaceCapabilitiesKHR surfaceCapabilities{};
	vlkRes = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_vlkPhysicalDevice, g_vlkSurface, &surfaceCapabilities);
	if (vlkRes != VK_SUCCESS)
	{
		printf_s("In Resize() : Could not vkGetPhysicalDeviceSurfaceCapabilitiesKHR()\n");
		return vlkRes;
	}
	else
	{
		printf_s("In Resize() : vkGetPhysicalDeviceSurfaceCapabilitiesKHR() SUCCESS\n");
	}

	// Find supported SurfasceFormats
	uint32_t iSurfaceFormatCount = 0;
	std::vector<VkSurfaceFormatKHR> surfaceFormatList;
	VkSurfaceFormatKHR desiredSurfaceFormat{};
	vkGetPhysicalDeviceSurfaceFormatsKHR(g_vlkPhysicalDevice, g_vlkSurface, &iSurfaceFormatCount, nullptr);
	surfaceFormatList.resize(iSurfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(g_vlkPhysicalDevice, g_vlkSurface, &iSurfaceFormatCount, surfaceFormatList.data());

	// Find Supported Present Modes
	uint32_t iPresentModesCount = 0;
	std::vector<VkPresentModeKHR> presentModesList;
	VkPresentModeKHR desiredPresentMode{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(g_vlkPhysicalDevice, g_vlkSurface, &iPresentModesCount, nullptr);
	presentModesList.resize(iPresentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(g_vlkPhysicalDevice, g_vlkSurface, &iPresentModesCount, presentModesList.data());

	// Chech if NUMBER_OF_FRAMES is actually possible 
	if ((NUMBER_OF_FRAMES >= surfaceCapabilities.minImageCount)&&(NUMBER_OF_FRAMES< surfaceCapabilities.maxImageCount))
	{
		printf_s("In Resize() : Desired Number of Frames can be obtained\n");
	}
	else
	{
		printf_s("In Resize() : Minimum Image available are not available %d\n", surfaceCapabilities.minImageCount);
		return VK_RESULT_MAX_ENUM;
	}

	// Choose a Surface Format
	for (size_t iIdx = 0; iIdx < surfaceFormatList.size(); iIdx++)
	{
		if (surfaceFormatList[iIdx].format == VK_FORMAT_B8G8R8A8_UNORM)
		{
			desiredSurfaceFormat = surfaceFormatList[iIdx];
		}
	}

	// Chose a Present Mode
	for (size_t iIdx = 0; iIdx < presentModesList.size(); iIdx++)
	{
		if (presentModesList[iIdx] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			desiredPresentMode = presentModesList[iIdx];
		}
	}

	// Set Swapchain Image Size/ Image Extent
	VkExtent2D swapChainExtent = surfaceCapabilities.currentExtent;

	if (!(surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) 
	{
		printf_s("swap chain image does not support VK_IMAGE_TRANSFER_DST usage\n");
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

	// Create swapchain
	VkSwapchainCreateInfoKHR swapChainCreateInfo{};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.pNext = nullptr;
	swapChainCreateInfo.surface = g_vlkSurface;
	swapChainCreateInfo.minImageCount = NUMBER_OF_FRAMES;
	swapChainCreateInfo.imageFormat = desiredSurfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = desiredSurfaceFormat.colorSpace;
	swapChainCreateInfo.imageExtent = swapChainExtent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.oldSwapchain = g_vlkSwapchain;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.queueFamilyIndexCount = 0;
	swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	swapChainCreateInfo.preTransform = surfaceTraansformBit;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = desiredPresentMode;
	swapChainCreateInfo.clipped = VK_TRUE;

	vlkRes = vkCreateSwapchainKHR(g_vlkDevice, &swapChainCreateInfo, nullptr, &g_vlkSwapchain);
	if (vlkRes != VK_SUCCESS)
	{
		printf_s("In Resize() : Could not vkCreateSwapchainKHR()\n");
		return vlkRes;
	}
	else
	{
		printf_s("In Resize() : vkCreateSwapchainKHR() SUCCESS\n");
	}

	uint32_t iSwapChainImageCount = 0;

	vkGetSwapchainImagesKHR(g_vlkDevice, g_vlkSwapchain, &iSwapChainImageCount,nullptr);
	g_vlkSwapChainImages.resize(iSwapChainImageCount);
	printf_s("Obtained %d Images foe swapchain\n", iSwapChainImageCount);
	vkGetSwapchainImagesKHR(g_vlkDevice, g_vlkSwapchain, &iSwapChainImageCount, g_vlkSwapChainImages.data());



	VkCommandPoolCreateInfo presentCommandPoolCreateInfo{};
	presentCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	presentCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	presentCommandPoolCreateInfo.pNext = nullptr;
	presentCommandPoolCreateInfo.queueFamilyIndex = iPresentFamilyIndex;

	vlkRes = vkCreateCommandPool(g_vlkDevice, &presentCommandPoolCreateInfo, nullptr, &g_vlkPresentCommandPool);
	if (vlkRes != VK_SUCCESS)
	{
		printf_s("In Resize() : Could not vkCreateSwapchainKHR()\n");
		return vlkRes;
	}
	else
	{
		printf_s("In Resize() : vkCreateSwapchainKHR() SUCCESS\n");
	}

	g_vlkPresentCommandBuffers.resize(NUMBER_OF_FRAMES);
	VkCommandBufferAllocateInfo presentCmdBufferAllocInfo{};
	presentCmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	presentCmdBufferAllocInfo.pNext = nullptr;
	presentCmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	presentCmdBufferAllocInfo.commandBufferCount = NUMBER_OF_FRAMES;
	presentCmdBufferAllocInfo.commandPool = g_vlkPresentCommandPool;
	vlkRes = vkAllocateCommandBuffers(g_vlkDevice,&presentCmdBufferAllocInfo, g_vlkPresentCommandBuffers.data());
	if (vlkRes != VK_SUCCESS)
	{
		printf_s("In Resize() : Could not vkAllocateCommandBuffers(g_vlkPresentCommandBuffers)\n");
		return vlkRes;
	}
	else
	{
		printf_s("In Resize() : vkAllocateCommandBuffers(g_vlkPresentCommandBuffers) SUCCESS\n");
	}

	for (size_t iIdx = 0; iIdx < NUMBER_OF_FRAMES; iIdx++)
	{
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = 0;
		vkCreateFence(g_vlkDevice, &fenceCreateInfo, nullptr, &g_vlkPresentCommandBufferFence[iIdx]);
		if (vlkRes != VK_SUCCESS)
		{
			printf_s("In Resize() : Could not vkCreateFence(%zd)\n", iIdx);
			return vlkRes;
		}
		else
		{
			printf_s("In Resize() : vkCreateFence() SUCCESS\n");
		}
	}

	return vlkRes;
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


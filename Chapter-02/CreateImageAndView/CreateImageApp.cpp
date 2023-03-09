#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

VkInstance g_vlkInstance = VK_NULL_HANDLE;
VkDevice g_vlkDevice = VK_NULL_HANDLE;
VkPhysicalDevice g_vlkRequiredPhysicalDevice = VK_NULL_HANDLE;

uint32_t g_queueFamilyIndex = 0;
uint32_t g_queueIndex = 0;
VkQueue g_vlkQueue = VK_NULL_HANDLE;

VkImage g_vlkImage = VK_NULL_HANDLE;
VkImageView g_vlkImageView = VK_NULL_HANDLE;

void cleanUp(); // clean all gloabal resources

int main()
{
    VkResult vlkRes = VK_SUCCESS;

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = "DummyInstance";
    applicationInfo.pEngineName = "NoEngine";

    VkInstanceCreateInfo instancecreateInfo{}; // Cretes instance with zero values as well
    instancecreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instancecreateInfo.pNext = nullptr;
    instancecreateInfo.flags = 0;
    instancecreateInfo.pApplicationInfo = &applicationInfo;

    // Layers
    // ONLY WHAT YOU NEED..!!
    std::vector<const char *> layerNames;
    layerNames.push_back("VK_LAYER_KHRONOS_validation");
    // layerNames.push_back("VK_LAYER_LUNARG_api_dump");
    // layerNames.push_back("VK_LAYER_KHRONOS_synchronization2");  // Enable while using events, semaphores and Barriers.Basically for tracking the objects.
    // layerNames.push_back("VK_LAYER_LUNARG_monitor");          //  display FPS on title bar only available in WIN32 and XLib, no other platforms.
    instancecreateInfo.enabledLayerCount = (uint32_t)layerNames.size();
    instancecreateInfo.ppEnabledLayerNames = layerNames.data();

    // uint32_t iLayerCount = -1;
    // std::vector<VkLayerProperties> layerList;
    // vkEnumerateInstanceLayerProperties(&iLayerCount, nullptr);
    // layerList.resize(iLayerCount);
    // vkEnumerateInstanceLayerProperties(&iLayerCount, layerList.data());
    // for (size_t i = 0; i < layerList.size(); i++)
    // {
    //     std::cout << layerList[i].layerName << std::endl;
    // }

    // Extensions
    // AS PER OUR USE, BUT TAKING ALL OF THEM.
    uint32_t iExtensionCount = 0;
    std::vector<VkExtensionProperties> extensionList;
    std::vector<const char *> extensionNames;

    vlkRes = vkEnumerateInstanceExtensionProperties(nullptr, &iExtensionCount, nullptr);
    if (vlkRes != VK_SUCCESS)
    {
        cleanUp();
        exit(EXIT_FAILURE);
    }
    extensionList.resize(iExtensionCount);
    vlkRes = vkEnumerateInstanceExtensionProperties(nullptr, &iExtensionCount, extensionList.data());
    if (vlkRes != VK_SUCCESS)
    {
        cleanUp();
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < extensionList.size(); i++)
    {
        extensionNames.push_back(extensionList[i].extensionName);
    }

    instancecreateInfo.enabledExtensionCount = (uint32_t)extensionNames.size();
    instancecreateInfo.ppEnabledExtensionNames = extensionNames.data();

    vlkRes = vkCreateInstance(&instancecreateInfo, nullptr, &g_vlkInstance);
    if (vlkRes != VK_SUCCESS)
    {
        if (vlkRes == VK_ERROR_INCOMPATIBLE_DRIVER)
        {
            std::cout << "Incompatible DRIVER, ";
        }

        std::cout << "Could Not create instance :" << vlkRes << std::endl;
        cleanUp();
        exit(EXIT_FAILURE);
    }
    else
    {
        std::cout << "SAM : Vulkan Instance Created" << std::endl;
    }

#pragma region EnableDebugging

    // Use DebugUtil not Debug Report

#pragma endregion // EnableDebugging

    // Physical devices

    std::vector<VkPhysicalDevice> physicalDeviceList;
    uint32_t iPhysicalDeviceCount;

    vlkRes = vkEnumeratePhysicalDevices(g_vlkInstance, &iPhysicalDeviceCount, nullptr);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkEnumeratePhysicalDevices() Failed, count" << std::endl;
        cleanUp();
        exit(EXIT_FAILURE);
    }
    physicalDeviceList.resize(iPhysicalDeviceCount);
    vlkRes = vkEnumeratePhysicalDevices(g_vlkInstance, &iPhysicalDeviceCount, physicalDeviceList.data());
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkEnumeratePhysicalDevices() Failed, data " << vlkRes << std::endl;
        cleanUp();
        exit(EXIT_FAILURE);
    }

    // iterate over all phusial devices to select one.
    std::cout << "iterate over all physical devices to select one." << std::endl;
    uint32_t iDesiredPhysicalDeviceMemory = -1;
    for (size_t i = 0; i < physicalDeviceList.size(); i++)
    {

        VkPhysicalDeviceProperties deviceProp{};
        // std::vector<VkDisplayPropertiesKHR> displayPropertiesList;
        uint32_t iPropertyCount = -1;
        vkGetPhysicalDeviceProperties(physicalDeviceList[i], &deviceProp);
        // Property 1 : DISCRETE GPU ONLY.
        if (deviceProp.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            // Property 2 : Has Display Capabilities OR Selecting device with Primary Monitor connection.
            /*  Need Surface for this              */
            /*  Use In case of Setting up Display  */

            // Property 3 : Maximum VRAM memory.
            VkPhysicalDeviceMemoryProperties memoryProperties{};
            uint32_t iDisplayProperty = -1;
            vkGetPhysicalDeviceMemoryProperties(physicalDeviceList[i], &memoryProperties);

            // std::cout << "memoryProperties.memoryHeaps " << memoryProperties.memoryHeaps << " , memoryProperties.memoryHeapCount " <<
            // memoryProperties.memoryHeapCount << ", addition " << std::dec(memoryProperties.memoryHeaps + memoryProperties.memoryHeapCount) <<
            // std::endl;

            auto heapsPointer = memoryProperties.memoryHeaps;
            auto heaps = std::vector<VkMemoryHeap>(heapsPointer, heapsPointer + memoryProperties.memoryHeapCount);

            for (const auto &heap : heaps)
            {
                if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                {
                    // Device local heap, should be size of total GPU VRAM.
                    // heap.size will be the size of VRAM in bytes. (bigger is better)
                    if ((heap.size) && (iDesiredPhysicalDeviceMemory > heap.size))
                    {
                        iDesiredPhysicalDeviceMemory = heap.size;
                        g_vlkRequiredPhysicalDevice = physicalDeviceList[i];
                    }
                }
            }
        }
        else
        {
            continue;
        }
    }

    if (g_vlkRequiredPhysicalDevice != VK_NULL_HANDLE)
    {
        std::cout << "SAM : Physical Device Obtained" << std::endl;
    }
    else
    {
        std::cout << "SAM : Physical Device NOT Obtained" << std::endl;
    }

    std::cout << "SAM : Logical Device Start" << std::endl;
    // Logical Device

    g_queueFamilyIndex = 0; // 1st family Index. Other wise check capabilites just before this stage.
    g_queueIndex = 0;       // can choose different queue based on max number of queus as per the family Index.

    float fPriority[] = {1.0f};
    VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = nullptr;
    deviceQueueCreateInfo.flags = 0;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.queueFamilyIndex = g_queueFamilyIndex; // 1st family Index. Other wise check capabilites just before this stage.
    deviceQueueCreateInfo.pQueuePriorities = fPriority;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;

    std::cout << "SAM : Layer Properties" << std::endl;
    uint32_t iDeviceLayerCount = 0;
    std::vector<VkLayerProperties> deviceLayerPropertiesList;
    std::vector<const char *> deviceLayerNamesList{};
    vkEnumerateDeviceLayerProperties(g_vlkRequiredPhysicalDevice, &iDeviceLayerCount, nullptr);
    deviceLayerPropertiesList.resize(iDeviceLayerCount);
    vkEnumerateDeviceLayerProperties(g_vlkRequiredPhysicalDevice, &iDeviceLayerCount, deviceLayerPropertiesList.data());

    // deviceLayerNamesList.push_back("VK_LAYER_KHRONOS_validation");
    // deviceLayerNamesList.push_back("VK_LAYER_LUNARG_api_dump");

    for (size_t iIdx = 0; iIdx < deviceLayerPropertiesList.size(); iIdx++)
    {
        // deviceLayerNamesList.push_back(deviceLayerPropertiesList[iIdx].layerName);
        std::cout << deviceLayerPropertiesList[iIdx].layerName << std::endl;
    }

    deviceCreateInfo.enabledLayerCount = (uint32_t)deviceLayerNamesList.size();
    deviceCreateInfo.ppEnabledLayerNames = deviceLayerNamesList.data();

    std::cout << "SAM : Extension Properties Start" << std::endl;
    uint32_t iDeviceExtensionCount = 0;
    std::vector<VkExtensionProperties> deviceExtensionPropertiesList;
    vkEnumerateDeviceExtensionProperties(g_vlkRequiredPhysicalDevice, nullptr, &iDeviceExtensionCount, nullptr);
    deviceExtensionPropertiesList.resize(iDeviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(g_vlkRequiredPhysicalDevice, nullptr, &iDeviceExtensionCount, deviceExtensionPropertiesList.data());
    std::vector<const char *> extensionNamesList{};

    for (size_t iIdx = 0; iIdx < deviceExtensionPropertiesList.size(); iIdx++)
    {
        // extensionNamesList.push_back(deviceExtensionPropertiesList[iIdx].extensionName);
        std::cout << deviceExtensionPropertiesList[iIdx].extensionName << std::endl;
    }

    deviceCreateInfo.enabledExtensionCount = (uint32_t)extensionNamesList.size();
    deviceCreateInfo.ppEnabledLayerNames = extensionNamesList.data();

    std::cout << "SAM : Device Features" << std::endl;
    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    vkGetPhysicalDeviceFeatures(g_vlkRequiredPhysicalDevice, &physicalDeviceFeatures);
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

    std::cout << "SAM : Just before vkCreateDevice()" << std::endl;
    vlkRes = vkCreateDevice(g_vlkRequiredPhysicalDevice, &deviceCreateInfo, nullptr, &g_vlkDevice);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "Could Not create Logical Device :" << vlkRes << std::endl;
        cleanUp();
        exit(EXIT_FAILURE);
    }
    else
    {
        std::cout << "SAM : Vulkan Logical Device Created" << std::endl;
    }

    vkGetDeviceQueue(g_vlkDevice, g_queueFamilyIndex, g_queueIndex, &g_vlkQueue);

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = 0;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.extent = {1024, 1024, 1};
    imageCreateInfo.mipLevels = 10;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = nullptr;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vlkRes = vkCreateImage(g_vlkDevice, &imageCreateInfo, nullptr, &g_vlkImage);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "Could Not Create Image :" << vlkRes << std::endl;
        cleanUp();
        exit(EXIT_FAILURE);
    }
    else
    {
        std::cout << "SAM : Vulkan Image Created" << std::endl;
    }

    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = nullptr;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.image = g_vlkImage;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    imageViewCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    vlkRes = vkCreateImageView(g_vlkDevice, &imageViewCreateInfo, nullptr, &g_vlkImageView);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "Could Not Create Image View:" << vlkRes << std::endl;
        cleanUp();
        exit(EXIT_FAILURE);
    }
    else
    {
        std::cout << "SAM : Vulkan Image View Created" << std::endl;
    }

    cleanUp();
    // Create the Device
    return EXIT_SUCCESS;
}

void cleanUp()
{
    if (g_vlkDevice)
    {
        vkDeviceWaitIdle(g_vlkDevice);

        if (g_vlkImageView)
        {
            vkDestroyImageView(g_vlkDevice, g_vlkImageView, nullptr);
            g_vlkImageView = VK_NULL_HANDLE;
        }

        if (g_vlkImage)
        {
            vkDestroyImage(g_vlkDevice, g_vlkImage, nullptr);
            g_vlkImage = nullptr;
        }

        vkDestroyDevice(g_vlkDevice, nullptr);
        g_vlkDevice = VK_NULL_HANDLE;
    }

    if (g_vlkInstance)
    {
        vkDestroyInstance(g_vlkInstance, nullptr);
        g_vlkInstance = VK_NULL_HANDLE;
    }
}

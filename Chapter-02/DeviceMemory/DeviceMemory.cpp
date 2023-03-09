#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

VkInstance g_vlkInstance = VK_NULL_HANDLE;
VkDevice g_vlkDevice = VK_NULL_HANDLE;
VkPhysicalDevice g_vlkRequiredPhysicalDevice = VK_NULL_HANDLE;

uint32_t g_queueFamilyIndex = 0;
uint32_t g_queueIndex = 0;
VkQueue g_vlkQueue = VK_NULL_HANDLE;

VkBuffer g_vlkTriVretexBuffer = VK_NULL_HANDLE; // This needs to be backed by Memory(VkMemory)
VkBufferView g_vlkBufferView = VK_NULL_HANDLE;

VkDeviceMemory g_vlkBufferDevicememory = VK_NULL_HANDLE;

void cleanUp(); // clean all gloabal resources
uint32_t FindMemoryIndexByType(uint32_t allowedTypeMask, VkMemoryPropertyFlags memoryPropertyFlags);

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
        std::cout << "deviceProp.limits.minMemoryMapAlignment = " << deviceProp.limits.minMemoryMapAlignment << std::endl;
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

    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(g_vlkRequiredPhysicalDevice, &physicalDeviceMemoryProperties);

    for (uint32_t iIdx = 0; iIdx < physicalDeviceMemoryProperties.memoryTypeCount; iIdx++)
    {
        std::cout << "SAM : PhysicalDeviceMemoryProperties.idx = " << iIdx << ", memoryTypes.propertyFlags " << physicalDeviceMemoryProperties.memoryTypes[iIdx].propertyFlags << ", memoryTypes.heapIndex " << physicalDeviceMemoryProperties.memoryTypes[iIdx].heapIndex << std::endl;
    }

    // for (uint32_t iIdx = 0; iIdx < physicalDeviceMemoryProperties.memoryHeapCount; iIdx++)
    //{
    //     std::cout << "SAM : PhysicalDeviceMemoryProperties.idx = " << iIdx << "memoryHeaps.flags" << physicalDeviceMemoryProperties.memoryHeaps[iIdx].flags << std::endl;
    // }

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

    // Crate 1MiB of data

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.flags = 0;
    bufferCreateInfo.size = 1024 * 1024;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.queueFamilyIndexCount = 0;
    bufferCreateInfo.pQueueFamilyIndices = nullptr;

    vlkRes = vkCreateBuffer(g_vlkDevice, &bufferCreateInfo, nullptr, &g_vlkTriVretexBuffer);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "Could Not create 1MiB data Buffer :" << vlkRes << std::endl;
        cleanUp();
        exit(EXIT_FAILURE);
    }
    else
    {
        std::cout << "SAM : 1MiB data Buffer Created" << std::endl;
    }

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(g_vlkDevice, g_vlkTriVretexBuffer, &memoryRequirements);

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.pNext = nullptr;
    memAllocInfo.allocationSize = bufferCreateInfo.size;
    memAllocInfo.memoryTypeIndex = FindMemoryIndexByType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    // memAllocInfo.memoryTypeIndex = 0;
    vlkRes = vkAllocateMemory(g_vlkDevice, &memAllocInfo, nullptr, &g_vlkBufferDevicememory);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "Could Not Allocate Buffer Memory :" << vlkRes << std::endl;
        cleanUp();
        exit(EXIT_FAILURE);
    }
    else
    {
        std::cout << "SAM : Allocated Buffer Memory" << std::endl;
    }

    vkBindBufferMemory(g_vlkDevice, g_vlkTriVretexBuffer, g_vlkBufferDevicememory, 0);

    VkBufferViewCreateInfo bufferViewCreateInfo{};
    bufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    bufferViewCreateInfo.flags = 0;
    bufferViewCreateInfo.pNext = nullptr;
    bufferViewCreateInfo.buffer = g_vlkTriVretexBuffer;
    bufferViewCreateInfo.format = VK_FORMAT_UNDEFINED;
    bufferViewCreateInfo.offset = 0;
    bufferViewCreateInfo.range = bufferCreateInfo.size;
    vlkRes = vkCreateBufferView(g_vlkDevice, &bufferViewCreateInfo, nullptr, &g_vlkBufferView);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "Could Not create Buffer View :" << vlkRes << std::endl;
        cleanUp();
        exit(EXIT_FAILURE);
    }
    else
    {
        std::cout << "SAM : Buffer View Created" << std::endl;
    }

    cleanUp();
    // Create the Device
    return EXIT_SUCCESS;
}

uint32_t FindMemoryIndexByType(uint32_t allowedTypeMask, VkMemoryPropertyFlags memoryPropertyFlags)
{
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(g_vlkRequiredPhysicalDevice, &memoryProperties);

    uint32_t typeMask = 1;
    for (uint32_t iDesiredIndewx = 0; iDesiredIndewx < memoryProperties.memoryTypeCount; iDesiredIndewx++, typeMask = typeMask << 1)
    {
        if ((typeMask & allowedTypeMask) != 0)
        {
            if ((memoryProperties.memoryTypes[iDesiredIndewx].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
            {
                std::cout << "SAM : FindMemoryIndexByType(), Got PropFlag " << memoryProperties.memoryTypes[iDesiredIndewx].propertyFlags << " Desired Prop Flaaaags " << memoryPropertyFlags << std::endl;
                std::cout << "SAM : FindMemoryIndexByType(), id Returned " << iDesiredIndewx << std::endl;
                return iDesiredIndewx;
            }
        }
    }

    return 0;
}

void cleanUp()
{
    if (g_vlkDevice)
    {
        vkDeviceWaitIdle(g_vlkDevice);

        if (g_vlkBufferDevicememory)
        {
            vkFreeMemory(g_vlkDevice, g_vlkBufferDevicememory, nullptr);
            g_vlkBufferDevicememory = VK_NULL_HANDLE;
        }

        if (g_vlkBufferView)
        {
            vkDestroyBufferView(g_vlkDevice, g_vlkBufferView, nullptr);
            g_vlkBufferView = VK_NULL_HANDLE;
        }

        if (g_vlkTriVretexBuffer)
        {
            vkDestroyBuffer(g_vlkDevice, g_vlkTriVretexBuffer, nullptr);
            g_vlkTriVretexBuffer = VK_NULL_HANDLE;
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

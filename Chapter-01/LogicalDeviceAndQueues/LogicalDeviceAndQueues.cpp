#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

VkInstance g_vlkInstance = VK_NULL_HANDLE;
VkDevice g_vlkDevice = VK_NULL_HANDLE;
VkPhysicalDevice g_vlkRequiredPhysicalDevice = VK_NULL_HANDLE;

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
    std::vector<char *> layerNames;
    layerNames.push_back("VK_LAYER_KHRONOS_validation");
    // layerNames.push_back("VK_LAYER_LUNARG_api_dump");
    //  layerNames.push_back("VK_LAYER_KHRONOS_synchronization2");  // Enable while using events, semaphores and Barriers.Basically for tracking the objects.
    //  layerNames.push_back("VK_LAYER_LUNARG_monitor");          //  display FPS on title bar only available in WIN32 and XLib, no other platforms.
    instancecreateInfo.enabledLayerCount = (uint32_t)layerNames.size();
    instancecreateInfo.ppEnabledLayerNames = layerNames.data();

    // Extensions
    // AS PER OUR USE, BUT TAKING ALL OF THEM.
    uint32_t iExtensionCount = 0;
    std::vector<VkExtensionProperties> extensionList;
    std::vector<char *> extensionNames;

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

    cleanUp();
    // Create the Device
    return EXIT_SUCCESS;
}

void cleanUp()
{
    if (g_vlkDevice)
    {
        vkDeviceWaitIdle(g_vlkDevice);
        vkDestroyDevice(g_vlkDevice, nullptr);
        g_vlkDevice = VK_NULL_HANDLE;
    }

    if (g_vlkInstance)
    {
        vkDestroyInstance(g_vlkInstance, nullptr);
        g_vlkInstance = VK_NULL_HANDLE;
    }
}

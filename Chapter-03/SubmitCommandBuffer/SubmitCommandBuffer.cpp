#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

#pragma comment(lib, "vulkan-1.lib")

enum queueIndices
{
    COMMAND_QUEUE = 0,
    PRESENT_QUEUE = 1,
    QUEUE_MAX_COUNT
};

VkInstance g_vlkInstance = VK_NULL_HANDLE;
VkPhysicalDevice g_vlkReqPhysicalDevice = VK_NULL_HANDLE;
VkDevice g_vlkDevice = VK_NULL_HANDLE;

VkQueue g_vlkCommandQueue = VK_NULL_HANDLE;
VkQueue g_vlkPresentationQueue = VK_NULL_HANDLE;

VkCommandPool g_vlkCommandPool = VK_NULL_HANDLE;
VkCommandBuffer g_vlkCommandBuffer = VK_NULL_HANDLE;

void cleanup();

int main(int argc, char **argv, char **envp)
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

    for (size_t iIdx = 0; iIdx < instanceLayerList.size(); iIdx++)
    {
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

    instanceCreateInfo.enabledLayerCount = (uint32_t)instanceLayerNames.size();
    instanceCreateInfo.ppEnabledLayerNames = instanceLayerNames.data();

    uint32_t iInstanceExtensionCount = 0;
    std::vector<VkExtensionProperties> instanceExtensionList;
    std::vector<char *> instanceExtensionNames;

    vkEnumerateInstanceExtensionProperties(nullptr, &iInstanceExtensionCount, nullptr);
    instanceExtensionList.resize(iInstanceExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &iInstanceExtensionCount, instanceExtensionList.data());

    for (size_t i = 0; i < instanceExtensionList.size(); i++)
    {
        instanceExtensionNames.push_back(instanceExtensionList[i].extensionName);
    }
    instanceExtensionList.shrink_to_fit();
    instanceExtensionNames.shrink_to_fit();

    instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensionNames.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionNames.data();

    vlkRes = vkCreateInstance(&instanceCreateInfo, nullptr, &g_vlkInstance);
    if (vlkRes != VK_SUCCESS)
    {
        cleanup();
        std::cout << "Failed to Create Instance" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<VkPhysicalDevice> physicalDeviceList;
    uint32_t iPhysicalDevices = 0;
    vkEnumeratePhysicalDevices(g_vlkInstance, &iPhysicalDevices, nullptr);
    physicalDeviceList.resize(iPhysicalDevices);
    vkEnumeratePhysicalDevices(g_vlkInstance, &iPhysicalDevices, physicalDeviceList.data());

    physicalDeviceList.shrink_to_fit();

    for (size_t i = 0; i < physicalDeviceList.size(); i++)
    {
        VkPhysicalDeviceProperties deviceProperties{};
        vkGetPhysicalDeviceProperties(physicalDeviceList[i], &deviceProperties);

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            g_vlkReqPhysicalDevice = physicalDeviceList[i];
        }
    }

    if (g_vlkReqPhysicalDevice == VK_NULL_HANDLE)
    {
        std::cout << "Could not get desired physical device" << std::endl;
        cleanup();
        return EXIT_FAILURE;
    }

    uint32_t iQueueFamilyIndex = -1;
    uint32_t iQueueFamilyPropertyCount = 0;
    std::vector<VkQueueFamilyProperties> queueFamilyPropList;
    vkGetPhysicalDeviceQueueFamilyProperties(g_vlkReqPhysicalDevice, &iQueueFamilyPropertyCount, nullptr);
    queueFamilyPropList.resize(iQueueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(g_vlkReqPhysicalDevice, &iQueueFamilyPropertyCount, queueFamilyPropList.data());

    for (size_t i = 0; i < queueFamilyPropList.size(); i++)
    {
        if (queueFamilyPropList[i].queueFlags & (VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT))
        {
            iQueueFamilyIndex = i;
        }
    }

    float fQueuePriorities[] = {1.0};
    VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = nullptr;
    deviceQueueCreateInfo.flags = 0;
    deviceQueueCreateInfo.queueFamilyIndex = iQueueFamilyIndex;
    deviceQueueCreateInfo.queueCount = 2;
    deviceQueueCreateInfo.pQueuePriorities = fQueuePriorities;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;

    uint32_t iDeviceLayerCount = 0;
    std::vector<VkLayerProperties> deviceLayerList;
    std::vector<char *> devLayerNames;

    vkEnumerateDeviceLayerProperties(g_vlkReqPhysicalDevice, &iDeviceLayerCount, nullptr);
    deviceLayerList.resize(iDeviceLayerCount);
    vkEnumerateDeviceLayerProperties(g_vlkReqPhysicalDevice, &iDeviceLayerCount, deviceLayerList.data());

    for (size_t i = 0; i < deviceLayerList.size(); i++)
    {
        devLayerNames.push_back(deviceLayerList[i].layerName);
    }

    deviceCreateInfo.enabledLayerCount = (uint32_t)devLayerNames.size();
    deviceCreateInfo.ppEnabledLayerNames = devLayerNames.data();

    uint32_t iDeviceExtensionCount = 0;
    std::vector<VkExtensionProperties> deviceExtensionList;
    std::vector<char *> devExtensionNames;

    vkEnumerateDeviceExtensionProperties(g_vlkReqPhysicalDevice, nullptr, &iDeviceExtensionCount, nullptr);
    deviceExtensionList.resize(iDeviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(g_vlkReqPhysicalDevice, nullptr, &iDeviceExtensionCount, deviceExtensionList.data());

    deviceCreateInfo.enabledExtensionCount = (uint32_t)devExtensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = devExtensionNames.data();
    vlkRes = vkCreateDevice(g_vlkReqPhysicalDevice, &deviceCreateInfo, nullptr, &g_vlkDevice);
    if (vlkRes != VK_SUCCESS)
    {
        cleanup();
        std::cout << "Failed to Create Device and Queues" << std::endl;
        return EXIT_FAILURE;
    }

    vkGetDeviceQueue(g_vlkDevice, iQueueFamilyIndex, COMMAND_QUEUE, &g_vlkCommandQueue);
    vkGetDeviceQueue(g_vlkDevice, iQueueFamilyIndex, PRESENT_QUEUE, &g_vlkPresentationQueue);

    std::cout << "g_vlkCommandQueue = " << g_vlkCommandQueue << std::endl;
    std::cout << "g_vlkPresentationQueue = " << g_vlkPresentationQueue << std::endl;

    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    commandPoolCreateInfo.queueFamilyIndex = iQueueFamilyIndex;
    vlkRes = vkCreateCommandPool(g_vlkDevice, &commandPoolCreateInfo, nullptr, &g_vlkCommandPool);
    if (vlkRes != VK_SUCCESS)
    {
        cleanup();
        std::cout << "Failed to Create Command Pool" << std::endl;
        return EXIT_FAILURE;
    }

    /*
        Note that we don’t tell Vulkan anything about the length or size of the command
        buffers we’re creating. The internal data structures representing device commands will generally vary
        too greatly for any unit of measurement, such as bytes or commands, to make much sense. Vulkan
        will manage the command buffer memory for you
    */
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandPool = g_vlkCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;
    vlkRes = vkAllocateCommandBuffers(g_vlkDevice, &commandBufferAllocateInfo, &g_vlkCommandBuffer);
    if (vlkRes != VK_SUCCESS)
    {
        cleanup();
        std::cout << "Failed to Obtain Command Buffers" << std::endl;
        return EXIT_FAILURE;
    }

    /*                  FRUTHER SECTION MUST GO IN A LOOP                            */

    /*              Reset should be called before recording the command Buffers             */
    // Reset Command buffer
    vlkRes = vkResetCommandBuffer(g_vlkCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    if (vlkRes != VK_SUCCESS)
    {
        cleanup();
        std::cout << "Failed to Reset Command Buffer" << std::endl;
        return EXIT_FAILURE;
    }

    // Reset all command buffeers in command pool
    /*
        Note:
        Command buffers allocated from the pool are not freed by vkResetCommandPool(), but all
        reenter their initial state as if they had been freshly allocated.
    */
    vlkRes = vkResetCommandPool(g_vlkDevice, g_vlkCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    if (vlkRes != VK_SUCCESS)
    {
        cleanup();
        std::cout << "Failed to Reset All Command Buffers in a Command Pool" << std::endl;
        return EXIT_FAILURE;
    }

    // Start Recording
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT re-use of command buffers
    commandBufferBeginInfo.pInheritanceInfo = nullptr;
    vlkRes = vkBeginCommandBuffer(g_vlkCommandBuffer, &commandBufferBeginInfo);
    if (vlkRes != VK_SUCCESS)
    {
        cleanup();
        std::cout << "Failed to Record Command Buffer" << std::endl;
        return EXIT_FAILURE;
    }

    /*
        Record Commands Only.
        RESET MUST BE CALLED BEFORE RECORDING.
        vkBeginCommandBuffer() sets some kind of marker for recording. If we call reset that maker is also gone.
    */

    // Stop Recording
    vlkRes = vkEndCommandBuffer(g_vlkCommandBuffer);
    if (vlkRes != VK_SUCCESS)
    {
        cleanup();
        std::cout << "Failed to End Recording Command Buffer" << std::endl;
        return EXIT_FAILURE;
    }

    // Submit Command Buffer
    VkSubmitInfo commandSubmitInfo{};
    commandSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    commandSubmitInfo.pNext = nullptr;
    commandSubmitInfo.commandBufferCount = 1;
    commandSubmitInfo.pCommandBuffers = &g_vlkCommandBuffer;
    vlkRes = vkQueueSubmit(g_vlkCommandQueue, 1, &commandSubmitInfo, VK_NULL_HANDLE);
    if (vlkRes != VK_SUCCESS)
    {
        cleanup();
        std::cout << "Failed to submit cmd Command Queue" << std::endl;
        return EXIT_FAILURE;
    }

    vkQueueWaitIdle(g_vlkCommandQueue);

    return EXIT_SUCCESS;
}

void cleanup()
{
    if (g_vlkCommandQueue)
    {
        vkQueueWaitIdle(g_vlkCommandQueue);
    }

    if (g_vlkPresentationQueue)
    {
        vkQueueWaitIdle(g_vlkPresentationQueue);
    }

    if (g_vlkDevice)
    {
        vkDeviceWaitIdle(g_vlkDevice);

        if (g_vlkCommandPool)
        {
            vkResetCommandPool(g_vlkDevice, g_vlkCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
            if (g_vlkCommandBuffer)
            {
                vkFreeCommandBuffers(g_vlkDevice, g_vlkCommandPool, 1, &g_vlkCommandBuffer);
                memset(g_vlkCommandBuffer, 0, sizeof(g_vlkCommandBuffer));
            }
            vkDestroyCommandPool(g_vlkDevice, g_vlkCommandPool, nullptr);
            g_vlkCommandPool = VK_NULL_HANDLE;
        }

        vkDestroyDevice(g_vlkDevice, nullptr); // Destroyes queues as well...!!
        g_vlkDevice = VK_NULL_HANDLE;
    }

    if (g_vlkInstance)
    {
        vkDestroyInstance(g_vlkInstance, nullptr);
        g_vlkInstance = VK_NULL_HANDLE;
    }
}

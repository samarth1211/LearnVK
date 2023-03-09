#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

#pragma comment(lib, "vulkan-1.lib")

enum queueIndices
{
    COMMAND_QUEUE = 0,
    PRESENT_QUEUE = 1,
    QUEUE_MAX
};

VkInstance g_vlkInstance = VK_NULL_HANDLE;
VkPhysicalDevice g_vlkReqPhysicalDevice = VK_NULL_HANDLE;
VkDevice g_vlkDevice = VK_NULL_HANDLE;

VkQueue g_vlkCommandQueue = VK_NULL_HANDLE;
VkQueue g_vlkPresentationQueue = VK_NULL_HANDLE;

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

    float fQueuePriorities[] = {1.0, 1.0};
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
        vkDestroyDevice(g_vlkDevice, nullptr); // Destroyes queues as well...!!
        g_vlkDevice = VK_NULL_HANDLE;
    }

    if (g_vlkInstance)
    {
        vkDestroyInstance(g_vlkInstance, nullptr);
        g_vlkInstance = VK_NULL_HANDLE;
    }
}

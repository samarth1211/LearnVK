#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

VkInstance g_vlkInstance = VK_NULL_HANDLE;

/*
VK_KHR_device_group_creation
VK_KHR_external_fence_capabilities          => requies VK_KHR_get_physical_device_properties2
VK_KHR_external_memory_capabilities         => requies VK_KHR_get_physical_device_properties2
VK_KHR_external_semaphore_capabilities      => requies VK_KHR_get_physical_device_properties2
VK_KHR_get_physical_device_properties2      => new entry points to query device features, device properties, and format properties in a way that can be easily extended by other 
                                               extensions, without introducing any further entry points
VK_KHR_get_surface_capabilities2            => new entry points to query device surface capabilities in a way that can be easily extended by other extensions, without 
                                               introducing any further entry points
VK_KHR_surface                              => introduces VkSurfaceKHR objects, which abstract native platform surface or window objects for use with Vulkan.
VK_KHR_surface_protected_capabilities       => requires VK_KHR_get_surface_capabilities2,
VK_KHR_win32_surface                        => Only needed for Surface to interact with Windows OS.
VK_EXT_debug_report                         => Old extension, kept as suppourt for older devices or especially for older android devices.
VK_EXT_debug_utils                          => Supported by VK_LAYER_KHRONOS_validation (latest as per documentation)
VK_EXT_swapchain_colorspace
VK_NV_external_memory_capabilities
VK_KHR_portability_enumeration
*/

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

    uint32_t iExtensionCount = 0;
    std::vector<VkExtensionProperties> extensionList;
    std::vector<char *> extensionNames;

    vkEnumerateInstanceExtensionProperties(nullptr, &iExtensionCount, nullptr);
    extensionList.resize(iExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &iExtensionCount, extensionList.data());

    for (size_t i = 0; i < extensionList.size(); i++)
    {
        extensionNames.push_back(extensionList[i].extensionName);
        std::cout << extensionList[i].extensionName << std::endl;
    }

    instancecreateInfo.enabledExtensionCount = (uint32_t)extensionNames.size();
    instancecreateInfo.ppEnabledExtensionNames = extensionNames.data();
    instancecreateInfo.enabledLayerCount = 0;
    instancecreateInfo.ppEnabledLayerNames = nullptr;

    vlkRes = vkCreateInstance(&instancecreateInfo, nullptr, &g_vlkInstance);
    if (vlkRes != VK_SUCCESS)
    {
        if (vlkRes == VK_ERROR_INCOMPATIBLE_DRIVER)
        {
            std::cout << "Incompatible DRIVER, ";
        }

        std::cout << "Could Not create instance" << std::endl;
        goto LAST_SA;
    }
    else
    {
        std::cout << "Vulkan Instance Created" << std::endl;
    }

LAST_SA:

    if (g_vlkInstance)
    {
        vkDestroyInstance(g_vlkInstance, nullptr);
        g_vlkInstance = VK_NULL_HANDLE;
    }
    return EXIT_SUCCESS;
}

#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

VkInstance g_vlkInstance = VK_NULL_HANDLE;

/*
Layers Present :
VK_LAYER_NV_optimus
VK_LAYER_OBS_HOOK
VK_LAYER_RENDERDOC_Capture    
VK_LAYER_VALVE_steam_overlay  
VK_LAYER_VALVE_steam_fossilize
VK_LAYER_EOS_Overlay
VK_LAYER_EOS_Overlay

VK_LAYER_LUNARG_api_dump      
VK_LAYER_LUNARG_gfxreconstruct      => Capturing and replay tool By LunarG, can be used for 'Driver Regression Tewsting', 'Architecture Simulation', 'Silicon Bringup', 
                                       and 'Bug Reporting'.
VK_LAYER_KHRONOS_synchronization2   => used for tracking Vulkan queue submission, events, and pipeline barriers.
VK_LAYER_KHRONOS_validation         => isolating incorrect usage, and in verifying that applications correctly use the API
VK_LAYER_LUNARG_monitor             => displays the real-time frame rate in frames-per-second in the application's title bar. It is only compatible with the Win32 and XCB 
                                       windowing systems and will not display the frame rate on other platforms.
VK_LAYER_LUNARG_screenshot          => Record Frames to Images, needs to set some environment variables
VK_LAYER_KHRONOS_profiles           => The Khronos Profiles Layer helps test across a wide range of hardware capabilities without requiring a physical copy of every device

*/

// Note : VkValidationFeatureEnableEXT::VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
// vkDeviceWaitIdle or vkQueueWaitIdle.

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

    vlkRes = vkEnumerateInstanceExtensionProperties(nullptr, &iExtensionCount, nullptr);
    if (vlkRes != VK_SUCCESS)
    {
        //goto LAST_SA;
        exit(EXIT_FAILURE);
    }
    extensionList.resize(iExtensionCount);
    vlkRes = vkEnumerateInstanceExtensionProperties(nullptr, &iExtensionCount, extensionList.data());
    if (vlkRes != VK_SUCCESS)
    {
        //goto LAST_SA;
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < extensionList.size(); i++)
    {
        extensionNames.push_back(extensionList[i].extensionName);
    }

    instancecreateInfo.enabledExtensionCount = (uint32_t)extensionNames.size();
    instancecreateInfo.ppEnabledExtensionNames = extensionNames.data();

    //uint32_t iLayerCount = 0;
    //std::vector<VkLayerProperties> layerList;
    std::vector<char *> layerNames;
    //vlkRes = vkEnumerateInstanceLayerProperties(&iLayerCount, nullptr);
    //if (vlkRes != VK_SUCCESS)
    //{
    //    goto LAST_SA;
    //}
    //layerList.resize(iLayerCount);
    //vlkRes = vkEnumerateInstanceLayerProperties(&iLayerCount, layerList.data());
    //if (vlkRes != VK_SUCCESS)
    //{
    //    goto LAST_SA;
    //}
    //for (size_t i = 0; i < layerList.size(); i++)
    //{
    //    layerNames.push_back(layerList[i].layerName);
    //    std::cout << layerList[i].layerName << std::endl;
    //}
    layerNames.push_back("VK_LAYER_KHRONOS_validation");
    layerNames.push_back("VK_LAYER_LUNARG_api_dump");
    //layerNames.push_back("VK_LAYER_KHRONOS_synchronization2");  // Enable while using events, semaphores and Barriers
    //layerNames.push_back("VK_LAYER_LUNARG_monitor");
    instancecreateInfo.enabledLayerCount = (uint32_t)layerNames.size();
    instancecreateInfo.ppEnabledLayerNames = layerNames.data();

    vlkRes = vkCreateInstance(&instancecreateInfo, nullptr, &g_vlkInstance);
    if (vlkRes != VK_SUCCESS)
    {
        if (vlkRes == VK_ERROR_INCOMPATIBLE_DRIVER)
        {
            std::cout << "Incompatible DRIVER, ";
        }

        std::cout << "Could Not create instance :" << vlkRes << std::endl;
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

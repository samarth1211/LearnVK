#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

#pragma comment(lib, "vulkan-1.lib")

// Instance
VkInstance g_vlkInstance = VK_NULL_HANDLE;
VkPhysicalDevice g_vlkPhysicalDevice = VK_NULL_HANDLE;

// Device
VkDevice g_vlkLogicalDevice = VK_NULL_HANDLE;
VkQueue g_vlkComputeQueue = VK_NULL_HANDLE;
VkCommandPool g_vlkComputeCommandPool = VK_NULL_HANDLE;
VkCommandBuffer g_vlkCommandBuffer = VK_NULL_HANDLE;

// Pipleine and Shader stage objects
VkPipeline g_vlkComputePipeline = VK_NULL_HANDLE;
VkPipelineLayout g_vlkPipelineLayout = VK_NULL_HANDLE;
VkShaderModule g_vlkComputeShaderModule = VK_NULL_HANDLE;

void CleanUp();
VkShaderModule CreateComputeShader(const char *);

int main(int argc, char **argv, char **envp)
{

    // Instance Creation

    VkResult vlkRes{};
    VkApplicationInfo vlkApplicationInfo{};
    vlkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vlkApplicationInfo.pNext = NULL;
    vlkApplicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    vlkApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vlkApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    vlkApplicationInfo.pApplicationName = "NoPipelineApp";
    vlkApplicationInfo.pEngineName = "NoEngine";

    std::vector<char *> layerNameList = {"VK_LAYER_KHRONOS_validation"};
    VkInstanceCreateInfo vlkInstanceCreateInfo{};
    vlkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vlkInstanceCreateInfo.pNext = nullptr;
    vlkInstanceCreateInfo.flags = 0;
    vlkInstanceCreateInfo.ppEnabledLayerNames = layerNameList.data();
    vlkInstanceCreateInfo.enabledLayerCount = (uint32_t)layerNameList.size();

    vlkRes = vkCreateInstance(&vlkInstanceCreateInfo, nullptr, &g_vlkInstance);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreateInstance() Failed" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    // PhysicalDevice
    std::vector<VkPhysicalDevice> physicalDeviceList;
    uint32_t iPhysicalDevCount;
    vlkRes = vkEnumeratePhysicalDevices(g_vlkInstance, &iPhysicalDevCount, nullptr);
    physicalDeviceList.resize(iPhysicalDevCount);
    vlkRes = vkEnumeratePhysicalDevices(g_vlkInstance, &iPhysicalDevCount, physicalDeviceList.data());

    // choose Physical Device
    for (uint32_t iPhysicalDeviceIdx = 0; iPhysicalDeviceIdx < physicalDeviceList.size(); iPhysicalDeviceIdx++)
    {
        VkPhysicalDeviceProperties vlkPhysicalDeviceProperties{};
        VkPhysicalDeviceFeatures vlkGetPhysicalDeviceFeatures{};

        vkGetPhysicalDeviceProperties(physicalDeviceList[iPhysicalDeviceIdx], &vlkPhysicalDeviceProperties);
        vkGetPhysicalDeviceFeatures(physicalDeviceList[iPhysicalDeviceIdx], &vlkGetPhysicalDeviceFeatures);

        if (vlkPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            g_vlkPhysicalDevice = physicalDeviceList[iPhysicalDeviceIdx];
            break;
        }
    }

    // Create Device

    uint32_t iQueueFamilyIndex = -1;
    uint32_t iQFamilyPropCount;
    std::vector<VkQueueFamilyProperties> QueueFailyPropList;
    vkGetPhysicalDeviceQueueFamilyProperties(g_vlkPhysicalDevice, &iQFamilyPropCount, nullptr);
    QueueFailyPropList.resize(iQFamilyPropCount);
    vkGetPhysicalDeviceQueueFamilyProperties(g_vlkPhysicalDevice, &iQFamilyPropCount, QueueFailyPropList.data());

    for (uint32_t iIdx = 0; iIdx < QueueFailyPropList.size(); iIdx++)
    {
        if (QueueFailyPropList[iIdx].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            iQueueFamilyIndex = iIdx;
            break;
        }
    }

    if (iQueueFamilyIndex == iQFamilyPropCount)
    {
        std::cout << "Can not get the Queue" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    float fQueuePriority[] = {1.0f};
    VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = nullptr;
    deviceQueueCreateInfo.flags = 0;
    deviceQueueCreateInfo.queueFamilyIndex = iQueueFamilyIndex;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = fQueuePriority;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    vlkRes = vkCreateDevice(g_vlkPhysicalDevice, &deviceCreateInfo, nullptr, &g_vlkLogicalDevice);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreateDevice() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    // Obtain the queue
    vkGetDeviceQueue(g_vlkLogicalDevice, iQueueFamilyIndex, 0, &g_vlkComputeQueue);

    // Create Compute Pipeline an pipeline Layout

    // Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    vlkRes = vkCreatePipelineLayout(g_vlkLogicalDevice, &pipelineLayoutCreateInfo, nullptr, &g_vlkPipelineLayout);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreatePipelineLayout() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    // Shader Module
    g_vlkComputeShaderModule = CreateComputeShader("Comp.spv");
    if (g_vlkComputeShaderModule == VK_NULL_HANDLE)
    {
        std::cout << "CreateComputeShader() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    // Pipeline
    VkComputePipelineCreateInfo createPipelineInfo{};
    createPipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createPipelineInfo.layout = g_vlkPipelineLayout;
    createPipelineInfo.basePipelineIndex = -1;
    createPipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createPipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    createPipelineInfo.stage.pName = "main";
    createPipelineInfo.stage.module = g_vlkComputeShaderModule;

    vlkRes = vkCreateComputePipelines(g_vlkLogicalDevice, VK_NULL_HANDLE, 1, &createPipelineInfo, nullptr, &g_vlkComputePipeline);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreateDevice() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    // Creat CommandPool
    VkCommandPoolCreateInfo commandPoolcreateInfo{};
    commandPoolcreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolcreateInfo.pNext = nullptr;
    commandPoolcreateInfo.flags = 0;
    commandPoolcreateInfo.queueFamilyIndex = iQueueFamilyIndex;
    vlkRes = vkCreateCommandPool(g_vlkLogicalDevice, &commandPoolcreateInfo, nullptr, &g_vlkComputeCommandPool);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreateCommandPool() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    // Prepare Command Buffer
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.commandPool = g_vlkComputeCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vlkRes = vkAllocateCommandBuffers(g_vlkLogicalDevice, &commandBufferAllocateInfo, &g_vlkCommandBuffer);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkAllocateCommandBuffers() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = 0;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    // Begin Recording the commands
    vlkRes = vkBeginCommandBuffer(g_vlkCommandBuffer, &commandBufferBeginInfo);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkBeginCommandBuffer() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    // Bind Pipeline
    vkCmdBindPipeline(g_vlkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, g_vlkComputePipeline);
    vkCmdDispatch(g_vlkCommandBuffer, 1, 1, 1);

    // Stop recording of commands
    vlkRes = vkEndCommandBuffer(g_vlkCommandBuffer);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkEndCommandBuffer() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    // Submit Command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &g_vlkCommandBuffer;

    vlkRes = vkQueueSubmit(g_vlkComputeQueue, 1, &submitInfo, NULL);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkQueueSubmit() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    CleanUp();

    return EXIT_SUCCESS;
}

void CleanUp()
{

    if (g_vlkLogicalDevice)
    {
        // Wai till device is using all the resources.
        vkDeviceWaitIdle(g_vlkLogicalDevice);

        if (g_vlkPipelineLayout)
        {
            vkDestroyPipelineLayout(g_vlkLogicalDevice, g_vlkPipelineLayout, nullptr);
            g_vlkPipelineLayout = VK_NULL_HANDLE;
        }

        if (g_vlkComputeShaderModule)
        {
            // can be destroyed as soon as pipeline is created
            vkDestroyShaderModule(g_vlkLogicalDevice, g_vlkComputeShaderModule, nullptr);
        }

        if (g_vlkComputePipeline)
        {
            vkDestroyPipeline(g_vlkLogicalDevice, g_vlkComputePipeline, nullptr);
            g_vlkComputePipeline = VK_NULL_HANDLE;
        }

        if (g_vlkComputeCommandPool)
        {
            vkDestroyCommandPool(g_vlkLogicalDevice, g_vlkComputeCommandPool, nullptr);
            g_vlkComputeCommandPool = VK_NULL_HANDLE;
        }
        vkDestroyDevice(g_vlkLogicalDevice, nullptr);
        g_vlkLogicalDevice = VK_NULL_HANDLE;
    }

    if (g_vlkInstance)
    {
        vkDestroyInstance(g_vlkInstance, nullptr);
        g_vlkInstance = VK_NULL_HANDLE;
    }
}

VkShaderModule CreateComputeShader(const char *fileName)
{
    uint8_t *shaderData = NULL;
    VkShaderModule shaderHandle = VK_NULL_HANDLE;
    size_t iFileSize = 0;
    FILE *pFile = NULL;
    if (fopen_s(&pFile, fileName, "rb") != 0)
    {
        std::cout << "CreateComputeShader : File " << fileName << " not found" << std::endl;
        return shaderHandle;
    }

    // get size of file
    fseek(pFile, 0, SEEK_END);
    iFileSize = ftell(pFile);
    rewind(pFile);

    shaderData = (uint8_t *)calloc(1, iFileSize);
    if (shaderData == NULL)
    {
        std::cout << "CreateComputeShader : Out of memory" << std::endl;
        return shaderHandle;
    }

    // copy the data
    fread_s(shaderData, iFileSize, iFileSize, 1, pFile);

    if (pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = iFileSize;
    shaderModuleCreateInfo.pCode = (uint32_t *)shaderData;

    VkResult vlkRes = vkCreateShaderModule(g_vlkLogicalDevice, &shaderModuleCreateInfo, nullptr, &shaderHandle);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "CreateComputeShader : vkCreateShaderModule() Failed " << vlkRes << std::endl;
        if (shaderData)
        {
            free(shaderData);
            shaderData = NULL;
        }
        return shaderHandle;
    }

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }

    return shaderHandle;
}

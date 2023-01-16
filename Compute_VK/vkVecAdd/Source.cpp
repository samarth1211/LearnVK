#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

#pragma comment(lib, "vulkan-1.lib")

#define NUM_ELEMENTS 11444777
#define LOCAL_SIZE 256

// Instance
VkInstance g_vlkInstance = VK_NULL_HANDLE;
VkPhysicalDevice g_vlkPhysicalDevice = VK_NULL_HANDLE;

// Device
VkDevice g_vlkLogicalDevice = VK_NULL_HANDLE;
VkQueue g_vlkComputeQueue = VK_NULL_HANDLE;

VkCommandPool g_vlkComputeCommandPool = VK_NULL_HANDLE;
VkCommandBuffer g_vlkCommandBuffer = VK_NULL_HANDLE;

VkDescriptorPool g_vlkDescriptorPool = VK_NULL_HANDLE;

// Pipleine and Shader stage objects
VkPipeline g_vlkComputePipeline = VK_NULL_HANDLE;
VkPipelineLayout g_vlkPipelineLayout = VK_NULL_HANDLE;
VkShaderModule g_vlkComputeShaderModule = VK_NULL_HANDLE;
VkDescriptorSetLayout g_vlkDescriptorSetLayout = VK_NULL_HANDLE;
VkDescriptorSet g_vlkDescriptorSet = VK_NULL_HANDLE;

// Buffers
VkBuffer g_vlkInputBuffer1 = VK_NULL_HANDLE;
VkBuffer g_vlkInputBuffer2 = VK_NULL_HANDLE;
VkBuffer g_vlkOutputBuffer = VK_NULL_HANDLE;
VkDeviceMemory g_vlkInputBuffer1Memory = VK_NULL_HANDLE;
VkDeviceMemory g_vlkInputBuffer2Memory = VK_NULL_HANDLE;
VkDeviceMemory g_vlkOutputBufferMemory = VK_NULL_HANDLE;

float InputData1[NUM_ELEMENTS]; // cpu to gpu
float InputData2[NUM_ELEMENTS]; // cpu to gpu
float OutputData[NUM_ELEMENTS]; // gpu to cpu

// Fence
VkFence gl_vlkFence = VK_NULL_HANDLE;

void CleanUp();
VkShaderModule CreateComputeShader(const char *);
uint32_t FindMemoryIndexByType(uint32_t allowedTypeMask, VkMemoryPropertyFlags flags);

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
    else
    {
        std::cout << "vkCreateInstance() Created" << std::endl;
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
            std::cout << "Physical Device ID : " << iPhysicalDeviceIdx << std::endl;
            // leave with the 1st best possible device.
            break;
        }
    }
    std::cout << "Physical Device handle " << g_vlkPhysicalDevice << std::endl;

    // Create Device

    uint32_t iQueueFamilyIndex = -1;
    uint32_t iQFamilyPropCount;
    std::vector<VkQueueFamilyProperties> QueueFailyPropList;
    vkGetPhysicalDeviceQueueFamilyProperties(g_vlkPhysicalDevice, &iQFamilyPropCount, nullptr);
    QueueFailyPropList.resize(iQFamilyPropCount);
    std::cout << "Found '" << iQFamilyPropCount << "' queue families" << std::endl;
    vkGetPhysicalDeviceQueueFamilyProperties(g_vlkPhysicalDevice, &iQFamilyPropCount, QueueFailyPropList.data());

    for (uint32_t iIdx = 0; iIdx < QueueFailyPropList.size(); iIdx++)
    {
        if (QueueFailyPropList[iIdx].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            iQueueFamilyIndex = iIdx;
            break;
        }
    }

    std::cout << "Chose queue family index " << iQueueFamilyIndex << std::endl;

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
    else
    {
        std::cout << "vkCreateDevice() Created" << std::endl;
    }

    // Obtain the queue
    vkGetDeviceQueue(g_vlkLogicalDevice, iQueueFamilyIndex, 0, &g_vlkComputeQueue);

    // Create Compute Pipeline an pipeline Layout
    // DescriptoerSetLayout

    VkDescriptorSetLayoutBinding bindings[3]{};
    bindings[0].binding = 0;
    bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 1;

    bindings[1].binding = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;

    bindings[2].binding = 2;
    bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[2].descriptorCount = 1;

    VkDescriptorSetLayoutCreateInfo descriptorLayoutSetInfo{};
    descriptorLayoutSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayoutSetInfo.pNext = nullptr;
    descriptorLayoutSetInfo.flags = 0;
    descriptorLayoutSetInfo.bindingCount = 3;
    descriptorLayoutSetInfo.pBindings = bindings;
    vlkRes = vkCreateDescriptorSetLayout(g_vlkLogicalDevice, &descriptorLayoutSetInfo, nullptr, &g_vlkDescriptorSetLayout);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreateDescriptorSetLayout() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkCreateDescriptorSetLayout() Created" << std::endl;
    }

    // Pipeline Layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &g_vlkDescriptorSetLayout;

    vlkRes = vkCreatePipelineLayout(g_vlkLogicalDevice, &pipelineLayoutCreateInfo, nullptr, &g_vlkPipelineLayout);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreatePipelineLayout() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkCreatePipelineLayout() Created" << std::endl;
    }

    // Shader Module
    g_vlkComputeShaderModule = CreateComputeShader("Comp.spv");
    if (g_vlkComputeShaderModule == VK_NULL_HANDLE)
    {
        std::cout << "CreateComputeShader() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "CreateComputeShader() Completed" << std::endl;
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
        std::cout << "vkCreateComputePipelines() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkCreateComputePipelines() Complete" << std::endl;
    }

    // Descriptor Set and Descriptor Pool
    // Desriptor Pool
    VkDescriptorPoolSize despriptorPoolSize{};
    despriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    despriptorPoolSize.descriptorCount = 3;

    VkDescriptorPoolCreateInfo despriptorPoolCreateInfo{};
    despriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    despriptorPoolCreateInfo.pNext = nullptr;
    despriptorPoolCreateInfo.flags = 0;
    despriptorPoolCreateInfo.maxSets = 1;
    despriptorPoolCreateInfo.poolSizeCount = 1;
    despriptorPoolCreateInfo.pPoolSizes = &despriptorPoolSize;
    vlkRes = vkCreateDescriptorPool(g_vlkLogicalDevice, &despriptorPoolCreateInfo, nullptr, &g_vlkDescriptorPool);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreateDescriptorPool() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkCreateDescriptorPool() Complete" << std::endl;
    }

    // Descriptor Set
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &g_vlkDescriptorSetLayout;
    descriptorSetAllocateInfo.descriptorPool = g_vlkDescriptorPool;
    vlkRes = vkAllocateDescriptorSets(g_vlkLogicalDevice, &descriptorSetAllocateInfo, &g_vlkDescriptorSet);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkAllocateDescriptorSets() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    // Create Buffers
    VkBufferCreateInfo inputBuffer1CreateInfo{};
    inputBuffer1CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    inputBuffer1CreateInfo.flags = 0;
    inputBuffer1CreateInfo.pNext = nullptr;
    inputBuffer1CreateInfo.size = sizeof(float) * NUM_ELEMENTS;
    inputBuffer1CreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    vlkRes = vkCreateBuffer(g_vlkLogicalDevice, &inputBuffer1CreateInfo, nullptr, &g_vlkInputBuffer1);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreateBuffer(g_vlkInputBuffer1) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkCreateBuffer(g_vlkInputBuffer1) Complete" << std::endl;
    }

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(g_vlkLogicalDevice, g_vlkInputBuffer1, &memoryRequirements);

    VkMemoryAllocateInfo memAllocInfoInput1Buffer{};
    memAllocInfoInput1Buffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfoInput1Buffer.pNext = nullptr;
    memAllocInfoInput1Buffer.allocationSize = memoryRequirements.size;
    memAllocInfoInput1Buffer.memoryTypeIndex = FindMemoryIndexByType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vlkRes = vkAllocateMemory(g_vlkLogicalDevice, &memAllocInfoInput1Buffer, nullptr, &g_vlkInputBuffer1Memory);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkAllocateMemory(g_vlkInputBuffer1Memory) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    vlkRes = vkBindBufferMemory(g_vlkLogicalDevice, g_vlkInputBuffer1, g_vlkInputBuffer1Memory, 0);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkBindBufferMemory(g_vlkInputBuffer1, g_vlkInputBuffer1Memory) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    //
    VkBufferCreateInfo inputBuffer2CreateInfo{};
    inputBuffer2CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    inputBuffer2CreateInfo.flags = 0;
    inputBuffer2CreateInfo.pNext = nullptr;
    inputBuffer2CreateInfo.size = sizeof(float) * NUM_ELEMENTS;
    inputBuffer2CreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    vlkRes = vkCreateBuffer(g_vlkLogicalDevice, &inputBuffer2CreateInfo, nullptr, &g_vlkInputBuffer2);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreateBuffer(g_vlkInputBuffer2) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkCreateBuffer(g_vlkInputBuffer2) Complete" << std::endl;
    }

    memset(&memoryRequirements, 0, sizeof(memoryRequirements));
    vkGetBufferMemoryRequirements(g_vlkLogicalDevice, g_vlkInputBuffer2, &memoryRequirements);

    VkMemoryAllocateInfo memAllocInfoInput2Buffer{};
    memAllocInfoInput2Buffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfoInput2Buffer.pNext = nullptr;
    memAllocInfoInput2Buffer.allocationSize = memoryRequirements.size;
    memAllocInfoInput2Buffer.memoryTypeIndex = FindMemoryIndexByType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vlkRes = vkAllocateMemory(g_vlkLogicalDevice, &memAllocInfoInput2Buffer, nullptr, &g_vlkInputBuffer2Memory);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkAllocateMemory(g_vlkInputBuffer2Memory) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    vlkRes = vkBindBufferMemory(g_vlkLogicalDevice, g_vlkInputBuffer2, g_vlkInputBuffer2Memory, 0);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkBindBufferMemory(g_vlkInputBuffer2, g_vlkInputBuffer2Memory) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }

    //
    VkBufferCreateInfo outputBufferCreateInfo{};
    outputBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    outputBufferCreateInfo.flags = 0;
    outputBufferCreateInfo.pNext = nullptr;
    outputBufferCreateInfo.size = sizeof(float) * NUM_ELEMENTS;
    outputBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    vlkRes = vkCreateBuffer(g_vlkLogicalDevice, &outputBufferCreateInfo, nullptr, &g_vlkOutputBuffer);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreateBuffer(g_vlkOutputBuffer) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkCreateBuffer(g_vlkOutputBuffer) Complete" << std::endl;
    }

    memset(&memoryRequirements, 0, sizeof(memoryRequirements));
    vkGetBufferMemoryRequirements(g_vlkLogicalDevice, g_vlkOutputBuffer, &memoryRequirements);

    VkMemoryAllocateInfo memAllocInfoOutputBuffer{};
    memAllocInfoOutputBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfoOutputBuffer.pNext = nullptr;
    memAllocInfoOutputBuffer.allocationSize = memoryRequirements.size;
    memAllocInfoOutputBuffer.memoryTypeIndex = FindMemoryIndexByType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vlkRes = vkAllocateMemory(g_vlkLogicalDevice, &memAllocInfoOutputBuffer, nullptr, &g_vlkOutputBufferMemory);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkAllocateMemory(g_vlkOutputBufferMemory) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkAllocateMemory(g_vlkOutputBufferMemory) Complete" << std::endl;
    }

    vlkRes = vkBindBufferMemory(g_vlkLogicalDevice, g_vlkOutputBuffer, g_vlkOutputBufferMemory, 0);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkBindBufferMemory(g_vlkOutputBuffer, g_vlkOutputBufferMemory) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkBindBufferMemory(g_vlkOutputBuffer, g_vlkOutputBufferMemory) Complete" << std::endl;
    }

    VkDescriptorBufferInfo descriptorBuffers[3]{};
    descriptorBuffers[0].buffer = g_vlkInputBuffer1;
    descriptorBuffers[0].offset = 0;
    descriptorBuffers[0].range = sizeof(float) * NUM_ELEMENTS;

    descriptorBuffers[1].buffer = g_vlkInputBuffer2;
    descriptorBuffers[1].offset = 0;
    descriptorBuffers[1].range = sizeof(float) * NUM_ELEMENTS;

    descriptorBuffers[2].buffer = g_vlkOutputBuffer;
    descriptorBuffers[2].offset = 0;
    descriptorBuffers[2].range = sizeof(float) * NUM_ELEMENTS;

    VkWriteDescriptorSet writeDecriptorSet{};
    writeDecriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDecriptorSet.dstSet = g_vlkDescriptorSet;
    writeDecriptorSet.dstBinding = 0;
    writeDecriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDecriptorSet.descriptorCount = 3;
    writeDecriptorSet.pBufferInfo = descriptorBuffers;

    vkUpdateDescriptorSets(g_vlkLogicalDevice, 1, &writeDecriptorSet, 0, nullptr);

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
    else
    {
        std::cout << "vkCreateCommandPool() Complete" << std::endl;
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
    else
    {
        std::cout << "vkAllocateCommandBuffers() Complete" << std::endl;
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
    else
    {
        std::cout << "vkBeginCommandBuffer() Complete" << std::endl;
    }

    // Bind Pipeline
    vkCmdBindPipeline(g_vlkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, g_vlkComputePipeline);
    vkCmdBindDescriptorSets(g_vlkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, g_vlkPipelineLayout, 0, 1, &g_vlkDescriptorSet, 0, nullptr);

    uint32_t rem = NUM_ELEMENTS / LOCAL_SIZE;
    uint32_t iLocalGropuCount = 0;
    if (rem == 0)
    {
        iLocalGropuCount = NUM_ELEMENTS;
    }
    else
    {
        iLocalGropuCount = NUM_ELEMENTS + LOCAL_SIZE - rem;
    }

    vkCmdDispatch(g_vlkCommandBuffer, iLocalGropuCount, 1, 1);

    // Stop recording of commands
    vlkRes = vkEndCommandBuffer(g_vlkCommandBuffer);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkEndCommandBuffer() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkEndCommandBuffer() Complete" << std::endl;
    }

    // Prepare the data and buffers to push to GPU
    for (uint32_t i = 0; i < NUM_ELEMENTS; i++)
    {
        InputData1[i] = i;
        InputData2[i] = i;
        OutputData[i] = 0.0f;
    }

    // Copy to input buffer
    void *inputAddress1;

    vlkRes = vkMapMemory(g_vlkLogicalDevice, g_vlkInputBuffer1Memory, 0, sizeof(float) * NUM_ELEMENTS, 0, &inputAddress1);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkMapMemory(g_vlkInputBuffer1Memory) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkMapMemory(g_vlkInputBuffer1Memory) Complete" << std::endl;
    }

    memcpy(inputAddress1, InputData1, sizeof(float) * NUM_ELEMENTS);

    vkUnmapMemory(g_vlkLogicalDevice, g_vlkInputBuffer1Memory);
    //
    void *inputAddress2;
    vlkRes = vkMapMemory(g_vlkLogicalDevice, g_vlkInputBuffer2Memory, 0, sizeof(float) * NUM_ELEMENTS, 0, &inputAddress2);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkMapMemory(g_vlkInputBuffer2Memory) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkMapMemory(g_vlkInputBuffer2Memory) Complete" << std::endl;
    }
    memcpy(inputAddress2, InputData2, sizeof(float) * NUM_ELEMENTS);
    vkUnmapMemory(g_vlkLogicalDevice, g_vlkInputBuffer2Memory);

    // Submit Command buffer
    // Create a fence
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = 0;
    vlkRes = vkCreateFence(g_vlkLogicalDevice, &fenceCreateInfo, nullptr, &gl_vlkFence);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkCreateFence() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkCreateFence() Complete" << std::endl;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &g_vlkCommandBuffer;

    vlkRes = vkQueueSubmit(g_vlkComputeQueue, 1, &submitInfo, gl_vlkFence);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkQueueSubmit() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkQueueSubmit() Complete" << std::endl;
    }

    vlkRes = vkWaitForFences(g_vlkLogicalDevice, 1, &gl_vlkFence, VK_TRUE, UINT_MAX);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkWaitForFences() FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkWaitForFences() Complete" << std::endl;
    }

    // Copy the output from gpu
    void *outputAddress;
    vlkRes = vkMapMemory(g_vlkLogicalDevice, g_vlkOutputBufferMemory, 0, sizeof(float) * NUM_ELEMENTS, 0, &outputAddress);
    if (vlkRes != VK_SUCCESS)
    {
        std::cout << "vkMapMemory(g_vlkOutputBufferMemory) FAILED" << std::endl;
        CleanUp();
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "vkMapMemory(g_vlkOutputBufferMemory) Complete" << std::endl;
    }

    memcpy(OutputData, outputAddress, sizeof(float) * NUM_ELEMENTS);

    vkUnmapMemory(g_vlkLogicalDevice, g_vlkOutputBufferMemory);

    CleanUp();

    // Verify data
    std::cout << "Display Data \n"
              << std::ends << std::endl;

    for (int iIdx = 0; iIdx < NUM_ELEMENTS; iIdx++)
    {
        std::cout << "idx => " << iIdx << " = " << OutputData[iIdx] << std::endl;
    }

    return EXIT_SUCCESS;
}

void CleanUp()
{
    std::cout << "\n"
              << std::endl;

    if (g_vlkLogicalDevice)
    {
        if (gl_vlkFence)
        {
            vkDestroyFence(g_vlkLogicalDevice, gl_vlkFence, nullptr);
            gl_vlkFence = VK_NULL_HANDLE;
        }
        std::cout << "Device is waiting, for the job to compplete" << std::endl;
        // Wait till device is using all the resources.
        vkDeviceWaitIdle(g_vlkLogicalDevice);
        std::cout << "Device waiting, OVER" << std::endl;

        if (g_vlkPipelineLayout)
        {
            vkDestroyPipelineLayout(g_vlkLogicalDevice, g_vlkPipelineLayout, nullptr);
            g_vlkPipelineLayout = VK_NULL_HANDLE;
            std::cout << "Pipeline Layout Destroyed" << std::endl;
        }

        if (g_vlkDescriptorSetLayout)
        {
            vkDestroyDescriptorSetLayout(g_vlkLogicalDevice, g_vlkDescriptorSetLayout, nullptr);
            g_vlkDescriptorSetLayout = VK_NULL_HANDLE;
            std::cout << "Descriptor Set Layout Destroyed" << std::endl;
        }

        if (g_vlkComputeShaderModule)
        {
            // can be destroyed as soon as pipeline is created
            vkDestroyShaderModule(g_vlkLogicalDevice, g_vlkComputeShaderModule, nullptr);
            std::cout << "Shader Module Destroyed" << std::endl;
        }

        if (g_vlkComputePipeline)
        {
            vkDestroyPipeline(g_vlkLogicalDevice, g_vlkComputePipeline, nullptr);
            g_vlkComputePipeline = VK_NULL_HANDLE;
            std::cout << "Pipeline Destroyed" << std::endl;
        }

        /*if (g_vlkDescriptorSet)
        {
            vkFreeDescriptorSet(g_vlkLogicalDevice, g_vlkDescriptorSet);
            std::cout << "DescriptorSet Destroyed" << std::endl;
        }*/

        if (g_vlkDescriptorPool)
        {
            vkDestroyDescriptorPool(g_vlkLogicalDevice, g_vlkDescriptorPool, nullptr);
            g_vlkDescriptorPool = VK_NULL_HANDLE;
            std::cout << "Descriptor Pool Destroyed" << std::endl;
        }

        if (g_vlkInputBuffer1)
        {
            vkDestroyBuffer(g_vlkLogicalDevice, g_vlkInputBuffer1, nullptr);
            g_vlkInputBuffer1 = VK_NULL_HANDLE;
            vkFreeMemory(g_vlkLogicalDevice, g_vlkInputBuffer1Memory, nullptr);
            g_vlkInputBuffer1Memory = VK_NULL_HANDLE;

            std::cout << "Input Buffer Destroyed" << std::endl;
        }

        if (g_vlkOutputBuffer)
        {
            vkDestroyBuffer(g_vlkLogicalDevice, g_vlkOutputBuffer, nullptr);
            g_vlkOutputBuffer = VK_NULL_HANDLE;
            vkFreeMemory(g_vlkLogicalDevice, g_vlkOutputBufferMemory, nullptr);
            g_vlkOutputBufferMemory = VK_NULL_HANDLE;
            std::cout << "Output Buffer Destroyed" << std::endl;
        }

        if (g_vlkCommandBuffer)
        {
            vkFreeCommandBuffers(g_vlkLogicalDevice, g_vlkComputeCommandPool, 1, &g_vlkCommandBuffer);
            g_vlkCommandBuffer = VK_NULL_HANDLE;
            std::cout << "CommandBuffer Freed" << std::endl;
        }

        if (g_vlkComputeCommandPool)
        {
            vkDestroyCommandPool(g_vlkLogicalDevice, g_vlkComputeCommandPool, nullptr);
            g_vlkComputeCommandPool = VK_NULL_HANDLE;
            std::cout << "Command Pool Destroyed" << std::endl;
        }
        vkDestroyDevice(g_vlkLogicalDevice, nullptr);
        g_vlkLogicalDevice = VK_NULL_HANDLE;
        std::cout << "Logical Device Destroyed" << std::endl;
    }

    if (g_vlkInstance)
    {
        vkDestroyInstance(g_vlkInstance, nullptr);
        g_vlkInstance = VK_NULL_HANDLE;
        std::cout << "Instance Destroyed" << std::endl;
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

    shaderData = (uint8_t *)calloc(1, iFileSize + 1);
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
    else
    {
        std::cout << "CreateComputeShader : vkCreateShaderModule() Cpomplete " << vlkRes << std::endl;
    }

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }

    return shaderHandle;
}

uint32_t FindMemoryIndexByType(uint32_t allowedTypeMask, VkMemoryPropertyFlags flags)
{
    std::cout << "In FindMemoryIndexByType()" << std::endl;

    VkPhysicalDeviceMemoryProperties memoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(g_vlkPhysicalDevice, &memoryProperties);

    uint32_t typeMask = 1;
    for (size_t i = 0; i < memoryProperties.memoryTypeCount; i++, typeMask <<= 1)
    {
        if ((allowedTypeMask & typeMask) != 0)
        {
            if ((memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
            {
                return i;
            }
        }
    }

    std::cout << "Failed to find memory index" << std::endl;
    return 0;
}

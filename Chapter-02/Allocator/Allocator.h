#pragma once
#include <cstdlib>
#include <vulkan/vulkan.h>

class CAllocator
{
public:
    // Operator that allows an instance of this class to be used as a
    // VkAllocationCallbacks structure
    inline operator VkAllocationCallbacks() const
    {
        VkAllocationCallbacks result;
        result.pUserData = (void *)this;
        result.pfnAllocation = &Allocation;
        result.pfnReallocation = &Reallocation;
        result.pfnFree = &Free;
        result.pfnInternalAllocation = nullptr;
        result.pfnInternalFree = nullptr;

        return result;
    }

private:
    static void *VKAPI_CALL Allocation(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope scope);
    static void *VKAPI_CALL Reallocation(void *pUserData, void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope scope);
    static void VKAPI_CALL Free(void *pUserData, void *pMemory);

    // Declare NonStatic Member function which will do the allocations
    void *Allocation(size_t size, size_t alignment, VkSystemAllocationScope scope);
    void *Reallocation(void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope scope);
    void Free(void *pMemory);
    /* data */
};

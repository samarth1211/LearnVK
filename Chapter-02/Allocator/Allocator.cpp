#include "Allocator.h"

void *CAllocator::Allocation(size_t size, size_t alignment, VkSystemAllocationScope scope)
{
    return _aligned_malloc(size, alignment);
}

void *VKAPI_CALL CAllocator::Allocation(void *pUserData, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
    return static_cast<CAllocator *>(pUserData)->Allocation(size, alignment, scope);
}

void *CAllocator::Reallocation(void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
    return _aligned_realloc(pOriginal, size, alignment);
}

void *VKAPI_CALL CAllocator::Reallocation(void *pUserData, void *pOriginal, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
    return static_cast<CAllocator *>(pUserData)->Reallocation(pOriginal, size, alignment, scope);
}

void CAllocator::Free(void *pMemory)
{
    _aligned_free(pMemory);
}

void VKAPI_CALL CAllocator::Free(void *pUserData, void *pMemory)
{
    static_cast<CAllocator *>(pUserData)->Free(pMemory);
}

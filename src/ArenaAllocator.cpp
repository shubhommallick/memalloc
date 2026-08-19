#include "ArenaAllocator.hpp"
#include <cstdlib>
#include <iostream>

ArenaAllocator::ArenaAllocator(size_t capacity)
    : totalCapacity(capacity), currentOffset(0) {
    memoryBuffer = static_cast<uint8_t*>(std::malloc(totalCapacity));
    if (!memoryBuffer) {
        throw std::bad_alloc();
    }
}

ArenaAllocator::~ArenaAllocator() {
    if (memoryBuffer) {
        std::free(memoryBuffer);
        memoryBuffer = nullptr;
    }
}

uintptr_t ArenaAllocator::alignForward(uintptr_t ptr, size_t alignment) {
    if (alignment == 0) return ptr;
    uintptr_t remainder = ptr % alignment;
    if (remainder == 0) return ptr;
    return ptr + (alignment - remainder);
}

void* ArenaAllocator::allocate(size_t size, size_t alignment) {
    uintptr_t currentAddress = reinterpret_cast<uintptr_t>(memoryBuffer) + currentOffset;
    uintptr_t alignedAddress = alignForward(currentAddress, alignment);
    size_t padding = alignedAddress - currentAddress;

    if (currentOffset + padding + size > totalCapacity) {
        std::cerr << "[ArenaAllocator] Out of Memory! Required: " 
                  << (size + padding) << " bytes, Remaining: " 
                  << getRemainingMemory() << " bytes\n";
        return nullptr;
    }

    currentOffset += (padding + size);
    void* allocatedPtr = reinterpret_cast<void*>(alignedAddress);

    MemoryTracker::getInstance().recordAllocation(allocatedPtr, size, "Arena");
    return allocatedPtr;
}

void ArenaAllocator::deallocate(void* ptr, size_t size) {
    // Individual deallocations in Arena are no-ops. Reset is used for total cleanup.
    MemoryTracker::getInstance().recordDeallocation(ptr, size);
}

void ArenaAllocator::reset() {
    currentOffset = 0;
}

#ifndef FREE_LIST_ALLOCATOR_HPP
#define FREE_LIST_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include "MemoryTracker.hpp"

class FreeListAllocator {
public:
    struct BlockHeader {
        size_t size;        // Size of data payload
        bool isFree;        // Status flag
        BlockHeader* next;  // Next block in memory order
        BlockHeader* prev;  // Prev block in memory order
    };

    explicit FreeListAllocator(size_t capacity);
    ~FreeListAllocator();

    FreeListAllocator(const FreeListAllocator&) = delete;
    FreeListAllocator& operator=(const FreeListAllocator&) = delete;

    void* allocate(size_t size);
    void deallocate(void* ptr);
    void reset();

    size_t getCapacity() const { return totalCapacity; }
    size_t getUsedMemory() const { return usedMemory; }
    size_t getFreeMemory() const { return totalCapacity - usedMemory; }
    const BlockHeader* getHeadBlock() const { return headBlock; }
    const void* getBufferStart() const { return memoryBuffer; }

private:
    void coalesce(BlockHeader* block);

    uint8_t* memoryBuffer{nullptr};
    size_t totalCapacity{0};
    size_t usedMemory{0};
    BlockHeader* headBlock{nullptr};
};

#endif // FREE_LIST_ALLOCATOR_HPP

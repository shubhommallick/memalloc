#ifndef POOL_ALLOCATOR_HPP
#define POOL_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include "MemoryTracker.hpp"

class PoolAllocator {
public:
    PoolAllocator(size_t blockSize, size_t blockCount);
    ~PoolAllocator();

    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;

    void* allocate();
    void deallocate(void* ptr);
    void reset();

    size_t getBlockSize() const { return blockSize; }
    size_t getBlockCount() const { return totalBlocks; }
    size_t getUsedBlocks() const { return usedBlocks; }
    size_t getFreeBlocks() const { return totalBlocks - usedBlocks; }
    const void* getBufferStart() const { return memoryBuffer; }

private:
    struct Node {
        Node* next;
    };

    uint8_t* memoryBuffer{nullptr};
    size_t blockSize{0};
    size_t totalBlocks{0};
    size_t usedBlocks{0};
    Node* freeListHead{nullptr};
};

#endif // POOL_ALLOCATOR_HPP

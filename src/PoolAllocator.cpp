#include "PoolAllocator.hpp"
#include <cstdlib>
#include <iostream>
#include <algorithm>

PoolAllocator::PoolAllocator(size_t chunkBlockSize, size_t blockCount)
    : totalBlocks(blockCount), usedBlocks(0) {
    // Ensure block size is at least large enough to store Node pointer
    blockSize = std::max(chunkBlockSize, sizeof(Node));

    size_t totalSize = blockSize * totalBlocks;
    memoryBuffer = static_cast<uint8_t*>(std::malloc(totalSize));
    if (!memoryBuffer) {
        throw std::bad_alloc();
    }

    reset();
}

PoolAllocator::~PoolAllocator() {
    if (memoryBuffer) {
        std::free(memoryBuffer);
        memoryBuffer = nullptr;
    }
}

void PoolAllocator::reset() {
    usedBlocks = 0;
    freeListHead = reinterpret_cast<Node*>(memoryBuffer);
    
    Node* current = freeListHead;
    for (size_t i = 0; i < totalBlocks - 1; ++i) {
        uint8_t* nextBlock = reinterpret_cast<uint8_t*>(current) + blockSize;
        current->next = reinterpret_cast<Node*>(nextBlock);
        current = current->next;
    }
    current->next = nullptr;
}

void* PoolAllocator::allocate() {
    if (freeListHead == nullptr) {
        std::cerr << "[PoolAllocator] Pool exhausted! No free blocks remaining.\n";
        return nullptr;
    }

    Node* allocatedNode = freeListHead;
    freeListHead = freeListHead->next;
    usedBlocks++;

    void* ptr = static_cast<void*>(allocatedNode);
    MemoryTracker::getInstance().recordAllocation(ptr, blockSize, "Pool");
    return ptr;
}

void PoolAllocator::deallocate(void* ptr) {
    if (!ptr) return;

    // Push freed block back to free list stack
    Node* node = static_cast<Node*>(ptr);
    node->next = freeListHead;
    freeListHead = node;
    
    if (usedBlocks > 0) {
        usedBlocks--;
    }

    MemoryTracker::getInstance().recordDeallocation(ptr, blockSize);
}

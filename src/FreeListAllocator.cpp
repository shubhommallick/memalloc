#include "FreeListAllocator.hpp"
#include <cstdlib>
#include <iostream>

FreeListAllocator::FreeListAllocator(size_t capacity)
    : totalCapacity(capacity), usedMemory(0) {
    memoryBuffer = static_cast<uint8_t*>(std::malloc(totalCapacity));
    if (!memoryBuffer) {
        throw std::bad_alloc();
    }
    reset();
}

FreeListAllocator::~FreeListAllocator() {
    if (memoryBuffer) {
        std::free(memoryBuffer);
        memoryBuffer = nullptr;
    }
}

void FreeListAllocator::reset() {
    usedMemory = sizeof(BlockHeader);
    headBlock = reinterpret_cast<BlockHeader*>(memoryBuffer);
    headBlock->size = totalCapacity - sizeof(BlockHeader);
    headBlock->isFree = true;
    headBlock->next = nullptr;
    headBlock->prev = nullptr;
}

void* FreeListAllocator::allocate(size_t requestedSize) {
    if (requestedSize == 0) return nullptr;

    BlockHeader* current = headBlock;

    // First-Fit search for free block of sufficient size
    while (current) {
        if (current->isFree && current->size >= requestedSize) {
            // Check if block can be split into payload + new free header
            if (current->size >= requestedSize + sizeof(BlockHeader) + 8) {
                uint8_t* currentBytePtr = reinterpret_cast<uint8_t*>(current);
                uint8_t* newBlockPtr = currentBytePtr + sizeof(BlockHeader) + requestedSize;
                
                BlockHeader* newBlock = reinterpret_cast<BlockHeader*>(newBlockPtr);
                newBlock->size = current->size - requestedSize - sizeof(BlockHeader);
                newBlock->isFree = true;
                newBlock->next = current->next;
                newBlock->prev = current;

                if (current->next) {
                    current->next->prev = newBlock;
                }

                current->next = newBlock;
                current->size = requestedSize;
            }

            current->isFree = false;
            usedMemory += current->size;

            void* dataPtr = reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(current) + sizeof(BlockHeader));
            MemoryTracker::getInstance().recordAllocation(dataPtr, current->size, "FreeList");
            return dataPtr;
        }
        current = current->next;
    }

    std::cerr << "[FreeListAllocator] Out of Memory! Allocation failed for size: " << requestedSize << " bytes\n";
    return nullptr;
}

void FreeListAllocator::deallocate(void* ptr) {
    if (!ptr) return;

    BlockHeader* block = reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(ptr) - sizeof(BlockHeader));
    if (block->isFree) {
        std::cerr << "[FreeListAllocator] Double Free Detected on pointer " << ptr << "!\n";
        return;
    }

    block->isFree = true;
    if (usedMemory >= block->size) {
        usedMemory -= block->size;
    }

    MemoryTracker::getInstance().recordDeallocation(ptr, block->size);

    coalesce(block);
}

void FreeListAllocator::coalesce(BlockHeader* block) {
    // Coalesce with next block if free
    if (block->next && block->next->isFree) {
        block->size += sizeof(BlockHeader) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }

    // Coalesce with prev block if free
    if (block->prev && block->prev->isFree) {
        block->prev->size += sizeof(BlockHeader) + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}

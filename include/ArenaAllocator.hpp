#ifndef ARENA_ALLOCATOR_HPP
#define ARENA_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "MemoryTracker.hpp"

class ArenaAllocator {
public:
    explicit ArenaAllocator(size_t capacity);
    ~ArenaAllocator();

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));
    void deallocate(void* ptr, size_t size);
    void reset();

    size_t getCapacity() const { return totalCapacity; }
    size_t getUsedMemory() const { return currentOffset; }
    size_t getRemainingMemory() const { return totalCapacity - currentOffset; }
    const void* getBufferStart() const { return memoryBuffer; }

private:
    static uintptr_t alignForward(uintptr_t ptr, size_t alignment);

    uint8_t* memoryBuffer{nullptr};
    size_t totalCapacity{0};
    size_t currentOffset{0};
};

#endif // ARENA_ALLOCATOR_HPP

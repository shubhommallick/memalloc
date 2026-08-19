#include <iostream>
#include <cassert>
#include <vector>
#include "ArenaAllocator.hpp"
#include "PoolAllocator.hpp"
#include "FreeListAllocator.hpp"
#include "CustomAllocatorAdapter.hpp"

void testArenaAllocator() {
    std::cout << "[TEST] Running Arena Allocator Tests...\n";
    ArenaAllocator arena(1024);
    assert(arena.getRemainingMemory() == 1024);

    void* p1 = arena.allocate(128);
    assert(p1 != nullptr);

    void* p2 = arena.allocate(256);
    assert(p2 != nullptr);
    assert(arena.getUsedMemory() >= 384);

    arena.reset();
    assert(arena.getUsedMemory() == 0);
    std::cout << " -> Arena Allocator PASSED!\n\n";
}

void testPoolAllocator() {
    std::cout << "[TEST] Running Pool Allocator Tests...\n";
    PoolAllocator pool(64, 10);
    assert(pool.getFreeBlocks() == 10);

    void* p1 = pool.allocate();
    void* p2 = pool.allocate();
    assert(p1 != nullptr && p2 != nullptr);
    assert(pool.getUsedBlocks() == 2);

    pool.deallocate(p1);
    assert(pool.getUsedBlocks() == 1);

    pool.deallocate(p2);
    assert(pool.getUsedBlocks() == 0);
    std::cout << " -> Pool Allocator PASSED!\n\n";
}

void testFreeListAllocator() {
    std::cout << "[TEST] Running FreeList Allocator Tests...\n";
    FreeListAllocator freeList(2048);

    void* p1 = freeList.allocate(100);
    void* p2 = freeList.allocate(200);
    assert(p1 != nullptr && p2 != nullptr);

    freeList.deallocate(p1);
    freeList.deallocate(p2);
    std::cout << " -> FreeList Allocator PASSED!\n\n";
}

void testSTLAdapter() {
    std::cout << "[TEST] Running STL Container Custom Allocator Adapter Tests...\n";
    ArenaAllocator arena(4096);
    CustomAllocatorAdapter<int, ArenaAllocator> adapter(&arena);

    std::vector<int, CustomAllocatorAdapter<int, ArenaAllocator>> vec(adapter);
    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
    }
    assert(vec.size() == 100);
    assert(arena.getUsedMemory() > 0);
    std::cout << " -> STL Adapter PASSED!\n\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "    RUNNING ALLOCATOR UNIT TEST SUITE      \n";
    std::cout << "===========================================\n\n";

    testArenaAllocator();
    testPoolAllocator();
    testFreeListAllocator();
    testSTLAdapter();

    MemoryTracker::getInstance().printReport();
    std::cout << "ALL ALLOCATOR TESTS PASSED SUCCESSFULLY!\n";
    return 0;
}

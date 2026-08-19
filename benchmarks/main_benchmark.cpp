#include <iostream>
#include <chrono>
#include <vector>
#include "ArenaAllocator.hpp"
#include "PoolAllocator.hpp"

constexpr size_t NUM_OPERATIONS = 100000;
constexpr size_t BLOCK_SIZE = 64;

void benchmarkMallocFree() {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<void*> ptrs;
    ptrs.reserve(NUM_OPERATIONS);

    for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
        ptrs.push_back(std::malloc(BLOCK_SIZE));
    }
    for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
        std::free(ptrs[i]);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << " [BENCHMARK] Standard malloc/free: " << duration << " us (" << (duration / 1000.0) << " ms)\n";
}

void benchmarkArenaAllocator() {
    ArenaAllocator arena(NUM_OPERATIONS * (BLOCK_SIZE + 16));

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
        arena.allocate(BLOCK_SIZE);
    }
    arena.reset(); // Bulk fast reset

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << " [BENCHMARK] ArenaAllocator (Bump): " << duration << " us (" << (duration / 1000.0) << " ms)\n";
}

void benchmarkPoolAllocator() {
    PoolAllocator pool(BLOCK_SIZE, NUM_OPERATIONS);
    std::vector<void*> ptrs;
    ptrs.reserve(NUM_OPERATIONS);

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
        ptrs.push_back(pool.allocate());
    }
    for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
        pool.deallocate(ptrs[i]);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << " [BENCHMARK] PoolAllocator (Fixed Block): " << duration << " us (" << (duration / 1000.0) << " ms)\n";
}

int main() {
    std::cout << "=======================================================\n";
    std::cout << "    PERFORMANCE BENCHMARK: 100,000 ALLOCATIONS         \n";
    std::cout << "=======================================================\n\n";

    benchmarkMallocFree();
    benchmarkArenaAllocator();
    benchmarkPoolAllocator();

    std::cout << "\n=======================================================\n";
    return 0;
}

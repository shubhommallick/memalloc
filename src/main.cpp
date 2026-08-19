#include <iostream>
#include <cstdlib>
#include "ArenaAllocator.hpp"
#include "PoolAllocator.hpp"
#include "FreeListAllocator.hpp"
#include "MemoryVisualizerServer.hpp"
#include "UniquePtr.hpp"
#include "SharedPtr.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main() {
    std::cout << "=======================================================\n";
    std::cout << "    C++ CUSTOM MEMORY ALLOCATOR & VISUALIZER SUITE     \n";
    std::cout << "=======================================================\n\n";

    // Read port from environment (Render compatibility) or default to 8080
    int port = 8080;
    const char* envPort = std::getenv("PORT");
    if (envPort) {
        port = std::atoi(envPort);
    }

    // Initialize allocators
    ArenaAllocator arena(4096);       // 4KB Arena
    PoolAllocator pool(64, 32);       // 32 blocks of 64B
    FreeListAllocator freeList(4096); // 4KB Free-List

    // Perform initial demo allocations
    std::cout << "[SYSTEM] Initializing sample allocations across engines...\n";
    void* a1 = arena.allocate(128);
    void* a2 = arena.allocate(256);

    void* p1 = pool.allocate();
    void* p2 = pool.allocate();
    void* p3 = pool.allocate();

    void* f1 = freeList.allocate(128);
    void* f2 = freeList.allocate(512);

    (void)a1; (void)a2; (void)p1; (void)p2; (void)p3; (void)f1; (void)f2;

    // Start Visualizer Server
    MemoryVisualizerServer server(port, arena, pool, freeList);
    server.start();

    std::cout << "[SYSTEM] Visualizer server is running continuously on port " << port << ".\n";

    // Keep server process running continuously
    while (true) {
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
    }

    return 0;
}

#ifndef MEMORY_VISUALIZER_SERVER_HPP
#define MEMORY_VISUALIZER_SERVER_HPP

#include <string>
#include <atomic>
#include <vector>
#include "ArenaAllocator.hpp"
#include "PoolAllocator.hpp"
#include "FreeListAllocator.hpp"

class MemoryVisualizerServer {
public:
    MemoryVisualizerServer(int port, ArenaAllocator& arena, PoolAllocator& pool, FreeListAllocator& freeList);
    ~MemoryVisualizerServer();

    void start();
    void stop();
    void runServerLoop();

private:
    std::string handleRequest(const std::string& request);
    std::string buildJsonStats();
    std::string readFile(const std::string& filepath);

    int serverPort;
    std::atomic<bool> isRunning{false};
    void* threadHandle{nullptr};

    ArenaAllocator& arenaRef;
    PoolAllocator& poolRef;
    FreeListAllocator& freeListRef;

    std::vector<void*> uiArenaPtrs;
    std::vector<void*> uiPoolPtrs;
    std::vector<void*> uiFreeListPtrs;
};

#endif // MEMORY_VISUALIZER_SERVER_HPP

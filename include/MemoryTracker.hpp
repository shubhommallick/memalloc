#ifndef MEMORY_TRACKER_HPP
#define MEMORY_TRACKER_HPP

#include <cstddef>
#include <iostream>
#include <atomic>
#include <string>
#include <vector>

struct AllocationSnapshot {
    void* address;
    size_t size;
    std::string allocatorType; // "Arena", "Pool", "FreeList"
    bool active;
};

class MemoryTracker {
public:
    static MemoryTracker& getInstance();

    void recordAllocation(void* ptr, size_t bytes, const std::string& allocatorType);
    void recordDeallocation(void* ptr, size_t bytes);
    void reset();

    size_t getTotalAllocated() const;
    size_t getActiveAllocations() const;
    size_t getPeakMemory() const;
    size_t getAllocationCount() const;

    std::vector<AllocationSnapshot> getSnapshots() const;
    void printReport() const;

private:
    MemoryTracker() = default;
    ~MemoryTracker() = default;

    MemoryTracker(const MemoryTracker&) = delete;
    MemoryTracker& operator=(const MemoryTracker&) = delete;

    void lock() const;
    void unlock() const;

    mutable std::atomic_flag lockFlag = ATOMIC_FLAG_INIT;
    size_t totalAllocated{0};
    size_t currentActive{0};
    size_t peakMemory{0};
    size_t allocationCount{0};

    std::vector<AllocationSnapshot> snapshots;
};

#endif // MEMORY_TRACKER_HPP

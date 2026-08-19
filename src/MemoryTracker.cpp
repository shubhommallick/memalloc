#include "MemoryTracker.hpp"
#include <iomanip>
#include <algorithm>

MemoryTracker& MemoryTracker::getInstance() {
    static MemoryTracker instance;
    return instance;
}

void MemoryTracker::lock() const {
    while (lockFlag.test_and_set(std::memory_order_acquire)) {
        // Spinlock lock
    }
}

void MemoryTracker::unlock() const {
    lockFlag.clear(std::memory_order_release);
}

void MemoryTracker::recordAllocation(void* ptr, size_t bytes, const std::string& allocatorType) {
    lock();
    totalAllocated += bytes;
    currentActive += bytes;
    if (currentActive > peakMemory) {
        peakMemory = currentActive;
    }
    allocationCount++;

    snapshots.push_back({ptr, bytes, allocatorType, true});
    unlock();
}

void MemoryTracker::recordDeallocation(void* ptr, size_t bytes) {
    lock();
    if (currentActive >= bytes) {
        currentActive -= bytes;
    } else {
        currentActive = 0;
    }

    for (auto& snap : snapshots) {
        if (snap.address == ptr && snap.active) {
            snap.active = false;
            break;
        }
    }
    unlock();
}

void MemoryTracker::reset() {
    lock();
    totalAllocated = 0;
    currentActive = 0;
    peakMemory = 0;
    allocationCount = 0;
    snapshots.clear();
    unlock();
}

size_t MemoryTracker::getTotalAllocated() const {
    lock();
    size_t val = totalAllocated;
    unlock();
    return val;
}

size_t MemoryTracker::getActiveAllocations() const {
    lock();
    size_t val = currentActive;
    unlock();
    return val;
}

size_t MemoryTracker::getPeakMemory() const {
    lock();
    size_t val = peakMemory;
    unlock();
    return val;
}

size_t MemoryTracker::getAllocationCount() const {
    lock();
    size_t val = allocationCount;
    unlock();
    return val;
}

std::vector<AllocationSnapshot> MemoryTracker::getSnapshots() const {
    lock();
    std::vector<AllocationSnapshot> val = snapshots;
    unlock();
    return val;
}

void MemoryTracker::printReport() const {
    lock();
    std::cout << "\n===========================================\n";
    std::cout << "        MEMORY TRACKER REPORT              \n";
    std::cout << "===========================================\n";
    std::cout << " Total Bytes Allocated: " << totalAllocated << " bytes\n";
    std::cout << " Current Active Bytes:  " << currentActive << " bytes\n";
    std::cout << " Peak Memory Usage:     " << peakMemory << " bytes\n";
    std::cout << " Total Allocations:     " << allocationCount << "\n";
    
    if (currentActive > 0) {
        std::cout << " [WARNING] Memory Leaks Detected! Active bytes: " << currentActive << " bytes\n";
    } else {
        std::cout << " [SUCCESS] Zero Leaks! All allocated memory freed.\n";
    }
    std::cout << "===========================================\n\n";
    unlock();
}

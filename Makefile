CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS = -lws2_32

SRC = src/MemoryTracker.cpp src/ArenaAllocator.cpp src/PoolAllocator.cpp src/FreeListAllocator.cpp src/MemoryVisualizerServer.cpp

all: main.exe test_allocators.exe test_smart_pointers.exe main_benchmark.exe

main.exe: src/main.cpp $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

test_allocators.exe: tests/test_allocators.cpp src/MemoryTracker.cpp src/ArenaAllocator.cpp src/PoolAllocator.cpp src/FreeListAllocator.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

test_smart_pointers.exe: tests/test_smart_pointers.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

main_benchmark.exe: benchmarks/main_benchmark.cpp src/MemoryTracker.cpp src/ArenaAllocator.cpp src/PoolAllocator.cpp src/FreeListAllocator.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

run: main.exe
	./main.exe

test: test_allocators.exe test_smart_pointers.exe
	./test_allocators.exe
	./test_smart_pointers.exe

benchmark: main_benchmark.exe
	./main_benchmark.exe

clean:
	del /f /q *.exe

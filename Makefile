CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32
    TARGET = main.exe
    TEST_ALLOC = test_allocators.exe
    TEST_SMART = test_smart_pointers.exe
    BENCHMARK = main_benchmark.exe
    RM_CMD = del /f /q *.exe
else
    LDFLAGS = -pthread
    TARGET = main
    TEST_ALLOC = test_allocators
    TEST_SMART = test_smart_pointers
    BENCHMARK = main_benchmark
    RM_CMD = rm -f main test_allocators test_smart_pointers main_benchmark *.exe
endif

SRC = src/MemoryTracker.cpp src/ArenaAllocator.cpp src/PoolAllocator.cpp src/FreeListAllocator.cpp src/MemoryVisualizerServer.cpp

all: $(TARGET) $(TEST_ALLOC) $(TEST_SMART) $(BENCHMARK)

$(TARGET): src/main.cpp $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_ALLOC): tests/test_allocators.cpp src/MemoryTracker.cpp src/ArenaAllocator.cpp src/PoolAllocator.cpp src/FreeListAllocator.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_SMART): tests/test_smart_pointers.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BENCHMARK): benchmarks/main_benchmark.cpp src/MemoryTracker.cpp src/ArenaAllocator.cpp src/PoolAllocator.cpp src/FreeListAllocator.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_ALLOC) $(TEST_SMART)
	./$(TEST_ALLOC)
	./$(TEST_SMART)

benchmark: $(BENCHMARK)
	./$(BENCHMARK)

clean:
	$(RM_CMD)

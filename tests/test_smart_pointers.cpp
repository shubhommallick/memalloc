#include <iostream>
#include <cassert>
#include <string>
#include "UniquePtr.hpp"
#include "SharedPtr.hpp"

struct DummyObject {
    std::string name;
    static int instances;

    DummyObject(const std::string& n) : name(n) { instances++; }
    ~DummyObject() { instances--; }
};

int DummyObject::instances = 0;

void testUniquePtr() {
    std::cout << "[TEST] Running UniquePtr Tests...\n";
    {
        UniquePtr<DummyObject> u1 = makeUnique<DummyObject>("TestObj");
        assert(u1.get() != nullptr);
        assert(u1->name == "TestObj");
        assert(DummyObject::instances == 1);

        // Ownership transfer via Move
        UniquePtr<DummyObject> u2 = std::move(u1);
        assert(u1.get() == nullptr);
        assert(u2.get() != nullptr);
        assert(DummyObject::instances == 1);
    }
    assert(DummyObject::instances == 0);
    std::cout << " -> UniquePtr PASSED!\n\n";
}

void testSharedPtr() {
    std::cout << "[TEST] Running SharedPtr Tests...\n";
    {
        SharedPtr<DummyObject> s1 = makeShared<DummyObject>("SharedResource");
        assert(s1.useCount() == 1);
        assert(DummyObject::instances == 1);

        {
            SharedPtr<DummyObject> s2 = s1; // Copy increases ref count
            assert(s1.useCount() == 2);
            assert(s2.useCount() == 2);
            assert(DummyObject::instances == 1);
        } // s2 goes out of scope

        assert(s1.useCount() == 1);
        assert(DummyObject::instances == 1);
    } // s1 goes out of scope -> auto deleted
    assert(DummyObject::instances == 0);
    std::cout << " -> SharedPtr PASSED!\n\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "   RUNNING SMART POINTER UNIT TEST SUITE   \n";
    std::cout << "===========================================\n\n";

    testUniquePtr();
    testSharedPtr();

    std::cout << "ALL SMART POINTER TESTS PASSED SUCCESSFULLY!\n";
    return 0;
}

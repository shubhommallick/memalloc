#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include <utility>
#include <cstddef>
#include <atomic>

template <typename T>
class SharedPtr {
private:
    struct ControlBlock {
        std::atomic<size_t> refCount{1};
    };

public:
    constexpr SharedPtr() noexcept : ptr(nullptr), controlBlock(nullptr) {}
    constexpr SharedPtr(std::nullptr_t) noexcept : ptr(nullptr), controlBlock(nullptr) {}

    explicit SharedPtr(T* p) : ptr(p) {
        if (ptr) {
            controlBlock = new ControlBlock();
        }
    }

    ~SharedPtr() {
        release();
    }

    // Copy Semantics
    SharedPtr(const SharedPtr& other) noexcept : ptr(other.ptr), controlBlock(other.controlBlock) {
        if (controlBlock) {
            controlBlock->refCount++;
        }
    }

    SharedPtr& operator=(const SharedPtr& other) noexcept {
        if (this != &other) {
            release();
            ptr = other.ptr;
            controlBlock = other.controlBlock;
            if (controlBlock) {
                controlBlock->refCount++;
            }
        }
        return *this;
    }

    // Move Semantics
    SharedPtr(SharedPtr&& other) noexcept : ptr(other.ptr), controlBlock(other.controlBlock) {
        other.ptr = nullptr;
        other.controlBlock = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            release();
            ptr = other.ptr;
            controlBlock = other.controlBlock;
            other.ptr = nullptr;
            other.controlBlock = nullptr;
        }
        return *this;
    }

    size_t useCount() const noexcept {
        return controlBlock ? controlBlock->refCount.load() : 0;
    }

    T* get() const noexcept { return ptr; }
    T& operator*() const { return *ptr; }
    T* operator->() const noexcept { return ptr; }
    explicit operator bool() const noexcept { return ptr != nullptr; }

    void reset(T* p = nullptr) {
        release();
        if (p) {
            ptr = p;
            controlBlock = new ControlBlock();
        } else {
            ptr = nullptr;
            controlBlock = nullptr;
        }
    }

private:
    void release() {
        if (controlBlock) {
            if (--controlBlock->refCount == 0) {
                delete ptr;
                delete controlBlock;
            }
            ptr = nullptr;
            controlBlock = nullptr;
        }
    }

    T* ptr{nullptr};
    ControlBlock* controlBlock{nullptr};
};

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
    return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

#endif // SHARED_PTR_HPP

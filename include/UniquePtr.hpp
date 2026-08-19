#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP

#include <utility>
#include <cstddef>

template <typename T>
struct DefaultDeleter {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    constexpr UniquePtr() noexcept : ptr(nullptr) {}
    constexpr UniquePtr(std::nullptr_t) noexcept : ptr(nullptr) {}
    explicit UniquePtr(T* p) noexcept : ptr(p) {}

    ~UniquePtr() {
        reset();
    }

    // Move-only semantics
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) noexcept : ptr(other.ptr), deleter(std::move(other.deleter)) {
        other.ptr = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            reset();
            ptr = other.ptr;
            deleter = std::move(other.deleter);
            other.ptr = nullptr;
        }
        return *this;
    }

    T* get() const noexcept { return ptr; }
    T& operator*() const { return *ptr; }
    T* operator->() const noexcept { return ptr; }
    explicit operator bool() const noexcept { return ptr != nullptr; }

    T* release() noexcept {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    void reset(T* p = nullptr) noexcept {
        T* old = ptr;
        ptr = p;
        if (old) {
            deleter(old);
        }
    }

private:
    T* ptr{nullptr};
    Deleter deleter{};
};

template <typename T, typename... Args>
UniquePtr<T> makeUnique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

#endif // UNIQUE_PTR_HPP

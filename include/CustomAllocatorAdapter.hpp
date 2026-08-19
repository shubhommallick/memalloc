#ifndef CUSTOM_ALLOCATOR_ADAPTER_HPP
#define CUSTOM_ALLOCATOR_ADAPTER_HPP

#include <cstddef>
#include <memory>
#include <utility>

template <typename T, typename AllocatorEngine>
class CustomAllocatorAdapter {
public:
    using value_type = T;

    explicit CustomAllocatorAdapter(AllocatorEngine* engine = nullptr) : engineRef(engine) {}

    template <typename U>
    CustomAllocatorAdapter(const CustomAllocatorAdapter<U, AllocatorEngine>& other) noexcept
        : engineRef(other.engineRef) {}

    T* allocate(size_t n) {
        if (!engineRef) {
            return static_cast<T*>(::operator new(n * sizeof(T)));
        }
        void* ptr = engineRef->allocate(n * sizeof(T));
        if (!ptr) {
            throw std::bad_alloc();
        }
        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, size_t n) noexcept {
        if (!engineRef) {
            ::operator delete(p);
            return;
        }
        engineRef->deallocate(p, n * sizeof(T));
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) {
        p->~U();
    }

    AllocatorEngine* engineRef{nullptr};
};

template <typename T, typename U, typename AllocatorEngine>
bool operator==(const CustomAllocatorAdapter<T, AllocatorEngine>& a, const CustomAllocatorAdapter<U, AllocatorEngine>& b) {
    return a.engineRef == b.engineRef;
}

template <typename T, typename U, typename AllocatorEngine>
bool operator!=(const CustomAllocatorAdapter<T, AllocatorEngine>& a, const CustomAllocatorAdapter<U, AllocatorEngine>& b) {
    return !(a == b);
}

#endif // CUSTOM_ALLOCATOR_ADAPTER_HPP

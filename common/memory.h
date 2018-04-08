#ifndef MEMORY_H
#define MEMORY_H

#include <cstdlib>
#include <stdexcept>
#include <memory>
#include <vector>

// C++ aligned allocation convenience functions
namespace ppc {
    // C++ memory allocator that allocates 32-byte aligned memory.
    // Useful with for example std::vector
    template <typename T>
    struct allocator {
        typedef T value_type;

        allocator() = default;

        template <typename U>
        constexpr allocator(const allocator<U>&) noexcept {}

        T* allocate(std::size_t n) {
            T* ret = nullptr;
            if (posix_memalign((void**)&ret, alignof(T), n*sizeof(T))) {
                throw std::bad_alloc();
            }
            return ret;
        }

        void deallocate(T* p, std::size_t) noexcept {
            free(p);
        }
    };

    template <typename T, typename U>
    bool operator==(const allocator<T>&, const allocator<U>&) { return true; }
    template <typename T, typename U>
    bool operator!=(const allocator<T>&, const allocator<U>&) { return false; }

    template <typename T>
    using vector = std::vector<T, allocator<T>>;

    template <typename T>    
    struct free_deleter {
        void operator()(T* p) const {
            free(const_cast<std::remove_const_t<T>*>(p));
        }
    };

    template <typename T>
    using unique_ptr = std::unique_ptr<T, free_deleter<T>>;

    template <typename T>
    unique_ptr<T> alloc(size_t count) {
        T* ptr = nullptr;
        if (posix_memalign((void**)&ptr, alignof(T), count*sizeof(T))) {
            return nullptr;
        }
        return unique_ptr<T>(ptr);
    }
}

#endif

#ifndef MEH_ALLOCATOR
#define MEH_ALLOCATOR

#include <cstring>
#include <memory>

namespace meh {

#define MEH_MAX(x, y) (x > y) ? x : y

// increment size by 1.5 times everytime when its full
#define INCREASE_SIZE(size) ((size) < 16 ? 16 : (size)*1.5)

#define MEH_ALLOCATE(type, pointer, old_size, new_size)                        \
  static_cast<type *>(                                                         \
      allocate(pointer, sizeof(type) * old_size, sizeof(type) * new_size))

#define FREE_ALLOCATION(type, pointer, old_size)                               \
  allocate(pointer, sizeof(type) * old_size, 0)

#define MEH_MEM_POOL_ADD(block, type)\
(type*) block.store(sizeof(type), alignof(type));

    static void* allocate(void* pointer, std::size_t old_size,
        std::size_t new_size) {
        if (new_size == 0) {
            std::free(pointer);
            return NULL;
        }
        void* p = std::realloc(pointer, new_size);
        if (p == NULL) {
            exit(1);
        }
        return p;
    }

    struct meh_mem_pool {
        void* memory;
        char* brk;
        std::size_t num_allocations;
        std::size_t _size;
        std::size_t used;

        meh_mem_pool(std::size_t size) {
            memory = malloc(size);
            if (!memory) {
                // output memory allocation error
            }
            brk = (char*)memory;
            used = 0;
            num_allocations = 0;
            _size = size;
        }

        void* store(std::size_t size, std::size_t alignment) {
            if (used + 1 > _size) {
                std::size_t i = size;
                _size = (size_t)INCREASE_SIZE(i);
                memory = allocate(memory, i, _size);
            }

            std::size_t b = (uintptr_t)brk;
            if (!(b % alignment == 0)) {
                brk += alignment - (b % alignment);
            }

            void* ptr = brk;
            num_allocations++;
            brk += size;
            used += size;
            return ptr;
        }

    };


}





#endif

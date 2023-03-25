#include <iostream>
#include "MemoryPool/MemoryPool.h"

int main() {
    std::size_t  chunk_size = 100;
    std::size_t pool_size = 10;
    MemoryPool<unsigned char> memoryPool(100, 100);

    auto *data = memoryPool.allocate();

    return 0;
}

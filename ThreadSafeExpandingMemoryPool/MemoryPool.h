//
// Created by corey on 3/25/23.
//

#ifndef MEMORYPOOLTEST_MEMORYPOOL_H
#define MEMORYPOOLTEST_MEMORYPOOL_H

#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <memory>
#include <mutex>

template<typename T>
class MemoryPool {
public:
    MemoryPool(std::size_t chunkSize, std::size_t poolSize)
            : m_chunkSize{chunkSize}
            , m_poolSize{poolSize}
            , m_memory{std::make_unique<std::uint8_t[]>(m_chunkSize * m_poolSize)}
            , m_freeList{nullptr}
    {}

    ~MemoryPool() {
        clear();
    }

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    MemoryPool(MemoryPool&& other) noexcept
            : m_chunkSize{other.m_chunkSize}
            , m_poolSize{other.m_poolSize}
            , m_memory{std::move(other.m_memory)}
            , m_freeList{other.m_freeList}
    {
        other.m_freeList = nullptr;
    }

    MemoryPool& operator=(MemoryPool&& other) noexcept {
        if (this != &other) {
            m_chunkSize = other.m_chunkSize;
            m_poolSize = other.m_poolSize;
            m_memory = std::move(other.m_memory);
            m_freeList = other.m_freeList;
            other.m_freeList = nullptr;
        }
        return *this;
    }

    T* allocate() {
        std::unique_lock<std::mutex> lock{m_mutex};
        if (m_freeList == nullptr) {
            // Allocate additional memory
            std::uint8_t* newMemory = nullptr;
            try {
                newMemory = reinterpret_cast<std::uint8_t*>(std::malloc(m_chunkSize * m_poolSize));
            } catch (...) {
                throw std::bad_alloc{};
            }
            // Initialize the new memory as a free list
            for (std::size_t i = 0; i < m_poolSize; ++i) {
                std::uint8_t* chunk = newMemory + (i * m_chunkSize);
                *reinterpret_cast<std::uint8_t**>(chunk) = m_freeList;
                m_freeList = chunk;
            }
            // Release the old memory
            m_memory.reset(newMemory);
        }
        std::uint8_t* chunk = m_freeList;
        m_freeList = *reinterpret_cast<std::uint8_t**>(chunk);
        lock.unlock();
        return reinterpret_cast<T*>(chunk);
    }

    void deallocate(T* ptr) {
        std::unique_lock<std::mutex> lock{m_mutex};
        std::uint8_t* chunk = reinterpret_cast<std::uint8_t*>(ptr);
        *reinterpret_cast<std::uint8_t**>(chunk) = m_freeList;
        m_freeList = chunk;
    }

    void clear() {
        std::unique_lock<std::mutex> lock{m_mutex};
        m_memory.reset();
        m_freeList = nullptr;
    }

private:
    std::size_t m_chunkSize;
    std::size_t m_poolSize;
    std::unique_ptr<std::uint8_t[]> m_memory;
    std::uint8_t* m_freeList;
    std::mutex m_mutex;
};


#endif //MEMORYPOOLTEST_MEMORYPOOL_H

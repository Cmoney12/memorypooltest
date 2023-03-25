#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <memory>

template<typename T>
class MemoryPool {
public:
    MemoryPool(std::size_t chunkSize, std::size_t poolSize)
            : m_chunkSize{chunkSize}
            , m_poolSize{poolSize}
            , m_memory{std::make_unique<std::uint8_t[]>(m_chunkSize * m_poolSize)}
            , m_freeList{nullptr}
    {
        for (std::size_t i = 0; i < m_poolSize; ++i) {
            std::uint8_t* chunk = m_memory.get() + (i * m_chunkSize);
            *reinterpret_cast<std::uint8_t**>(chunk) = m_freeList;
            m_freeList = chunk;
        }
    }

    ~MemoryPool() { clear(); }

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
        if (m_freeList == nullptr) {
            throw std::bad_alloc{};
        }
        std::uint8_t* chunk = m_freeList;
        m_freeList = *reinterpret_cast<std::uint8_t**>(chunk);
        return reinterpret_cast<T*>(chunk);
    }

    void deallocate(T* ptr) {
        auto* chunk = reinterpret_cast<std::uint8_t*>(ptr);
        *reinterpret_cast<std::uint8_t**>(chunk) = m_freeList;
        m_freeList = chunk;
    }

    void clear() {
        m_memory.reset();
        m_freeList = nullptr;
    }

private:
    std::size_t m_chunkSize;
    std::size_t m_poolSize;
    std::unique_ptr<std::uint8_t[]> m_memory;
    std::uint8_t* m_freeList;
};
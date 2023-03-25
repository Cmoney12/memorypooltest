cmake_minimum_required(VERSION 3.24)
project(memorypooltest)

set(CMAKE_CXX_STANDARD 17)

add_executable(memorypooltest main.cpp MemoryPool/MemoryPool.h ExpandingMemoryPool/MemoryPool.h ThreadSafeExpandingMemoryPool/MemoryPool.h)

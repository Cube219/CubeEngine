cmake_minimum_required(VERSION 3.19)

include(FetchContent)

FetchContent_Declare(
    D3D12MemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator
    GIT_TAG        6a61103936dfadedca584e1a521fab6943d3b5ad # v2.0.1
)
FetchContent_MakeAvailable(D3D12MemoryAllocator)

target_include_directories(D3D12MemoryAllocator
    PUBLIC
        ${D3D12MemoryAllocator_SOURCE_DIR}/include
)

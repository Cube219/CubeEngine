cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching D3D12MemoryAllocator...")

FetchContent_Declare(
    D3D12MemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator
    GIT_TAG        0fa62ed3a0a69b73230a8ec1faa752d4061c8dc8 # v3.1.0
)
FetchContent_MakeAvailable(D3D12MemoryAllocator)

target_link_libraries(D3D12MemoryAllocator
    PRIVATE
        dxagility
)

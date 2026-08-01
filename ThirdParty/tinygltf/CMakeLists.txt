cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching tinygltf...")

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf
    GIT_TAG        b956fa3e9deeb7ee7f0f22c08bda9b3a684a98dc # v2.9.5
)
FetchContent_MakeAvailable(tinygltf)

target_compile_definitions(tinygltf
    PRIVATE
        TINYGLTF_USE_CPP14
)

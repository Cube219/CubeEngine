cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching tinygltf...")

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf
    GIT_TAG        a5e653e46cca64bbc0ecee7ede007347a94c39b0 # v2.9.5
)
FetchContent_MakeAvailable(tinygltf)

target_compile_definitions(tinygltf
    PRIVATE
        TINYGLTF_USE_CPP14
)

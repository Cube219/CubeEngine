cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching tinyobjloader...")

FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader
    GIT_TAG        6ac677b958a146b956635c95b40ac05d2502f564 # v2.0.0rc11
)
FetchContent_MakeAvailable(tinyobjloader)

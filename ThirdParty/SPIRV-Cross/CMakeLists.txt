cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching SPIRV-Cross...")

FetchContent_Declare(
    spirvcross
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross
    GIT_TAG        d8e3e2b141b8c8a167b2e3984736a6baacff316c # vulkan-sdk-1.4.321.0
)
FetchContent_MakeAvailable(spirvcross)

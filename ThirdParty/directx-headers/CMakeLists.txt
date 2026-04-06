cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching directx-headers...")

FetchContent_Declare(
    directx-headers
    GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers
    GIT_TAG        9e393d6d8a3b30dcc6f2806ef604ec16a27b0d7e # v1.619.1
)
FetchContent_MakeAvailable(directx-headers)

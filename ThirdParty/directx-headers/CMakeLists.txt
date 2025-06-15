cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching directx-headers...")

FetchContent_Declare(
    directx-headers
    GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers
    GIT_TAG        c94b9b23aaadc2034dd1cad656a5a69f1526f98a # v1.616.0
)
FetchContent_MakeAvailable(directx-headers)

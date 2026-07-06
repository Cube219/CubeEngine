cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching fmt...")

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        123913715afeb8a437e6388b4473fcc4753e1c9a # 11.1.4
)
FetchContent_MakeAvailable(fmt)

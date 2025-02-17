cmake_minimum_required(VERSION 3.19)

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        11.0.2)
FetchContent_MakeAvailable(fmt)

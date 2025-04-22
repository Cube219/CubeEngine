cmake_minimum_required(VERSION 3.19)

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        0c9fce2ffefecfdce794e1859584e25877b7b592 # 11.0.2
)
FetchContent_MakeAvailable(fmt)

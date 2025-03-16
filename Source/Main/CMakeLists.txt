cmake_minimum_required(VERSION 3.19)

# Target settings
add_executable(CE-Main WIN32 main.cpp)
target_link_libraries(CE-Main
    PRIVATE
        CE-Base
        CE-GAPI
        CE-Core
)

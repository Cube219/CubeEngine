cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/Allocator.h
    Public/CubeString.h
    Public/Event.h
    Public/Flags.h
    Public/Format.h
    Public/KeyCode.h
    Public/Mouse.h
    Public/Types.h
)

set(PRIVATE_FILES
    Private/CubeString.cpp
    Private/CubeFormat.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES})

# Target settings
add_library(CE-Base STATIC
    ${PUBLIC_FILES} ${PRIVATE_FILES}
)
target_include_directories(CE-Base
    PUBLIC
        Public
    PRIVATE
        Private
)

# Third-party
target_link_libraries(CE-Base
    fmt::fmt
)

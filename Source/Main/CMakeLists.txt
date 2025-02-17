cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/Allocator/FrameAllocator.h
    Public/Engine.h
    Public/MainHeader.h
)
set(PRIVATE_FILES
    Private/Allocator/AllocatorUtility.h
    Private/Allocator/FrameAllocator.cpp
    Private/Renderer/Renderer.cpp
    Private/Renderer/Renderer.h
    Private/Engine.cpp
    main.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES})

target_include_directories(CE-GAPI
    PUBLIC
        Public
    PRIVATE
        Private
)

# Target settings
add_executable(CE-Main WIN32 ${PUBLIC_FILES} ${PRIVATE_FILES})
target_link_libraries(CE-Main
    PRIVATE
        imgui
        CE-Base
        CE-Logger
        CE-Platform
        CE-GAPI
)

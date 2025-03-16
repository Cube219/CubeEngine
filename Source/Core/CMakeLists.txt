cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/Allocator/FrameAllocator.h
    Public/CoreHeader.h
    Public/Engine.h
)
set(PRIVATE_FILES
    Private/Allocator/AllocatorUtility.h
    Private/Allocator/FrameAllocator.cpp
    Private/Renderer/Renderer.cpp
    Private/Renderer/Renderer.h
    Private/Engine.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES})

# Target settings
add_library(CE-Core SHARED
    ${PUBLIC_FILES} ${PRIVATE_FILES}
)
target_include_directories(CE-Core
    PUBLIC
        Public
    PRIVATE
        Private
)
target_link_libraries(CE-Core
    PRIVATE
        imgui
        CE-Base
        CE-Logger
        CE-Platform
        CE-GAPI
)
target_compile_definitions(CE-Core
    PRIVATE
        CUBE_MODULE_CORE
)

cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/Allocator/AllocatorUtility.h
    Public/Allocator/FrameAllocator.h
    Public/Renderer/RenderTypes.h
    Public/CoreHeader.h
    Public/Engine.h
)
set(PRIVATE_FILES
    Private/Allocator/FrameAllocator.cpp
    Private/Renderer/BaseMeshGenerator.cpp
    Private/Renderer/BaseMeshGenerator.h
    Private/Renderer/Material.cpp
    Private/Renderer/Material.h
    Private/Renderer/Mesh.cpp
    Private/Renderer/Mesh.h
    Private/Renderer/Renderer.cpp
    Private/Renderer/Renderer.h
    Private/Renderer/RenderObject.cpp
    Private/Renderer/RenderObject.h
    Private/Renderer/SamplerManager.cpp
    Private/Renderer/SamplerManager.h
    Private/Renderer/Shader.cpp
    Private/Renderer/Shader.h
    Private/Renderer/Texture.cpp
    Private/Renderer/Texture.h
    Private/Renderer/TextureManager.cpp
    Private/Renderer/TextureManager.h
    Private/Systems/CameraSystem.cpp
    Private/Systems/CameraSystem.h
    Private/Systems/ModelLoaderSystem.cpp
    Private/Systems/ModelLoaderSystem.h
    Private/Systems/StatsSystem.cpp
    Private/Systems/StatsSystem.h
    Private/Engine.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES})

# Target settings
add_library(CE-Core SHARED
    ${GLOBAL_FILES}
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
        tinygltf
        tinyobjloader
        CE-Base
        CE-Logger
        CE-Platform
        CE-GAPI
)
target_compile_definitions(CE-Core
    PRIVATE
        CUBE_MODULE_CORE
)

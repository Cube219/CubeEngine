cmake_minimum_required (VERSION 3.6)

set(INCLUDE
    Allocator/AllocatorUtility.h
    Allocator/FrameAllocator.h
    Module/Module.h
    Module/ModuleManager.h
    Renderer/BaseMeshGenerator.h
    Renderer/Material.h
    Renderer/Mesh.h
    Renderer/RenderableComponent.h
    Renderer/RendererManager.h
    Renderer/RenderingThread.h
    Renderer/RenderObject.h
    Renderer/Shader.h
    Renderer/Texture.h
    Resource/Resource.h
    Resource/ResourceImporter.h
    Resource/ResourceManager.h
    Thread/Async.h
    Thread/MutexLock.h
    Thread/TaskQueue.h
    Time/GameTime.h
    Time/TimeManager.h
    World/World.h
    World/WorldObject.h
    Assertion.h
    Core.h
    CoreFwd.h
    CoreHeader.h
    GameThread.h
    Handler.h
    LogWriter.h
)
set(SOURCE
    Allocator/FrameAllocator.cpp
    Module/Module.cpp
    Module/ModuleManager.cpp
    Renderer/BaseMeshGenerator.cpp
    Renderer/Material.cpp
    Renderer/Mesh.cpp
    Renderer/RenderableComponent.cpp
    Renderer/RendererManager.cpp
    Renderer/RenderingThread.cpp
    Renderer/RenderObject.cpp
    Renderer/Shader.h
    Renderer/Texture.cpp
    Resource/Resource.cpp
    Resource/ResourceManager.cpp
    Time/GameTime.cpp
    Time/TimeManager.cpp
    World/World.cpp
    World/WorldObject.cpp
    Core.cpp
    GameThread.cpp
    LogWriter.cpp
)

# Target
add_library(CE-Core SHARED ${INCLUDE} ${SOURCE})

# Defines
target_compile_definitions(CE-Core PRIVATE CORE_EXPORTS)

# Internal libs
target_link_libraries(CE-Core PUBLIC CE-Utility CE-Platform)

# External includes
target_include_directories(CE-Core PUBLIC ${DEPENDENCIES_DIR}/entt/src)

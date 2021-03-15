#pragma once

namespace cube
{
    template <typename T>
    class Handler;

    // Allocator/FrameAllocator.h
    class FrameAllocator;

    // Module/Module.h
    struct ModuleInfo;
    class Module;
    // Module/ModuleManager.h
    struct ModuleNode;
    class ModuleManager;

    // World/World.h
    class World;
    // World/WorldObject.h
    struct Transform;
    class WorldObject;
    using HWorldObject = Handler<WorldObject>;

    // Resource/Resource.h
    class Resource;
    template <typename T>
    class ResourcePointer;
    // Resource/ResourceImporter.h
    class ResourceImporter;
    // Resource/ResourceManager.h
    class ResourceManager;

    // Renderer/RenderableComponent.h
    struct RenderableComponent;
    // Renderer/RendererManager.h
    class RendererManager;
    // Renderer/RenderingThread.h
    class RenderingThread;
    // Renderer/RenderObject.h
    class RenderObject;
    // Renderer/Texture.h
    class TextureData;
    class Texture;

    namespace rt
    {
        class RenderObject;
        class Texture;
    } // namespace rt

    // Thread/Async.h
    class AsyncSignal;
    class Async;
    // Thread/TaskQueue.h
    class TaskQueue;

    // Time/TimeManager.h
    class TimeManager;
    // Time/GameTime.h
    class GameTime;

    // Core.h
    class Core;

    // GameThread.h
    class GameThread;

    // Handler.h
    class Handlable;
    class HandlerTable;
} // namespace cube

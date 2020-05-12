#pragma once

namespace cube
{
    // Allocator/FrameAllocator.h
    class FrameAllocator;

    // Module/Module.h
    struct ModuleInfo;
    class Module;
    // Module/ModuleManager.h
    struct ModuleNode;
    class ModuleManager;

    // Resource/Resource.h
    class Resource;
    template <typename T>
    class ResourcePointer;
    // Resource/ResourceImporter.h
    class ResourceImporter;
    // Resource/ResourceManager.h
    class ResourceManager;

    // Renderer/RenderingThread.h
    class RenderingThread;

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
    template <typename T>
    class Handler;
    class Handlable;
    class HandlerTable;
} // namespace cube

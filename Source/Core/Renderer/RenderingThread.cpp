#include "RenderingThread.h"

#include "Platform/Platform.h"
#include "../GameThread.h"
#include "../Allocator/FrameAllocator.h"
#include "RendererManager.h"
#include "../Assertion.h"

namespace cube
{
    std::thread::id RenderingThread::mThreadId;

    EventFunction<void()> RenderingThread::mLoopEventFunc;
    EventFunction<void(Uint32, Uint32)> RenderingThread::mResizeEventFunc;

    Mutex RenderingThread::mSyncTaskQueueMutex;
    TaskQueue RenderingThread::mSyncTaskQueue;

    Mutex RenderingThread::mTaskQueueMutex;
    TaskQueue RenderingThread::mTaskQueue;
    TaskQueue RenderingThread::mLastTaskQueue;

    void RenderingThread::Init()
    {
        mThreadId = std::this_thread::get_id();
    }

    void RenderingThread::Prepare()
    {
        GetFrameAllocator().Initialize("Rendering Thread FrameAllocator", 10 * 1024 * 1024); // 10 MiB

        RendererManager::Initialize(RenderAPIType::Vulkan);

        mLoopEventFunc = platform::Platform::GetLoopEvent().AddListener(&RenderingThread::Loop);
        mResizeEventFunc = platform::Platform::GetResizeEvent().AddListener(&RenderingThread::OnResize);
    }

    void RenderingThread::Run()
    {
        GLOBAL_TRY_CATCH_BEGIN

        // For flushing initial resources
        Sync();

        GetFrameAllocator().DiscardAllocations();

        mLastTaskQueue.ExecuteAll();
        mLastTaskQueue.Flush();

        RendererManager::Render();

        platform::Platform::StartLoop();

        GLOBAL_TRY_CATCH_END
    }

    void RenderingThread::PrepareDestroy()
    {
        platform::Platform::GetResizeEvent().RemoveListener(mResizeEventFunc);
        platform::Platform::GetLoopEvent().RemoveListener(mLoopEventFunc);

        mSyncTaskQueue.Flush();
        mTaskQueue.Flush();
    }

    void RenderingThread::Destroy()
    {
        mTaskQueue.ExecuteAll();
        mTaskQueue.Flush();

        RendererManager::Shutdown();

        GetFrameAllocator().Shutdown();
    }

    void RenderingThread::Loop()
    {
        if(GameThread::mWillbeDestroyed == true) {
            platform::Platform::FinishLoop();
            return;
        }

        Sync();

        Async a = GameThread::ExecuteTaskQueueAndSimulateAsync();

        GetFrameAllocator().DiscardAllocations();

        mLastTaskQueue.ExecuteAll();
        mLastTaskQueue.Flush();

        Rendering();

        a.WaitUntilFinished();
    }

    void RenderingThread::Sync()
    {
        mSyncTaskQueue.ExecuteAll();
        mSyncTaskQueue.Flush();

        mTaskQueue.QueueAndFlush(mLastTaskQueue);
    }

    void RenderingThread::Rendering()
    {
        RendererManager::Render();
    }

    void RenderingThread::OnResize(Uint32 width, Uint32 height)
    {
        RendererManager::Resize(width, height);
    }
} // namespace cube

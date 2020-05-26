#pragma once

#include "../CoreHeader.h"

#include "../Thread/Async.h"
#include "../Thread/MutexLock.h"
#include "../Thread/TaskQueue.h"
#include "Utility/Event.h"

namespace cube
{
    class CORE_EXPORT RenderingThread
    {
    public:
        RenderingThread() = delete;
        ~RenderingThread() = delete;

        RenderingThread(const RenderingThread& other) = delete;
        RenderingThread& operator=(const RenderingThread& rhs) = delete;

        static void Init();

        static void Prepare();

        static void Run();

        static void PrepareDestroy();
        static void Destroy();

        static void QueueSyncTask(std::function<void()> taskFunc)
        {
            Lock lock(mSyncTaskQueueMutex);

            mSyncTaskQueue.QueueTask(taskFunc);
        }

        static void QueueTask(std::function<void()> taskFunc)
        {
            Lock lock(mTaskQueueMutex);

            mTaskQueue.QueueTask(taskFunc);
        }

        static std::thread::id GetThreadId() { return mThreadId; }

    private:
        friend class GameThread;

        static void Loop();

        static void Sync();
        static void Rendering();

        static void OnResize(Uint32 width, Uint32 height);

        static std::thread::id mThreadId;

        static EventFunction<void()> mLoopEventFunc;
        static EventFunction<void(Uint32, Uint32)> mResizeEventFunc;

        static Mutex mSyncTaskQueueMutex;
        static TaskQueue mSyncTaskQueue;

        static Mutex mTaskQueueMutex;
        static TaskQueue mTaskQueue;
        static TaskQueue mLastTaskQueue;
    };
} // namespace cube

#ifdef _DEBUG

#include "../Assertion.h"
#define CHECK_IF_RENDERING_THREAD CHECK(cube::RenderingThread::GetThreadId() == std::this_thread::get_id(), "This function should be called in rendering thread.");
#define CHECK_IF_NOT_RENDERING_THREAD CHECK(cube::RenderingThread::GetThreadId() != std::this_thread::get_id(), "This function should not be called in rendering thread.");

#else

#define CHECK_IF_RENDERING_THREAD
#define CHECK_IF_NOT_RENDERING_THREAD

#endif // _DEBUG

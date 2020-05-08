#pragma once

#include "CoreHeader.h"

#include "Core/Thread/Async.h"
#include "Core/Thread/TaskQueue.h"

namespace cube
{
    class CORE_EXPORT GameThread
    {
    public:
        GameThread() = delete;
        ~GameThread() = delete;

        GameThread(const GameThread& other) = delete;
        GameThread& operator=(const GameThread& rhs) = delete;

        static void Init();

        static Async PrepareAsync();

        static Async SimulateAsync();
        static Async StartAndSimulateAsync();
        static Async ExecuteTaskQueueAndSimulateAsync();

        static Async PrepareDestroyAsync();
        static Async DestroyAsync();

        static void SetDestroy() { mWillbeDestroyed = true; }

        static void Join() { mThread.join(); }

        static void QueueTask(std::function<void()> taskFunc)
        {
            Lock lock(mTaskQueueMutex);

            mTaskQueue.QueueTask(taskFunc);
        }

        static std::thread::id GetId() { return mThreadId; }

    private:
        friend class RenderingThread;

        static void Loop();

        static void PrepareInternal();

        static void StartAndSimulateInternal();
        static void ExecuteTaskQueueAndSimulateInternal();
        static void SimulateInternal();

        static void PrepareDestroyInternal();
        static void DestroyInternal();

        static std::thread mThread;
        static std::thread::id mThreadId;
        static std::function<void()> mRunFunction;

        static bool mWillbeDestroyed; // Used in RenderingThread

        static AsyncSignal mStartSignal;
        static AsyncSignal mFinishSignal;
        static AsyncSignal mDestroySignal;

        static Mutex mTaskQueueMutex;
        static TaskQueue mTaskQueue;
    };
} // namespace cube

#ifdef _DEBUG

#include "Assertion.h"
#define CHECK_IF_GAME_THREAD CHECK(cube::GameThread::GetThreadId() == std::this_thread::get_id(), "This function should be called in game thread.");
#define CHECK_IF_NOT_GAME_THREAD CHECK(cube::GameThread::GetThreadId() != std::this_thread::get_id(), "This function should not be called in game thread.");

#else

#define CHECK_IF_GAME_THREAD
#define CHECK_IF_NOT_GAME_THREAD

#endif // _DEBUG

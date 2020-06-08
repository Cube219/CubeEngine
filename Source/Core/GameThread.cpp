#include "GameThread.h"

#include "Core.h"
#include "Allocator/FrameAllocator.h"
#include "Assertion.h"

namespace cube
{
    std::thread GameThread::mThread;
    std::thread::id GameThread::mThreadId;
    std::function<void()> GameThread::mRunFunction = nullptr;

    bool GameThread::mWillbeDestroyed = false;

    AsyncSignal GameThread::mStartSignal;
    AsyncSignal GameThread::mFinishSignal;
    AsyncSignal GameThread::mDestroySignal;

    Mutex GameThread::mTaskQueueMutex;
    TaskQueue GameThread::mTaskQueue;

    void GameThread::Init()
    {
        mThread = std::thread(&GameThread::Loop);
        mThreadId = mThread.get_id();

        mFinishSignal.DispatchCompletion();
    }

    Async GameThread::PrepareAsync()
    {
        Async a = Async(mFinishSignal);
        a.WaitUntilFinished();

        mRunFunction = &GameThread::PrepareInternal;
        mStartSignal.DispatchCompletion();

        mFinishSignal.Reset();

        return Async(mFinishSignal);
    }

    Async GameThread::SimulateAsync()
    {
        Async a = Async(mFinishSignal);
        a.WaitUntilFinished();

        mRunFunction = &GameThread::SimulateInternal;
        mStartSignal.DispatchCompletion();

        mFinishSignal.Reset();

        return Async(mFinishSignal);
    }

    Async GameThread::StartAndSimulateAsync()
    {
        Async a = Async(mFinishSignal);
        a.WaitUntilFinished();

        mRunFunction = &GameThread::StartAndSimulateInternal;
        mStartSignal.DispatchCompletion();

        mFinishSignal.Reset();

        return Async(mFinishSignal);
    }

    Async GameThread::ExecuteTaskQueueAndSimulateAsync()
    {
        Async a = Async(mFinishSignal);
        a.WaitUntilFinished();

        mRunFunction = &GameThread::ExecuteTaskQueueAndSimulateInternal;
        mStartSignal.DispatchCompletion();

        mFinishSignal.Reset();

        return Async(mFinishSignal);
    }

    Async GameThread::PrepareDestroyAsync()
    {
        Async a = Async(mFinishSignal);
        a.WaitUntilFinished();

        mRunFunction = &GameThread::PrepareDestroyInternal;
        mStartSignal.DispatchCompletion();

        mFinishSignal.Reset();

        return Async(mFinishSignal);
    }

    Async GameThread::DestroyAsync()
    {
        Async a = Async(mFinishSignal);
        a.WaitUntilFinished();

        mRunFunction = &GameThread::DestroyInternal;
        mStartSignal.DispatchCompletion();

        mFinishSignal.Reset();

        return Async(mFinishSignal);
    }

    void GameThread::Loop()
    {
        GLOBAL_TRY_CATCH_BEGIN

        while(1) {
            { // Wait until start
                Async a = Async(mStartSignal);
                a.WaitUntilFinished();

                mFinishSignal.Reset();
            }

            mRunFunction();

            { // Check destroy signal
                Async a = Async(mDestroySignal);
                if(a.IsDone()) {
                    break;
                }
            }

            { // Finish
                mRunFunction = nullptr;
                mStartSignal.Reset();
                mFinishSignal.DispatchCompletion();
            }
        }

        mFinishSignal.DispatchCompletion();

        GLOBAL_TRY_CATCH_END
    }

    void GameThread::PrepareInternal()
    {
        GetFrameAllocator().Initialize("Game Thread FrameAllocator", 10 * 1024 * 1024); // 10 MiB

        Core::Initialize();
    }

    void GameThread::StartAndSimulateInternal()
    {
        Core::Start();

        SimulateInternal();
    }

    void GameThread::ExecuteTaskQueueAndSimulateInternal()
    {
        mTaskQueue.ExecuteAll();
        mTaskQueue.Flush();

        SimulateInternal();
    }

    void GameThread::SimulateInternal()
    {
        GetFrameAllocator().DiscardAllocations();

        Core::OnUpdate();
    }

    void GameThread::PrepareDestroyInternal()
    {
        mTaskQueue.Flush();
    }

    void GameThread::DestroyInternal()
    {
        Core::Shutdown();
        mTaskQueue.Flush();

        GetFrameAllocator().Shutdown();

        mDestroySignal.DispatchCompletion();
    }
} // namespace cube

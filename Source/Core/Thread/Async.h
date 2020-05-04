#pragma once

#include "../CoreHeader.h"

#include "MutexLock.h"

namespace cube
{
    class CORE_EXPORT AsyncSignal
    {
    public:
        AsyncSignal() : mProgress(0.0f), mIsFinished(false)
        {}

        void UpdateState(float progress)
        {
            Lock lock(mMutex);

            mProgress = progress;
        }
        void DispatchCompletion()
        {
            Lock lock(mMutex);

            mProgress = 1.0f;
            mIsFinished = true;
            mFinishSignal.notify_all();
        }

        void Reset()
        {
            Lock lock(mMutex);

            mProgress = 0.0f;
            mIsFinished = false;
        }

    private:
        friend class Async;

        ThreadSignal mFinishSignal;
        Mutex mMutex;
        bool mIsFinished;
        float mProgress;
    };

    class CORE_EXPORT Async
    {
    public:
        Async(AsyncSignal& signal) : mAsyncSignal(signal)
        {}

        void WaitUntilFinished()
        {
            Lock lock(mAsyncSignal.mMutex);

            if(mAsyncSignal.mIsFinished == false) {
                mAsyncSignal.mFinishSignal.wait(lock);
            }
        }

        bool IsDone()
        {
            Lock lock(mAsyncSignal.mMutex);

            return mAsyncSignal.mIsFinished;
        }

        float GetProgress()
        {
            Lock lock(mAsyncSignal.mMutex);

            return mAsyncSignal.mProgress;
        }

    private:
        AsyncSignal& mAsyncSignal;
    };

} // namespace cube

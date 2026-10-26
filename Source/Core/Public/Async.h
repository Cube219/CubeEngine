#pragma once

#include <mutex>

namespace cube
{
    class Signal
    {
    public:
        void Wait()
        {
            std::unique_lock<std::mutex> lock(mMutex);
            if (!mIsNotified)
            {
                mCV.wait(lock);
            }
        }

        void Notify()
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mIsNotified = true;
            mCV.notify_all();
        }

        void Reset()
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mIsNotified = false;
        }
        
    private:
        std::mutex mMutex;
        std::condition_variable mCV;
        bool mIsNotified = false;
    };
} // namespace cube

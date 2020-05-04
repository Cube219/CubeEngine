#pragma once

#include "../CoreHeader.h"

#include <functional>

namespace cube
{
    class TaskQueue
    {
    public:
        TaskQueue() {}
        ~TaskQueue()
        {
            mTasks.clear();
        }

        void QueueTask(std::function<void()> taskFunction)
        {
            mTasks.push_back(taskFunction);
        }

        void QueueAndFlush(TaskQueue& dst)
        {
            dst.mTasks.insert(dst.mTasks.end(), mTasks.begin(), mTasks.end());
            mTasks.clear();
        }

        void Flush()
        {
            mTasks.clear();
        }

        void ExecuteAll()
        {
            for(auto& fun : mTasks) {
                fun();
            }
        }

    private:
        Vector<std::function<void()>> mTasks;
    };
} // namespace cube

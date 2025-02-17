#pragma once

#include "Types.h"

#include <functional>

namespace cube
{
    template <class T>
    class Event; // Undefined

    template <class Func>
    class EventFunction
    {
    private:
        friend class Event<Func>;

        int mId;
        std::function<Func> mFunc;
    };

    template <class Return, class... Args>
    class Event<Return(Args...)>
    {
    public:
        Event()
        {
            mNextId = 0;
        }

        EventFunction<Return(Args...)> AddListener(std::function<Return(Args...)> func)
        {
            EventFunction<Return(Args...)> eFunc;
            eFunc.mFunc = func;
            eFunc.mId = mNextId++;

            mListeners.push_back(eFunc);
            return eFunc;
        }
        void RemoveListener(const EventFunction<Return(Args...)>& func)
        {
            auto findIter = std::find_if(mListeners.cbegin(), mListeners.cend(), [&func](const EventFunction<Return(Args...)>& f)
            {
                return f.mId == func.mId;
            });

            mListeners.erase(findIter);
        }

        void Dispatch(Args... args)
        {
            for (auto& l : mListeners)
            {
                l.mFunc(args...);
            }
        }

    private:
        int mNextId;
        Vector<EventFunction<Return(Args...)>> mListeners;
    };
} // namespace cube

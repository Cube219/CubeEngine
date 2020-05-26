#include "TimeManager.h"

#include "../LogWriter.h"

#include "GameTime.h"

#include <chrono>

namespace cube
{
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds>;
    using Duration = std::chrono::nanoseconds;

    Uint64 TimeManager::mPreviousSystemTimePoint;
    Uint64 TimeManager::mCurrentSystemTimePoint;

    GameTime TimeManager::mGlobalGameTime;
    Array<GameTime, 10> TimeManager::mGameTimes;

    void TimeManager::Initialize()
    {
    }

    void TimeManager::Shutdown()
    {
    }

    double TimeManager::GetSystemTime()
    {
        return GetNow() / systemTimeRatio;
    }

    void TimeManager::Start()
    {
        mCurrentSystemTimePoint = GetNow();

        mGlobalGameTime.Start();
    }

    void TimeManager::Update()
    {
        mPreviousSystemTimePoint = mCurrentSystemTimePoint;
        mCurrentSystemTimePoint = GetNow();

        Uint64 systemDeltaTime = mCurrentSystemTimePoint - mPreviousSystemTimePoint;

        mGlobalGameTime.Update(systemDeltaTime);
        for(auto& time : mGameTimes) {
            time.Update(systemDeltaTime);
        }
    }

    Uint64 TimeManager::GetNow()
    {
        return std::chrono::time_point_cast<Duration>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    }
} // namespace cube

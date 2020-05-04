#pragma once

#include "../CoreHeader.h"

#include "GameTime.h"

namespace cube
{
    constexpr double systemTimeRatio = 1000000000.0;

    CORE_EXPORT TimeManager& GetTimeManager();

    class CORE_EXPORT TimeManager
    {
    public:
        TimeManager() {}
        ~TimeManager() {}

        TimeManager(const TimeManager& other) = delete;
        TimeManager& operator=(const TimeManager& rhs) = delete;

        void Initialize();
        void ShutDown();

        double GetSystemTime();

        const GameTime& GetGlobalGameTime() const { return mGlobalGameTime; };

        GameTime& GetGameTime(Uint32 index) { return mGameTimes[index]; }

        void Start();

        void Update();

    private:
        Uint64 GetNow() const;

        Uint64 mPreviousSystemTimePoint;
        Uint64 mCurrentSystemTimePoint;

        GameTime mGlobalGameTime;
        Array<GameTime, 10> mGameTimes;
    };
} // namespace cube

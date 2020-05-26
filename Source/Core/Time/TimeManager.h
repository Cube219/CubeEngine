#pragma once

#include "../CoreHeader.h"

#include "GameTime.h"

namespace cube
{
    constexpr double systemTimeRatio = 1000000000.0;

    class CORE_EXPORT TimeManager
    {
    public:
        TimeManager() = delete;
        ~TimeManager() = delete;

        TimeManager(const TimeManager& other) = delete;
        TimeManager& operator=(const TimeManager& rhs) = delete;

        static void Initialize();
        static void Shutdown();

        static double GetSystemTime();

        static const GameTime& GetGlobalGameTime() { return mGlobalGameTime; };

        static GameTime& GetGameTime(Uint32 index) { return mGameTimes[index]; }

        static void Start();

        static void Update();

    private:
        static Uint64 GetNow();

        static Uint64 mPreviousSystemTimePoint;
        static Uint64 mCurrentSystemTimePoint;

        static GameTime mGlobalGameTime;
        static Array<GameTime, 10> mGameTimes;
    };
} // namespace cube

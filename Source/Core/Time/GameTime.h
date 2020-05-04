#pragma once

#include "../CoreHeader.h"

namespace cube
{
    constexpr float timeRatio = 1000000000.0f;

    class CORE_EXPORT GameTime
    {
    public:
        GameTime();
        ~GameTime();

        float GetCurrentTime() const { return mCurrentTime / timeRatio; };
        float GetDeltaTime() const { return (mCurrentTimePoint - mPreviousTimePoint) / timeRatio; };

        bool IsPaused() const { return mIsPaused; };


        void SetTimeScale(float scale) { mTimeScale = scale; };

        void Start();
        void Restart();

        void Pause();
        void Resume();

    private:
        friend class TimeManager;

        void Update(Uint64 systemDeltaTime);

        bool mIsStarted;
        bool mIsPaused;

        Uint64 mCurrentTime;
        float mTimeScale;

        Uint64 mPreviousTimePoint;
        Uint64 mCurrentTimePoint;
    };
} // namespace cube

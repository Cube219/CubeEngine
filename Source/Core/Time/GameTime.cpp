#include "GameTime.h"

#include "TimeManager.h"

namespace cube
{
    GameTime::GameTime() :
        mIsStarted(false),
        mTimeScale(1.0f)
    {}

    GameTime::~GameTime()
    {}

    void GameTime::Start()
    {
        if(mIsStarted)
            return;

        mPreviousTimePoint = 0;
        mCurrentTimePoint = 0;
        mCurrentTime = 0;

        mIsStarted = true;
        mIsPaused = false;
    }

    void GameTime::Restart()
    {
        mIsStarted = false;
        Start();
    }

    void GameTime::Pause()
    {
        mIsPaused = true;
    }

    void GameTime::Resume()
    {
        mIsPaused = false;
    }

    void GameTime::Update(Uint64 systemDeltaTime)
    {
        if(!mIsStarted || mIsPaused)
            return;

        mCurrentTime += SCast(Uint64)(systemDeltaTime * mTimeScale);

        mPreviousTimePoint = mCurrentTimePoint;
        mCurrentTimePoint = mCurrentTime;
    }
} // namespace cube

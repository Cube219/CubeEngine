#pragma once

#include "CoreHeader.h"

namespace cube
{
    class StatsSystem
    {
    public:
        StatsSystem() = delete;
        ~StatsSystem() = delete;

        static void Initialize();
        static void Shutdown();

        static void OnLoop(double deltaTime);
        static void OnLoopImGUI();

    private:
        static void CalculateStats(double deltaTimeSec);

        static float mCurrentFrameTimeMS;
        static float mCurrentFPS;
        static float mCurrentGPUTimeMS;

        static constexpr int NUM_AVERAGE_SAMPLE = 16;
        static float mSumFPSForAverage;
        static float mMinFPS;
        static float mMaxFPS;

        static constexpr Uint64 NUM_STATS_HISTORY = 400;
        static Uint64 mCurrentHistoryIndex;
        static Array<float, NUM_STATS_HISTORY * 2> mFrameTimeMSHistory;
        static Array<float, NUM_STATS_HISTORY * 2> mFPSHistory;
        static Array<float, NUM_STATS_HISTORY * 2> mGPUTimeMSHistory;

        static float mCurrentPhysicalVRAMMiB;
        static float mMaximumPhysicalVRAMMiB;
        static float mCurrentLogicalVRAMMiB;
        static float mMaximumLogicalVRAMMiB;
        static Array<float, NUM_STATS_HISTORY * 2> mPhysicalVRAMMiBHistory;
        static Array<float, NUM_STATS_HISTORY * 2> mLogicalVRAMMiBHistory;
    };
} // namespace cube

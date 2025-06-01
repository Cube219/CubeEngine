#include "StatsSystem.h"

#include "imgui.h"
#include "implot.h"

#include "Engine.h"
#include "Renderer/Renderer.h"

namespace cube
{
    float StatsSystem::mCurrentFrameTimeMS;
    float StatsSystem::mCurrentFPS;
    float StatsSystem::mCurrentGPUTimeMS;

    float StatsSystem::mSumFPSForAverage;
    float StatsSystem::mMinFPS;
    float StatsSystem::mMaxFPS;

    Uint64 StatsSystem::mCurrentHistoryIndex = 0;
    Array<float, StatsSystem::NUM_STATS_HISTORY * 2> StatsSystem::mFrameTimeMSHistory;
    Array<float, StatsSystem::NUM_STATS_HISTORY * 2> StatsSystem::mFPSHistory;
    Array<float, StatsSystem::NUM_STATS_HISTORY * 2> StatsSystem::mGPUTimeMSHistory;

    void StatsSystem::Initialize()
    {
    }

    void StatsSystem::Shutdown()
    {
    }

    void StatsSystem::OnLoop(double deltaTime)
    {
        CalculateFrameTimeAndFPS(deltaTime);
    }

    void StatsSystem::OnLoopImGUI()
    {
        // Always top-right position
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImGui::SetNextWindowPos({ work_pos.x + work_size.x - PAD, work_pos.x + PAD }, ImGuiCond_Always, { 1.0f, 0.0f });

        ImGui::Begin("Stats");

        if (ImGui::CollapsingHeader("Frame", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static double axisX_min = static_cast<double>(NUM_STATS_HISTORY) / 2;
            static double axisX_max = NUM_STATS_HISTORY - 1;

            ImGui::Text("FPS: %.2f (Avg: %.2f, Min: %.2f, Max: %.2f)", mCurrentFPS, mSumFPSForAverage / NUM_AVERAGE_SAMPLE, mMinFPS, mMaxFPS);
            if (ImPlot::BeginPlot("##FPS plot", ImVec2(-1, 150), ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText | ImPlotFlags_NoBoxSelect))
            {
                ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_LockMax, ImPlotAxisFlags_LockMin);
                ImPlot::SetupAxisLimits(ImAxis_X1, static_cast<double>(NUM_STATS_HISTORY) / 2,  NUM_STATS_HISTORY - 1);
                ImPlot::SetupAxisLimits(ImAxis_Y1,0, 120);
                ImPlot::SetupAxisLinks(ImAxis_X1, &axisX_min, &axisX_max);

                ImPlot::PlotLine("FPS", &mFPSHistory[mCurrentHistoryIndex], NUM_STATS_HISTORY);

                ImPlot::EndPlot();
            }

            ImGui::Text("FrameTime: %.3f ms", mCurrentFrameTimeMS);
            ImGui::Text("GPU: %.2f ms", mCurrentGPUTimeMS);

            if (ImPlot::BeginPlot("##Time plot", ImVec2(-1, 150), ImPlotFlags_NoMouseText | ImPlotFlags_NoBoxSelect))
            {
                ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_LockMax, ImPlotAxisFlags_LockMin);
                ImPlot::SetupAxisLimits(ImAxis_X1, static_cast<double>(NUM_STATS_HISTORY) / 2,  NUM_STATS_HISTORY - 1);
                ImPlot::SetupAxisLimits(ImAxis_Y1,0, 50);
                ImPlot::SetupAxisLinks(ImAxis_X1, &axisX_min, &axisX_max);

                ImPlot::PlotLine("FrameTime", &mFrameTimeMSHistory[mCurrentHistoryIndex], NUM_STATS_HISTORY);
                ImPlot::PlotLine("GPU", &mGPUTimeMSHistory[mCurrentHistoryIndex], NUM_STATS_HISTORY);

                ImPlot::EndPlot();
            }
        }

        ImGui::End();
    }

    void StatsSystem::CalculateFrameTimeAndFPS(double deltaTimeSec)
    {
        // TODO: Smooth way?
        mCurrentFrameTimeMS = static_cast<float>(deltaTimeSec * 1000.0f);
        mCurrentFPS = static_cast<float>(1.0 / deltaTimeSec);
        mCurrentGPUTimeMS = Engine::GetRenderer()->GetGPUTimeMS();

        mFrameTimeMSHistory[mCurrentHistoryIndex] = mFrameTimeMSHistory[mCurrentHistoryIndex + NUM_STATS_HISTORY] = mCurrentFrameTimeMS;
        mFPSHistory[mCurrentHistoryIndex] = mFPSHistory[mCurrentHistoryIndex + NUM_STATS_HISTORY] = mCurrentFPS;
        mGPUTimeMSHistory[mCurrentHistoryIndex] = mGPUTimeMSHistory[mCurrentHistoryIndex + NUM_STATS_HISTORY] = mCurrentGPUTimeMS;

        mCurrentHistoryIndex = (mCurrentHistoryIndex + 1) % NUM_STATS_HISTORY;

        mSumFPSForAverage = 0;
        mMinFPS = 10000000.0f;
        mMaxFPS = 0.0f;
        for (int i = 0; i < NUM_AVERAGE_SAMPLE; ++i)
        {
            float sampleFPS = mFPSHistory[mCurrentHistoryIndex + NUM_STATS_HISTORY - 1 - i];

            mSumFPSForAverage += sampleFPS;
            mMinFPS = std::min(mMinFPS, sampleFPS);
            mMaxFPS = std::max(mMaxFPS, sampleFPS);
        }
    }
} // namespace cube

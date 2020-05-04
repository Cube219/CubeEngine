#include "Core.h"

#include "Platform/Platform.h"
#include "LogWriter.h"
#include "Time/TimeManager.h"

namespace cube
{
    static Core gCore;

    Core& GetCore()
    {
        return gCore;
    }

    void Core::PreInitialize()
    {
        LogWriter::Init();
    }

    void Core::Initialize()
    {
        GetTimeManager().Initialize();
    }

    void Core::Shutdown()
    {
        GetTimeManager().ShutDown();
    }

    void Core::Start()
    {
        GetTimeManager().Start();
    }

    float Core::GetFPS()
    {
        // TODO: 더 좋은 방법으로 개선
        return 1.0f / GetTimeManager().GetGlobalGameTime().GetDeltaTime();
    }

    void Core::SetFPSLimit(Uint64 fps)
    {
        mFPSLimit = fps;
    }

    void Core::OnUpdate()
    {
        GetTimeManager().Update();

        // Limit FPS
        double currentTime = GetTimeManager().GetSystemTime();

        if(mFPSLimit > 0) {
            double nextTime = currentTime + (1.0 / mFPSLimit);
            currentTime = GetTimeManager().GetSystemTime();

            double waitTime = nextTime - currentTime;

            if(waitTime > 0.1) {
                platform::Platform::Sleep(SCast(int)(waitTime * 1000));
            } else if(waitTime > 0.0) {
                while(nextTime > currentTime) {
                    currentTime = GetTimeManager().GetSystemTime();
                }
            }
        }
    }

    void Core::OnResize(Uint32 width, Uint32 height)
    {
    }
} // namespace cube

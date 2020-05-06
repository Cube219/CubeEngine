#include "Core.h"

#include "Platform/Platform.h"
#include "LogWriter.h"
#include "Time/TimeManager.h"

namespace cube
{
    Uint64 Core::mFPSLimit = 0;

    void Core::PreInitialize()
    {
        LogWriter::Init();
    }

    void Core::Initialize()
    {
        TimeManager::Initialize();
    }

    void Core::Shutdown()
    {
        TimeManager::ShutDown();
    }

    void Core::Start()
    {
        TimeManager::Start();
    }

    float Core::GetFPS()
    {
        // TODO: 더 좋은 방법으로 개선
        return 1.0f / TimeManager::GetGlobalGameTime().GetDeltaTime();
    }

    void Core::SetFPSLimit(Uint64 fps)
    {
        mFPSLimit = fps;
    }

    void Core::OnUpdate()
    {
        TimeManager::Update();

        // Limit FPS
        double currentTime = TimeManager::GetSystemTime();

        if(mFPSLimit > 0) {
            double nextTime = currentTime + (1.0 / mFPSLimit);
            currentTime = TimeManager::GetSystemTime();

            double waitTime = nextTime - currentTime;

            if(waitTime > 0.1) {
                platform::Platform::Sleep(SCast(int)(waitTime * 1000));
            } else if(waitTime > 0.0) {
                while(nextTime > currentTime) {
                    currentTime = TimeManager::GetSystemTime();
                }
            }
        }
    }

    void Core::OnResize(Uint32 width, Uint32 height)
    {
    }
} // namespace cube

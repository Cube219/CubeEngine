#include "Core.h"

#include "Platform/Platform.h"
#include "LogWriter.h"
#include "Time/TimeManager.h"
#include "Module/ModuleManager.h"
#include "Resource/ResourceManager.h"

namespace cube
{
    Uint64 Core::mFPSLimit = 0;
    entt::registry Core::mRegistry;

    World Core::mWorld(mRegistry);

    void Core::PreInitialize()
    {
        LogWriter::Init();
    }

    void Core::Initialize()
    {
        // RendererManager will be initialized in RenderingThread

        TimeManager::Initialize();

        ResourceManager::Initialize();

        ModuleManager::Initialize();
        ModuleManager::LoadModule(CUBE_T("CE-InputModule"));
        ModuleManager::InitModules();

        mWorld.Initialize();

        HWorldObject wo = WorldObject::Create();
        HWorldObject wo2 = WorldObject::Create();

        wo->Destroy();
        wo2->Destroy();
    }

    void Core::Shutdown()
    {
        // RendererManager will be initialized in RenderingThread

        mWorld.Shutdown();

        ModuleManager::Shutdown();

        ResourceManager::Shutdown();

        TimeManager::Shutdown();
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

        double currentTime = TimeManager::GetSystemTime();

        float dt = TimeManager::GetGlobalGameTime().GetDeltaTime();

        mWorld.Update(dt);

        ModuleManager::UpdateAllModules(dt);

        // Limit FPS

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

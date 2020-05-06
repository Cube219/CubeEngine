#include "Engine.h"

#include "Platform/Platform.h"
#include "Core/Core.h"
#include "Core/GameThread.h"
#include "Core/Renderer/RenderingThread.h"

namespace cube
{
    EventFunction<void()> Engine::closingEventFunc;

    void Engine::Initialize(const EngineInitOption& initOption)
    {
        using namespace platform;
        Platform::Init();

        Platform::InitWindow(initOption.title,
            initOption.windowWidth, initOption.windowHeight, initOption.windowPositionX, initOption.windowPositionY);
        Platform::ShowWindow();

        Core::PreInitialize();

        RenderingThread::Init();
        GameThread::Init();

        Async gtPrepareAsync = GameThread::PrepareAsync();
        RenderingThread::Prepare();
        gtPrepareAsync.WaitUntilFinished();

        closingEventFunc = Platform::GetClosingEvent().AddListener(&Engine::DefaultClosingFunction);
        Core::SetFPSLimit(60);
    }

    void Engine::ShutDown()
    {
        platform::Platform::GetClosingEvent().RemoveListener(closingEventFunc);

        Async gtPrepareDestroyAsync = GameThread::PrepareDestroyAsync();
        RenderingThread::PrepareDestroy();
        gtPrepareDestroyAsync.WaitUntilFinished();

        Async gtDestroyAsync = GameThread::DestroyAsync();
        gtDestroyAsync.WaitUntilFinished();
        RenderingThread::Destroy();

        GameThread::Join();
    }

    void Engine::Run()
    {
        Async gtSimulateAsync = GameThread::SimulateAsync();
        gtSimulateAsync.WaitUntilFinished();
        RenderingThread::Run();
    }

    void Engine::Close()
    {
        GameThread::SetDestroy();
    }

    void Engine::SetCustomClosingFunction(std::function<void()> func)
    {
        platform::Platform::GetClosingEvent().RemoveListener(closingEventFunc);
        closingEventFunc = platform::Platform::GetClosingEvent().AddListener(func);
    }

    void Engine::DefaultClosingFunction()
    {
        Close();
    }
} // namespace cube

#include "Engine.h"

#include "imgui.h"

#include "Checker.h"
#include "FileSystem.h"
#include "GAPI_Viewport.h"
#include "Logger.h"
#include "Platform.h"
#include "PlatformDebug.h"

#include "Allocator/FrameAllocator.h"
#include "Renderer/Renderer.h"

namespace cube
{
    class STDAllocator : public IAllocator
    {
    public:
        void* Allocate(SizeType n) override
        {
            return malloc(n);
        }

        void Free(void* ptr, SizeType n) override
        {
            free(ptr);
        }
    };

    STDAllocator tempAllocator; // TODO: Use frame allocator

    class EngineLoggerExtension : public ILoggerExtension
    {
    public:
        void WriteFormattedLog(StringView formattedLog) override
        {
            platform::PlatformDebug::PrintToDebugConsole(formattedLog);
        }
    };

    class EngineCheckerExtension : public ICheckerExtension
    {
    public:
        void ProcessFailedCheck(const char* fullFileName, int lineNum, StringView exprAndMsg) override
        {
            String stackTrace = platform::PlatformDebug::DumpStackTrace();
            const char* fileName = platform::FileSystem::SplitFileNameFromFullPath(fullFileName);

            String formattedMsg = Format(CUBE_T("Check failed!\n    In {}:{}\n    {}\n\n{}"), fileName, lineNum, exprAndMsg, stackTrace);
            Logger::WriteLogFormatting(LogType::Error, fullFileName, lineNum, CUBE_T("Checker"), formattedMsg);

            platform::PlatformDebug::ProcessFailedCheck(fileName, lineNum, formattedMsg);
        }
    };

    EventFunction<void()> Engine::mOnClosingEventFunc;
    EventFunction<void()> Engine::mOnLoopEventFunc;
    EventFunction<void(Uint32, Uint32)> Engine::mOnResizeEventFunc;

    UniquePtr<Renderer> Engine::mRenderer;

    ImGUIContext Engine::mImGUIContext;
    bool Engine::mImGUIShowDemoWindow = true;

    void Engine::Initialize()
    {
        platform::Platform::Init();

        GetMyThreadFrameAllocator().Initialize("Mane thread frame allocator", 10 * 1024 * 1024); // 10 MiB

        Logger::Init(&tempAllocator);
        Logger::SetFilePathSeparator(platform::FileSystem::GetSeparator());
        Logger::RegisterExtension<EngineLoggerExtension>();

        Checker::RegisterExtension<EngineCheckerExtension>();

        platform::Platform::InitWindow(CUBE_T("CubeEngine"), 1024, 768, 100, 100);
        platform::Platform::ShowWindow();

        mOnLoopEventFunc = platform::Platform::GetLoopEvent().AddListener(OnLoop);
        mOnClosingEventFunc = platform::Platform::GetClosingEvent().AddListener(OnClosing);
        mOnResizeEventFunc = platform::Platform::GetResizeEvent().AddListener(OnResize);

        CUBE_LOG(LogType::Info, Engine, "Initialize CubeEngine.");
        {
            // ImGUI init
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

            ImGui::StyleColorsDark();

            mImGUIContext.context = ImGui::GetCurrentContext();
            ImGui::GetAllocatorFunctions(
                (ImGuiMemAllocFunc*)&mImGUIContext.allocFunc,
                (ImGuiMemFreeFunc*)&mImGUIContext.freeFunc,
                &mImGUIContext.userData
            );
        }

        mRenderer = std::make_unique<Renderer>();
        mRenderer->Initialize(GAPIName::DX12, mImGUIContext);

        CUBE_LOG(LogType::Info, Engine, "Start CubeEngine.");
        platform::Platform::StartLoop();
    }

    void Engine::Shutdown()
    {
        CUBE_LOG(LogType::Info, Engine, "Shutdown CubeEngine.");

        mRenderer->Shutdown(mImGUIContext);
        mRenderer = nullptr;

        ImGui::SetCurrentContext((ImGuiContext*)(mImGUIContext.context));
        ImGui::SetAllocatorFunctions(
            (ImGuiMemAllocFunc)(mImGUIContext.allocFunc),
            (ImGuiMemFreeFunc)(mImGUIContext.freeFunc),
            (void*)(mImGUIContext.userData));

        ImGui::DestroyContext((ImGuiContext*)mImGUIContext.context);

        platform::Platform::GetResizeEvent().RemoveListener(mOnResizeEventFunc);
        platform::Platform::GetClosingEvent().RemoveListener(mOnClosingEventFunc);
        platform::Platform::GetLoopEvent().RemoveListener(mOnLoopEventFunc);

        GetMyThreadFrameAllocator().Shutdown();
    }

    void Engine::OnLoop()
    {
        GetMyThreadFrameAllocator().DiscardAllocations();

        LoopImGUI();

        mRenderer->Render();
    }

    void Engine::OnClosing()
    {
        platform::Platform::FinishLoop();
    }

    void Engine::OnResize(Uint32 width, Uint32 height)
    {
        mRenderer->OnResize(width, height);
    }

    void Engine::LoopImGUI()
    {
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (mImGUIShowDemoWindow)
            ImGui::ShowDemoWindow(&mImGUIShowDemoWindow);
    }
} // namespace cube

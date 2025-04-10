#include "Engine.h"

#include <chrono>
#include "imgui.h"

#include "Checker.h"
#include "FileSystem.h"
#include "Logger.h"
#include "Platform.h"
#include "PlatformDebug.h"

#include "Allocator/FrameAllocator.h"
#include "Camera.h"
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
        void WriteFormattedLog(LogType logType, StringView formattedLog) override
        {
            platform::PrintColorCategory colorCategory;
            switch (logType)
            {
            case LogType::Warning:
                colorCategory = platform::PrintColorCategory::Warning;
                break;
            case LogType::Error:
                colorCategory = platform::PrintColorCategory::Error;
                break;
            case LogType::Info:
            default:
                colorCategory = platform::PrintColorCategory::Default;
                break;
            }
            platform::PlatformDebug::PrintToDebugConsole(formattedLog, colorCategory);
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
    bool Engine::mRunShutdownInClosingFunc;

    UniquePtr<Renderer> Engine::mRenderer;

    ImGUIContext Engine::mImGUIContext;
    bool Engine::mImGUIShowDemoWindow = true;

    String Engine::mRootDirectoryPath = CUBE_T("../..");

    Uint64 Engine::mStartTime;
    Uint64 Engine::mLastTime;
    Uint64 Engine::mCurrentTime;

    void Engine::Initialize(int argc, const char* argv[], bool runShutdownInOnClosingFunc)
    {
        mRunShutdownInClosingFunc = runShutdownInOnClosingFunc;

        platform::Platform::Initialize();

        GetMyThreadFrameAllocator().Initialize("Main thread frame allocator", 10 * 1024 * 1024); // 10 MiB

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

        // mRenderer = std::make_unique<Renderer>();
        // mRenderer->Initialize(GAPIName::DX12, mImGUIContext);

        // CameraSystem::Initialize();

        mStartTime = GetNowFrameTime();
        mCurrentTime = mStartTime;

        CUBE_LOG(LogType::Info, Engine, "Start CubeEngine.");
        platform::Platform::StartLoop();
    }

    void Engine::Shutdown()
    {
        CUBE_LOG(LogType::Info, Engine, "Shutdown CubeEngine.");

        // CameraSystem::Shutdown();

        // mRenderer->Shutdown(mImGUIContext);
        // mRenderer = nullptr;

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

        platform::Platform::Shutdown();
    }

    void Engine::OnLoop()
    {
        GetMyThreadFrameAllocator().DiscardAllocations();

        mLastTime = mCurrentTime;
        mCurrentTime = GetNowFrameTime();

        const double deltaTimeSec = static_cast<double>(mCurrentTime - mLastTime) / std::nano::den;

        // CameraSystem::OnLoop(deltaTimeSec);

        LoopImGUI();

        // mRenderer->Render();
    }

    void Engine::OnClosing()
    {
        platform::Platform::FinishLoop();

        // Also execute Engine::Shutdown function. Some platform cannot execute remain logic after Engine::Initialize().
        //   Ex) MacOS
        if (mRunShutdownInClosingFunc)
        {
            Shutdown();
        }
    }

    void Engine::OnResize(Uint32 width, Uint32 height)
    {
        // mRenderer->OnResize(width, height);
        // CameraSystem::OnResize(width, height);
    }

    void Engine::LoopImGUI()
    {
        // ImGui::NewFrame();

        // // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        // if (mImGUIShowDemoWindow)
        //     ImGui::ShowDemoWindow(&mImGUIShowDemoWindow);

        // CameraSystem::OnLoopImGUI();
    }

    Uint64 Engine::GetNowFrameTime()
    {
        return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    }
} // namespace cube

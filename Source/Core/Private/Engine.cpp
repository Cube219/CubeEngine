#include "Engine.h"

#include <chrono>
#include "imgui.h"

#include "Checker.h"
#include "FileSystem.h"
#include "Logger.h"
#include "Platform.h"
#include "PlatformDebug.h"

#include "Allocator/FrameAllocator.h"
#include "Renderer/Renderer.h"
#include "Systems/CameraSystem.h"
#include "Systems/ModelLoaderSystem.h"

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
    bool Engine::mDrawImGUI;

    ImGUIContext Engine::mImGUIContext;
    bool Engine::mImGUIShowDemoWindow = true;

    String Engine::mRootDirectoryPath;

    Uint64 Engine::mStartTime;
    Uint64 Engine::mLastTime;
    Uint64 Engine::mCurrentTime;
    float Engine::mCurrentFrameTimeMS;
    float Engine::mCurrentFPS;
    float Engine::mCurrentGPUTimeMS;

    void Engine::Initialize(const EngineInitializeInfo& initInfo)
    {
        mRunShutdownInClosingFunc = initInfo.runShutdownInOnClosingFunc;

        platform::Platform::Initialize();

        GetMyThreadFrameAllocator().Initialize("Main thread frame allocator", 100u * 1024 * 1024); // 100 MiB

        Logger::Init(&tempAllocator);
        Logger::SetFilePathSeparator(platform::FileSystem::GetSeparator());
        Logger::RegisterExtension<EngineLoggerExtension>();

        Checker::RegisterExtension<EngineCheckerExtension>();

        CUBE_LOG(Info, Engine, "Initialize CubeEngine.");

        SearchAndSetRootDirectory();

        platform::Platform::InitWindow(CUBE_T("CubeEngine"), 1200, 900, 100, 100);
        platform::Platform::ShowWindow();

        mOnLoopEventFunc = platform::Platform::GetLoopEvent().AddListener(OnLoop);
        mOnClosingEventFunc = platform::Platform::GetClosingEvent().AddListener(OnClosing);
        mOnResizeEventFunc = platform::Platform::GetResizeEvent().AddListener(OnResize);

        mDrawImGUI = initInfo.drawImGUI;
        if (mDrawImGUI)
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
                &mImGUIContext.userData);
        }

        mRenderer = std::make_unique<Renderer>();
        CUBE_LOG(Info, Engine, "Using GAPI {}.", GAPINameToString(initInfo.gapi));
        mRenderer->Initialize(initInfo.gapi, mImGUIContext);

        ModelLoaderSystem::Initialize();
        CameraSystem::Initialize();

        mStartTime = GetNow();
        mCurrentTime = mStartTime;

        CUBE_LOG(Info, Engine, "Start CubeEngine.");
        platform::Platform::StartLoop();
    }

    void Engine::Shutdown()
    {
        CUBE_LOG(Info, Engine, "Shutdown CubeEngine.");

        CameraSystem::Shutdown();
        ModelLoaderSystem::Shutdown();

        mRenderer->Shutdown(mImGUIContext);
        mRenderer = nullptr;

        if (mDrawImGUI)
        {
            ImGui::SetCurrentContext((ImGuiContext*)(mImGUIContext.context));
            ImGui::SetAllocatorFunctions(
                (ImGuiMemAllocFunc)(mImGUIContext.allocFunc),
                (ImGuiMemFreeFunc)(mImGUIContext.freeFunc),
                (void*)(mImGUIContext.userData));

            ImGui::DestroyContext((ImGuiContext*)mImGUIContext.context);
        }
        platform::Platform::GetResizeEvent().RemoveListener(mOnResizeEventFunc);
        platform::Platform::GetClosingEvent().RemoveListener(mOnClosingEventFunc);
        platform::Platform::GetLoopEvent().RemoveListener(mOnLoopEventFunc);

        GetMyThreadFrameAllocator().Shutdown();

        platform::Platform::Shutdown();
    }

    void Engine::SetMesh(SharedPtr<MeshData> mesh)
    {
        mRenderer->SetMesh(mesh);
    }

    void Engine::OnLoop()
    {
        GetMyThreadFrameAllocator().DiscardAllocations();

        mLastTime = mCurrentTime;
        mCurrentTime = GetNow();

        const double deltaTimeSec = static_cast<double>(mCurrentTime - mLastTime) / std::nano::den;
        CalculateFrameTimeAndFPS(deltaTimeSec);

        CameraSystem::OnLoop(deltaTimeSec);

        LoopImGUI();

        mRenderer->Render();
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
        mRenderer->OnResize(width, height);
        CameraSystem::OnResize(width, height);
    }

    void Engine::LoopImGUI()
    {
        if (!mDrawImGUI)
        {
            return;
        }

        ImGui::NewFrame();

        // // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (mImGUIShowDemoWindow)
            ImGui::ShowDemoWindow(&mImGUIShowDemoWindow);

        { // Basic stats
            static bool showBasicStats = true;

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

            const float PAD = 10.0f;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
            ImVec2 work_size = viewport->WorkSize;
            ImGui::SetNextWindowPos({ work_pos.x + work_size.x - PAD, work_pos.x + PAD }, ImGuiCond_Always, { 1.0f, 0.0f });
            ImGui::SetNextWindowBgAlpha(0.35f);
            ImGui::SetNextWindowSize({ 180.0f, 70.0f });
            if (ImGui::Begin("Basic Stats", &showBasicStats, flags))
            {
                ImGui::Text("FrameTime: %.3f ms", mCurrentFrameTimeMS);
                ImGui::Text("FPS: %.2f ms", mCurrentFPS);
                ImGui::Text("GPU: %.2f ms", mCurrentGPUTimeMS);
            }
            ImGui::End();
        }

        CameraSystem::OnLoopImGUI();
        ModelLoaderSystem::OnLoopImGUI();
    }

    Uint64 Engine::GetNow()
    {
        return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    }

    void Engine::CalculateFrameTimeAndFPS(double deltaTimeSec)
    {
        // TODO: Smooth way?
        mCurrentFrameTimeMS = static_cast<float>(deltaTimeSec * 1000.0f);
        mCurrentFPS = static_cast<float>(1.0 / deltaTimeSec);
        mCurrentGPUTimeMS = mRenderer->GetGPUTimeMS();
    }

    void Engine::SearchAndSetRootDirectory()
    {
        FrameString path(platform::FileSystem::GetCurrentDirectoryPath());

        // Find the CubeEngine directory
        const Character sep = platform::FileSystem::GetSeparator();
        while (1)
        {
            Vector<String> list = platform::FileSystem::GetList(path);
            if (std::find(list.begin(), list.end(), CUBE_T("Resources")) != list.end())
            {
                mRootDirectoryPath = path;
                CUBE_LOG(Info, Engine, "Found the root directory path: {0}", mRootDirectoryPath);
                return;
            }

            int i;
            for (i = path.size() - 1; i >= 0; --i)
            {
                if (path[i] == sep)
                {
                    break;
                }
            }

            if (i >= 1)
            {
                // Move to parent
                path = path.substr(0, i);
            }
            else
            {
                mRootDirectoryPath = platform::FileSystem::GetCurrentDirectoryPath();
                CUBE_LOG(Error, Engine, "Failed to find the root directory path. Use the current directory path: {0}", mRootDirectoryPath);
                break;
            }
        }
    }
} // namespace cube

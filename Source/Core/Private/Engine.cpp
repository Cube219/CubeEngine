#include "Engine.h"

#include <chrono>
#include "imgui.h"
#include "implot.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "FileSystem.h"
#include "Logger.h"
#include "PathHelper.h"
#include "Platform.h"
#include "PlatformDebug.h"
#include "Renderer/Renderer.h"
#include "Systems/CameraSystem.h"
#include "Systems/ModelLoaderSystem.h"
#include "Systems/StatsSystem.h"

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

            String formattedMsg = Format(CUBE_T("Check failed!\n\n{0}\n    In {1}:{2}\n    {3}"), stackTrace, fileName, lineNum, exprAndMsg);
            Logger::WriteLogFormatting(LogType::Error, fullFileName, lineNum, CUBE_T("Checker"), formattedMsg);

            Checker::SetIsDebuggerAttached(platform::PlatformDebug::IsDebuggerAttached());
            platform::PlatformDebug::ProcessFailedCheck(fileName, lineNum, formattedMsg);
        }
    };

    EventFunction<void()> Engine::mOnClosingEventFunc;
    EventFunction<void()> Engine::mOnLoopEventFunc;
    EventFunction<void(Uint32, Uint32)> Engine::mOnResizeEventFunc;

    UniquePtr<Renderer> Engine::mRenderer;
    bool Engine::mDrawImGUI;

    ImGUIContext Engine::mImGUIContext;
    bool Engine::mImGUIShowDemoWindow = true;

    String Engine::mRootDirectoryPath;

    Uint64 Engine::mStartTime;
    Uint64 Engine::mLastTime;
    Uint64 Engine::mCurrentTime;

    void Engine::Initialize(const EngineInitializeInfo& initInfo)
    {
        platform::Platform::Initialize();

        if (initInfo.runInitializeAndShutdownInLoopFunction)
        {
            // Initialization logic will be executed in another main loop thread.
            return;
        }

        GetMyThreadFrameAllocator().Initialize("Main thread frame allocator", 100u * 1024 * 1024); // 100 MiB

        Logger::Init(&tempAllocator);
        Logger::SetFilePathSeparator(platform::FileSystem::GetSeparator());
        Logger::RegisterExtension<EngineLoggerExtension>();

        Checker::RegisterExtension<EngineCheckerExtension>();

        CUBE_LOG(Info, Engine, "Initialize CubeEngine.");

        PathHelper::Initialize();

        SearchAndSetRootDirectory();

        platform::Platform::InitWindow(CUBE_T("CubeEngine"), 1400, 900, 100, 100);
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

            ImPlot::CreateContext();
            ImPlot::StyleColorsDark();
        }

        mRenderer = std::make_unique<Renderer>();
        CUBE_LOG(Info, Engine, "Using GAPI {}.", GAPINameToString(initInfo.gapi));
        mRenderer->Initialize(initInfo.gapi, mImGUIContext);

        StatsSystem::Initialize();
        ModelLoaderSystem::Initialize();
        CameraSystem::Initialize();
    }

    void Engine::StartLoop()
    {
        CUBE_LOG(Info, Engine, "Start CubeEngine.");

        mStartTime = GetNow();
        mCurrentTime = mStartTime;

        platform::Platform::StartLoop();
    }

    void Engine::Shutdown()
    {
        CUBE_LOG(Info, Engine, "Shutdown CubeEngine.");

        CameraSystem::Shutdown();
        ModelLoaderSystem::Shutdown();
        StatsSystem::Shutdown();

        mRenderer->Shutdown(mImGUIContext);
        mRenderer = nullptr;

        if (mDrawImGUI)
        {
            ImGui::SetCurrentContext((ImGuiContext*)(mImGUIContext.context));
            ImGui::SetAllocatorFunctions(
                (ImGuiMemAllocFunc)(mImGUIContext.allocFunc),
                (ImGuiMemFreeFunc)(mImGUIContext.freeFunc),
                (void*)(mImGUIContext.userData));

            ImPlot::DestroyContext();
            ImGui::DestroyContext((ImGuiContext*)mImGUIContext.context);
        }
        platform::Platform::GetResizeEvent().RemoveListener(mOnResizeEventFunc);
        platform::Platform::GetClosingEvent().RemoveListener(mOnClosingEventFunc);
        platform::Platform::GetLoopEvent().RemoveListener(mOnLoopEventFunc);

        PathHelper::Shutdown();

        GetMyThreadFrameAllocator().Shutdown();

        platform::Platform::Shutdown();
    }

    void Engine::SetMesh(SharedPtr<MeshData> mesh)
    {
        mRenderer->SetMesh(mesh);
    }

    void Engine::SetMaterials(const Vector<SharedPtr<Material>>& materials)
    {
        mRenderer->SetMaterials(materials);
    }

    void Engine::OnLoop()
    {
        GetMyThreadFrameAllocator().DiscardAllocations();

        mLastTime = mCurrentTime;
        mCurrentTime = GetNow();

        const double deltaTimeSec = static_cast<double>(mCurrentTime - mLastTime) / std::nano::den;

        CameraSystem::OnLoop(deltaTimeSec);
        StatsSystem::OnLoop(deltaTimeSec);

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

        CameraSystem::OnLoopImGUI();
        mRenderer->OnLoopImGUI();
        ModelLoaderSystem::OnLoopImGUI();
        StatsSystem::OnLoopImGUI();
    }

    Uint64 Engine::GetNow()
    {
        return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
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

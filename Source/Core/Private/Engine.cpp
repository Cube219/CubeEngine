#include "Engine.h"

#include <chrono>
#include "imgui.h"
#include "implot.h"

#include "Allocator/FrameAllocator.h"
#include "Checker.h"
#include "FileSystem.h"
#include "Logger.h"
#include "Platform.h"
#include "PlatformDebug.h"
#include "Renderer/Renderer.h"
#include "Systems/CameraSystem.h"
#include "Systems/ModelLoaderSystem.h"
#include "Systems/StatsSystem.h"

namespace cube
{
    class FrameAllocatorAdapter : public IAllocator
    {
    public:
        void* Allocate(SizeType n) override
        {
            return GetMyThreadFrameAllocator().Allocate(n);
        }

        void Free(void* ptr, SizeType n) override
        {
            GetMyThreadFrameAllocator().Free(ptr);
        }
    };

    FrameAllocatorAdapter frameAllocatorAdapter;

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
            platform::Debug::PrintToDebugConsole(formattedLog, colorCategory);
        }
    };

    class EngineCheckerExtension : public ICheckerExtension
    {
    public:
        void ProcessFailedCheck(const char* fullFileName, int lineNum, StringView exprAndMsg) override
        {
            String stackTrace = platform::Debug::DumpStackTrace();
            const char* fileName = platform::FileSystem::SplitFileNameFromFullPath(fullFileName);

            String formattedMsg = Format(CUBE_T("Check failed!\n\n{0}\n    In {1}:{2}\n    {3}"), stackTrace, fileName, lineNum, exprAndMsg);
            Logger::WriteLogFormatting(LogType::Error, fullFileName, lineNum, CUBE_T("Checker"), formattedMsg);

            Checker::SetIsDebuggerAttached(platform::Debug::IsDebuggerAttached());
            platform::Debug::ProcessFailedCheck(fileName, lineNum, formattedMsg);
        }
    };

    EventFunction<void()> Engine::mOnClosingEventFunc;
    EventFunction<void()> Engine::mOnLoopEventFunc;
    EventFunction<void(Uint32, Uint32)> Engine::mOnResizeEventFunc;

    UniquePtr<Renderer> Engine::mRenderer;
    bool Engine::mDrawImGUI;

    ImGUIContext Engine::mImGUIContext;
    bool Engine::mImGUIShowDemoWindow = true;

    platform::FilePath Engine::mRootDirectoryPath;
    platform::FilePath Engine::mShaderDirectoryPath;

    HashMap<AnsiString, AnsiString> Engine::mCommandLineArgs;

    Uint64 Engine::mStartTime;
    Uint64 Engine::mLastTime;
    Uint64 Engine::mCurrentTime;

    Uint32 Engine::mLoopCount = 0;

    void Engine::Initialize(const EngineInitializeInfo& initInfo)
    {
        if (initInfo.runLoopInOtherThread && initInfo.isInMainThread)
        {
            // Initialize frame allocator at platform main thread if the loop function does not run in main thread.
            // Some platform dispatch events in platform main thread. (e.g. MacOS)
            // Discard allocation will be executed in PostLoopFunction.
            GetMyThreadFrameAllocator().Initialize("Platform main thread frame allocator", 1 * 1024 * 1024); // 1 MiB
        }

        ParseCommandLineArgs(initInfo.argc, initInfo.argv);

        if (AnsiStringView testParam = GetCommandLineParam("test"); testParam == "1")
        {
            platform::Debug::SetTestMode(true);
        }

        platform::Platform::Initialize();

        if (initInfo.runLoopInOtherThread && initInfo.isInMainThread)
        {
            // Initialization logic was already executed in another loop thread.
            return;
        }

        GetMyThreadFrameAllocator().Initialize("Main thread frame allocator", 100u * 1024 * 1024); // 100 MiB

        Logger::Init(&frameAllocatorAdapter);
        Logger::SetFilePathSeparator(platform::FileSystem::GetSeparator());
        Logger::RegisterExtension<EngineLoggerExtension>();

        platform::Platform::SetPostLoopMainThreadFunction([]() {
            GetMyThreadFrameAllocator().DiscardAllocations();
        });

        Checker::RegisterExtension<EngineCheckerExtension>();

        CUBE_LOG(Info, Engine, "Initialize CubeEngine.");

        if (!mCommandLineArgs.empty())
        {
            FrameAnsiString params;
            for (const auto& [key, value] : mCommandLineArgs)
            {
                params += key;
                params += "=";
                params += value;
                params += " ";
            }
            CUBE_LOG(Info, Engine, "Command line parameters: {0}", params);
        }

        SearchAndSetRootDirectory();
        SetOtherDirectories();

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

        GetMyThreadFrameAllocator().Shutdown();

        platform::Platform::Shutdown();
    }

    void Engine::SetMesh(SharedPtr<MeshData> mesh, const MeshMetadata& meshMeta)
    {
        mRenderer->SetMesh(mesh, meshMeta);
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

        mRenderer->RenderAndPresent();

        mLoopCount++;
        if (platform::Debug::IsTestMode() && mLoopCount >= 20)
        {
            CUBE_LOG(Info, Engine, "Test mode: Auto-closing after {0} rendering loops.", mLoopCount);
            platform::Platform::TriggerClose();
        }
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
        platform::FilePath path = platform::FileSystem::GetCurrentDirectoryPath();

        // Find the CubeEngine directory
        while (1)
        {
            Vector<String> list = platform::FileSystem::GetList(path);
            if (std::find(list.begin(), list.end(), CUBE_T("Resources")) != list.end())
            {
                mRootDirectoryPath = path;
                CUBE_LOG(Info, Engine, "Found the root directory path: {0}", mRootDirectoryPath.ToString());
                return;
            }

            platform::FilePath parent = path.GetParent();
            if (parent.IsEmpty())
            {
                mRootDirectoryPath = platform::FileSystem::GetCurrentDirectoryPath();
                CUBE_LOG(Error, Engine, "Failed to find the root directory path. Use the current directory path: {0}", mRootDirectoryPath.ToString());
                break;
            }
            path = parent;
        }
    }

    void Engine::SetOtherDirectories()
    {
        mShaderDirectoryPath = mRootDirectoryPath / CUBE_T("Resources/Shaders");
    }

    void Engine::ParseCommandLineArgs(int argc, const char** argv)
    {
        if (argv == nullptr || argc <= 1)
        {
            return;
        }

        for (int i = 1; i < argc; ++i)
        {
            AnsiStringView arg(argv[i]);

            // Skip args that don't start with '-'
            if (arg.empty() || arg[0] != '-')
            {
                continue;
            }

            // Strip leading dashes
            SizeType dashEnd = arg.find_first_not_of('-');
            if (dashEnd == AnsiStringView::npos)
            {
                continue;
            }
            AnsiStringView keyValue = arg.substr(dashEnd);

            // Split on '=' for value
            SizeType eqPos = keyValue.find('=');
            if (eqPos != AnsiStringView::npos)
            {
                AnsiString key(keyValue.substr(0, eqPos));
                AnsiString value(keyValue.substr(eqPos + 1));
                mCommandLineArgs[key] = value;
            }
            else
            {
                AnsiString key(keyValue);
                mCommandLineArgs[key] = "1";
            }
        }
    }

    AnsiStringView Engine::GetCommandLineParam(AnsiStringView name)
    {
        AnsiString key(name);
        auto it = mCommandLineArgs.find(key);
        if (it != mCommandLineArgs.end())
        {
            return AnsiStringView(it->second);
        }
        return AnsiStringView();
    }
} // namespace cube

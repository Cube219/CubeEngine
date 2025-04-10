#pragma once

#include "CoreHeader.h"

#include "Event.h"
#include "GAPI.h"

namespace cube
{
    class Renderer;

    class Engine
    {
    public:
        Engine() = delete;
        ~Engine() = delete;

        struct EngineInitializeInfo
        {
            int argc;
            const char** argv;
            GAPIName gapi;
            bool runShutdownInOnClosingFunc = false;
        };
        CUBE_CORE_EXPORT static void Initialize(const EngineInitializeInfo& initInfo);
        CUBE_CORE_EXPORT static void Shutdown();

        static Renderer* GetRenderer() { return mRenderer.get(); }

        static const String& GetRootDirectoryPath() { return mRootDirectoryPath; }

    private:
        static void OnLoop();
        static void OnClosing();
        static void OnResize(Uint32 width, Uint32 height);

        static void LoopImGUI();

        static Uint64 GetNowFrameTime();

        static EventFunction<void()> mOnLoopEventFunc;
        static EventFunction<void()> mOnClosingEventFunc;
        static EventFunction<void(Uint32, Uint32)> mOnResizeEventFunc;
        static bool mRunShutdownInClosingFunc;

        static UniquePtr<Renderer> mRenderer;

        static ImGUIContext mImGUIContext;
        static bool mImGUIShowDemoWindow;

        static String mRootDirectoryPath;

        static Uint64 mStartTime;
        static Uint64 mLastTime;
        static Uint64 mCurrentTime;
    };
} // namespace cube

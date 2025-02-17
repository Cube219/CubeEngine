#pragma once

#include "MainHeader.h"

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

        static void Initialize();
        static void Shutdown();

    private:
        static void OnLoop();
        static void OnClosing();
        static void OnResize(Uint32 width, Uint32 height);

        static void LoopImGUI();

        static EventFunction<void()> mOnLoopEventFunc;
        static EventFunction<void()> mOnClosingEventFunc;
        static EventFunction<void(Uint32, Uint32)> mOnResizeEventFunc;

        static UniquePtr<Renderer> mRenderer;

        static ImGUIContext mImGUIContext;
        static bool mImGUIShowDemoWindow;
    };
} // namespace cube

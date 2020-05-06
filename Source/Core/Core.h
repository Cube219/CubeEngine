#pragma once

#include "CoreHeader.h"

namespace cube
{
    class CORE_EXPORT Core
    {
    public:
        Core() = delete;
        ~Core() = delete;

        Core(const Core& other) = delete;
        Core& operator=(const Core& rhs) = delete;

        static void PreInitialize();
        static void Initialize();
        static void Shutdown();

        static void Start();

        static float GetFPS();
        static void SetFPSLimit(Uint64 fps);

    private:
        friend class GameThread;

        static void OnUpdate();
        static void OnResize(Uint32 width, Uint32 height);

        static Uint64 mFPSLimit;
    };
} // namespace cube

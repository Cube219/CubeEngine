#pragma once

#include "CoreHeader.h"

namespace cube
{
    CORE_EXPORT Core& GetCore();

    class CORE_EXPORT Core
    {
    public:
        Core() : mFPSLimit(0) {}
        ~Core() {}

        Core(const Core& other) = delete;
        Core& operator=(const Core& rhs) = delete;

        void PreInitialize();
        void Initialize();
        void Shutdown();

        void Start();

        float GetFPS();
        void SetFPSLimit(Uint64 fps);

    private:
        friend class GameThread;

        void OnUpdate();
        void OnResize(Uint32 width, Uint32 height);

        Uint64 mFPSLimit;
    };
} // namespace cube

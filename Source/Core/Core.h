#pragma once

#include "CoreHeader.h"

#include <entt/entt.hpp>

#include "World/World.h"

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

        static World& GetWorld() { return mWorld; }

    private:
        friend class GameThread;

        static void OnUpdate();
        static void OnResize(Uint32 width, Uint32 height);

        static Uint64 mFPSLimit;
        static entt::registry mRegistry;

        static World mWorld;
    };
} // namespace cube

#pragma once

#include "EngineHeader.h"
#include "Utility/Event.h"

namespace cube
{
    struct ENGINE_EXPORT EngineInitOption
    {
        bool isWindowMode = false;
        Uint32 windowWidth;
        Uint32 windowHeight;
        Uint32 windowPositionX = 100;
        Uint32 windowPositionY = 100;
        String title;
    };

    class ENGINE_EXPORT Engine
    {
    public:
        Engine() = delete;
        ~Engine() = delete;

        static void Initialize(const EngineInitOption& initOption);
        static void Shutdown();

        static void Run();

        static void Close();

        static void SetCustomClosingFunction(std::function<void()> func);

    private:
        static void DefaultClosingFunction();

        static EventFunction<void()> closingEventFunc;
    };
} // namespace cube

#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "Platform.h"

#include "MacOSString.h"

namespace cube
{
    namespace platform
    {
        class MacOSPlatform : public Platform
        {
        public:
            static void InitializeImpl();
            static void ShutdownImpl();

            static void InitWindowImpl(StringView title, Uint32 width, Uint32 height, Uint32 posX, Uint32 posY);
            static void ShowWindowImpl();

            static void* AllocateImpl(Uint64 size);
            static void FreeImpl(void* ptr);
            static void* AllocateAlignedImpl(Uint64 size, Uint64 alignment);
            static void FreeAlignedImpl(void* ptr);

            static void StartLoopImpl();
            static void FinishLoopImpl();
            static void SleepImpl(Uint32 time);

            static void ShowCursorImpl();
            static void HideCursorImpl();
            static void MoveCursorImpl(int x, int y);
            static void GetCursorPosImpl(int& x, int& y);

            static Uint32 GetWindowWidthImpl();
            static Uint32 GetWindowHeightImpl();
            static Uint32 GetWindowPositionXImpl();
            static Uint32 GetWindowPositionYImpl();

            static SharedPtr<DLib> LoadDLibImpl(StringView path);

        private:
            MacOSPlatform() = delete;
            ~MacOSPlatform() = delete;
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS

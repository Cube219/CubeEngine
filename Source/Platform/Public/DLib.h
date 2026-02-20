#pragma once

#include "PlatformHeader.h"

#include "Checker.h"

namespace cube
{
    namespace platform
    {
        class CUBE_PLATFORM_EXPORT BaseDLib
        {
        public:
            BaseDLib() = default;
            ~BaseDLib() = default;

            void* GetFunction(StringView name) { NOT_IMPLEMENTED() return nullptr; }
        };
    } // namespace platform
} // namespace cube

#if defined(CUBE_PLATFORM_MACOS)
#include "MacOS/MacOSDLib.h"
#elif defined(CUBE_PLATFORM_WINDOWS)
#include "Windows/WindowsDLib.h"
#endif

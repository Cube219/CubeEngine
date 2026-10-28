#pragma once

#include "CoreHeader.h"

#include "Checker.h"

namespace cube
{
    namespace platform
    {
        class CUBE_CORE_EXPORT BaseDLib
        {
        public:
            BaseDLib() = default;
            ~BaseDLib() = default;

            void* GetFunction(StringView name) { NOT_IMPLEMENTED() return nullptr; }
        };
    } // namespace platform
} // namespace cube

#if defined(CUBE_PLATFORM_MACOS)
#include "Platform/MacOS/MacOSDLib.h"
#elif defined(CUBE_PLATFORM_WINDOWS)
#include "Platform/Windows/WindowsDLib.h"
#endif

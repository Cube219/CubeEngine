#pragma once

#ifdef CUBE_PLATFORM_WINDOWS

#include "PlatformHeader.h"
#include "DLib.h"
#include "FileSystem.h"

#include <Windows.h>

namespace cube
{
    namespace platform
    {
        class CUBE_PLATFORM_EXPORT WindowsDLib : public BaseDLib
        {
            // === Base member functions ===
        public:
            void* GetFunction(StringView name);
            // === Base member functions ===

        public:
            WindowsDLib(const FilePath& path);
            ~WindowsDLib();

            HMODULE GetModule() const { return mDLib; }

        private:
            HMODULE mDLib;
        };
        using DLib = WindowsDLib;
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS

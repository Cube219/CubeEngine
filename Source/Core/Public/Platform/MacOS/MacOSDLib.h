#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "CoreHeader.h"
#include "Platform/DLib.h"
#include "Platform/FileSystem.h"

namespace cube
{
    namespace platform
    {
        class MacOSDLib : public BaseDLib
        {
            // === Base member functions ===
        public:
            void* GetFunction(StringView name);
            // === Base member functions ===

#ifdef __OBJC__
        public:
            MacOSDLib(const FilePath& path);
            ~MacOSDLib();

            void* GetHandle() const { return mHandle; }

        private:
            void* mHandle;
#endif // __OBJC__
        };
        using DLib = MacOSDLib;
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS

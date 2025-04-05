#pragma once

#include "PlatformHeader.h"

#include "CubeString.h"

namespace cube
{
    namespace platform
    {
        class CUBE_PLATFORM_EXPORT DLib
        {
        public:
            DLib() = default;
            ~DLib() = default;

            void* GetFunction(StringView name);
        };

#define DLIB_CLASS_DEFINITIONS(ChildClass) \
        void* DLib::GetFunction(StringView name) { return reinterpret_cast<ChildClass*>(this)->GetFunctionImpl(name); }
    } // namespace platform
} // namespace cube

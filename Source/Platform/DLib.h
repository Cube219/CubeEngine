#pragma once

#include "PlatformHeader.h"

namespace cube
{
    namespace platform
    {
        class PLATFORM_EXPORT DLib
        {
        public:
            DLib() {}
            virtual ~DLib() {}

            void* GetFunction(StringView name);
        };

#define DLIB_DEFINITIONS(Child)                                \
        inline void* DLib::GetFunction(StringView name) {      \
            return RCast(Child*)(this)->GetFunctionImpl(name); \
        }
    } // namespace platform
} // namespace cube

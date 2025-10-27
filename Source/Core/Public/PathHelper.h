#pragma once

#include "CoreHeader.h"

namespace cube
{
    class PathHelper
    {
    public:
        PathHelper() = delete;
        ~PathHelper() = delete;

        static void Initialize();
        static void Shutdown();

        static CUBE_CORE_EXPORT StringView GetFileNameFromPath(StringView path);
        static CUBE_CORE_EXPORT StringView GetExtension(StringView path);

    private:
        static Character mPathSeperator;
    };
} // namespace cube

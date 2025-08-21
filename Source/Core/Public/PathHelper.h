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

        static StringView GetFileNameFromPath(StringView path);

    private:
        static Character mPathSeperator;
    };
} // namespace cube

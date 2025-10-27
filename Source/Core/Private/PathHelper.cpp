#include "PathHelper.h"

#include "FileSystem.h"

namespace cube
{
    Character PathHelper::mPathSeperator;

    void PathHelper::Initialize()
    {
        mPathSeperator = platform::FileSystem::GetSeparator();
    }

    void PathHelper::Shutdown()
    {
    }

    StringView PathHelper::GetFileNameFromPath(StringView path)
    {
        int separatorIndex = path.size() - 1;
        while (separatorIndex >= 0 && path[separatorIndex] != mPathSeperator && path[separatorIndex] != CUBE_T('/'))
        {
            separatorIndex--;
        }

        return path.substr(separatorIndex + 1);
    }

    StringView PathHelper::GetExtension(StringView path)
    {
        for (int i = (int)path.size() - 1; i >= 0; --i)
        {
            if (path[i] == CUBE_T('.'))
            {
                return path.substr(i + 1);
            }
        }

        return StringView();
    }
} // namespace cube

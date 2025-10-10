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
} // namespace cube

#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSFileSystem.h"

#include "Checker.h"

namespace cube
{
    namespace platform
    {
        FILE_CLASS_DEFINITIONS(MacOSFile)

        Uint64 MacOSFile::GetFileSizeImpl() const
        {
            NOT_IMPLEMENTED();
            return 0;
        }

        void MacOSFile::SetFilePointerImpl(Uint64 offset)
        {
            NOT_IMPLEMENTED();
        }

        void MacOSFile::MoveFilePointerImpl(Int64 distance)
        {
            NOT_IMPLEMENTED();
        }

        Uint64 MacOSFile::ReadImpl(void* pReadBuffer, Uint64 bufferSizeToRead)
        {
            NOT_IMPLEMENTED();
            return 0;
        }

        void MacOSFile::WriteImpl(void* pWriteBuffer, Uint64 bufferSize)
        {
            NOT_IMPLEMENTED();
        }

        MacOSFile::MacOSFile()
        {
            NOT_IMPLEMENTED();
        }

        MacOSFile::~MacOSFile()
        {
            NOT_IMPLEMENTED();
        }

        FILE_SYSTEM_CLASS_DEFINITIONS(MacOSFileSystem)

        SharedPtr<File> MacOSFileSystem::OpenFileImpl(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist)
        {
            NOT_IMPLEMENTED();
            return nullptr;
        }

        char MacOSFileSystem::GetSeparatorImpl()
        {
            return '/';
        }

        const char* MacOSFileSystem::SplitFileNameFromFullPathImpl(const char* fullPath)
        {
            const char* lastSeparator = strrchr(fullPath, GetSeparatorImpl());

            if (lastSeparator != nullptr)
            {
                return lastSeparator + 1;
            }

            return fullPath;
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS

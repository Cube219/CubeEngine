#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "FileSystem.h"

namespace cube
{
    namespace platform
    {
        class MacOSFile : public File
        {
        public:
            MacOSFile();
            ~MacOSFile();

            Uint64 GetFileSizeImpl() const;

            void SetFilePointerImpl(Uint64 offset);
            void MoveFilePointerImpl(Int64 distance);

            Uint64 ReadImpl(void* pReadBuffer, Uint64 bufferSizeToRead);
            void WriteImpl(void* pWriteBuffer, Uint64 bufferSize);

        private:
        };

        class MacOSFileSystem : public FileSystem
        {
        public:
            MacOSFileSystem() = delete;
            ~MacOSFileSystem() = delete;

            static SharedPtr<File> OpenFileImpl(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist = false);
            static char GetSeparatorImpl();
            static const char* SplitFileNameFromFullPathImpl(const char* fullPath);

        private:
        };
    }
}

#endif // CUBE_PLATFORM_MACOS

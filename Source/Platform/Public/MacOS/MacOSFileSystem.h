#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "FileSystem.h"

#include <Foundation/Foundation.h>

namespace cube
{
    namespace platform
    {
        class MacOSFile : public File
        {
        public:
            MacOSFile(NSFileHandle* fileHandle);
            ~MacOSFile();

            Uint64 GetFileSizeImpl() const;

            void SetFilePointerImpl(Uint64 offset);
            void MoveFilePointerImpl(Int64 distance);

            Uint64 ReadImpl(void* pReadBuffer, Uint64 bufferSizeToRead);
            void WriteImpl(void* pWriteBuffer, Uint64 bufferSize);

        private:
            NSFileHandle* mFileHandle;
            Uint64 mCurrentOffset;
            Uint64 mSize;
        };

        class MacOSFileSystem : public FileSystem
        {
        public:
            MacOSFileSystem() = delete;
            ~MacOSFileSystem() = delete;

            static bool IsExistImpl(StringView path);
            static bool IsDirectoryImpl(StringView path);
            static bool IsFileImpl(StringView path);
            static Vector<String> GetListImpl(StringView directoryPath);

            static String GetCurrentDirectoryPathImpl();
            static Character GetSeparatorImpl();

            static SharedPtr<File> OpenFileImpl(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist = false);
            static const char* SplitFileNameFromFullPathImpl(const char* fullPath);
        };
    }
}

#endif // CUBE_PLATFORM_MACOS

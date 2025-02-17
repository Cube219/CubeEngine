#pragma once

#ifdef CUBE_PLATFORM_WINDOWS

#include "PlatformHeader.h"
#include "FileSystem.h"

#include <Windows.h>

namespace cube
{
    namespace platform
    {
        class WindowsFile : public File
        {
        public:
            WindowsFile(HANDLE fileHandle);
            ~WindowsFile();

            Uint64 GetFileSizeImpl() const;

            void SetFilePointerImpl(Uint64 offset);
            void MoveFilePointerImpl(Int64 distance);

            Uint64 ReadImpl(void* pReadBuffer, Uint64 bufferSizeToRead);
            void WriteImpl(void* pWriteBuffer, Uint64 bufferSize);

        private:
            HANDLE mFileHandle;
        };
        FILE_CLASS_DEFINITIONS(WindowsFile)

        class WindowsFileSystem : public FileSystem
        {
        public:
            WindowsFileSystem() = delete;
            ~WindowsFileSystem() = delete;

            static SharedPtr<File> OpenFileImpl(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist = false);
            static char GetSeparatorImpl();
            static const char* SplitFileNameFromFullPathImpl(const char* fullPath);

        private:
            static DWORD GetDwDesiredAccess(FileAccessModeFlags accessModeFlags);
        };
        FILE_SYSTEM_CLASS_DEFINITIONS(WindowsFileSystem)
    }
}

#endif // CUBE_PLATFORM_WINDOWS

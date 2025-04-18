#pragma once

#ifdef CUBE_PLATFORM_WINDOWS

#include "FileSystem.h"
#include "PlatformHeader.h"

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

        class WindowsFileSystem : public FileSystem
        {
        public:
            WindowsFileSystem() = delete;
            ~WindowsFileSystem() = delete;

            static bool IsExistImpl(StringView path);
            static bool IsDirectoryImpl(StringView path);
            static bool IsFileImpl(StringView path);
            static Vector<String> GetListImpl(StringView directoryPath);

            static String GetCurrentDirectoryPathImpl();
            static Character GetSeparatorImpl();

            static SharedPtr<File> OpenFileImpl(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist = false);
            static const char* SplitFileNameFromFullPathImpl(const char* fullPath);

        private:
            static DWORD GetDwDesiredAccess(FileAccessModeFlags accessModeFlags);
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS

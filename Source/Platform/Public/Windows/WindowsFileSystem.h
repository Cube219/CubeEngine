#pragma once

#ifdef CUBE_PLATFORM_WINDOWS

#include "PlatformHeader.h"
#include "FileSystem.h"

#include <Windows.h>

namespace cube
{
    namespace platform
    {
        class CUBE_PLATFORM_EXPORT WindowsFile : public BaseFile
        {
            // === Base member functions ===
        public:
            Uint64 GetFileSize() const;
            Time GetWriteTime() const;

            void SetFilePointer(Uint64 offset);
            void MoveFilePointer(Int64 distance);

            Uint64 Read(void* pReadBuffer, Uint64 bufferSizeToRead);
            void Write(void* pWriteBuffer, Uint64 bufferSize);
            // === Base member functions ===

        public:
            WindowsFile(HANDLE fileHandle);
            ~WindowsFile();

        private:
            HANDLE mFileHandle;
        };

        class CUBE_PLATFORM_EXPORT WindowsFileSystem : public BaseFileSystem
        {
            // === Base member functions ===
        public:
            static bool IsExist(StringView path);
            static bool IsDirectory(StringView path);
            static bool IsFile(StringView path);
            static Vector<String> GetList(StringView directoryPath);

            static String GetCurrentDirectoryPath();
            static Character GetSeparator();

            static SharedPtr<WindowsFile> OpenFile(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist = false);
            static const char* SplitFileNameFromFullPath(const char* fullPath);
            // === Base member functions ===

        private:
            static DWORD GetDwDesiredAccess(FileAccessModeFlags accessModeFlags);

        public:
            WindowsFileSystem() = delete;
            ~WindowsFileSystem() = delete;
        };
        using File = WindowsFile;
        using FileSystem = WindowsFileSystem;
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS

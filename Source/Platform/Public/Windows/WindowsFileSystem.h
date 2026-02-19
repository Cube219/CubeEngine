#pragma once

#ifdef CUBE_PLATFORM_WINDOWS

#include "PlatformHeader.h"
#include "FileSystem.h"
#include "Windows/WindowsString.h"

#include <Windows.h>

namespace cube
{
    namespace platform
    {
        class CUBE_PLATFORM_EXPORT WindowsFilePath : public BaseFilePath
        {
            // === Base member functions ===
        public:
            WindowsFilePath() = default;
            ~WindowsFilePath() = default;

            WindowsFilePath(StringView path);
            WindowsFilePath(AnsiStringView path);

            String ToString() const;
            AnsiString ToAnsiString() const;

            WindowsFilePath GetParent() const;
            String GetFileName() const;
            String GetExtension() const;
            String GetStem() const;
            bool IsEmpty() const;

            WindowsFilePath operator/(const WindowsFilePath& rhs) const;
            WindowsFilePath& operator/=(const WindowsFilePath& rhs);
            WindowsFilePath operator/(StringView rhs) const;
            WindowsFilePath& operator/=(StringView rhs);
            WindowsFilePath operator/(AnsiStringView rhs) const;
            WindowsFilePath& operator/=(AnsiStringView rhs);

            bool operator==(const WindowsFilePath& rhs) const;
            // === Base member functions ===

        public:
            WindowsFilePath(WindowsStringView nativePath);

            WindowsStringView GetNativePath() const;

        private:
            WindowsString mPath;
        };
        using FilePath = WindowsFilePath;

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
            static bool IsExist(const FilePath& path);
            static bool IsDirectory(const FilePath& path);
            static bool IsFile(const FilePath& path);
            static Vector<String> GetList(const FilePath& directoryPath);

            static FilePath GetCurrentDirectoryPath();
            static Character GetSeparator();

            static SharedPtr<WindowsFile> OpenFile(const FilePath& path, FileAccessModeFlags accessModeFlags, bool createIfNotExist = false);
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

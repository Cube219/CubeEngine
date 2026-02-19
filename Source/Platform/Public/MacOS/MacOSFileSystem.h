#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"
#include "FileSystem.h"

#ifdef __OBJC__
#include <Foundation/Foundation.h>
#endif // __OBJC__

namespace cube
{
    namespace platform
    {
        class MacOSFilePath : public BaseFilePath
        {
            // === Base member functions ===
        public:
            MacOSFilePath();
            ~MacOSFilePath();

            MacOSFilePath(const MacOSFilePath& other);
            MacOSFilePath(MacOSFilePath&& other);
            MacOSFilePath& operator=(const MacOSFilePath& other);
            MacOSFilePath& operator=(MacOSFilePath&& other);

            MacOSFilePath(StringView path);
            MacOSFilePath(AnsiStringView path);

            String ToString() const;
            AnsiString ToAnsiString() const;

            MacOSFilePath GetParent() const;
            String GetFileName() const;
            String GetExtension() const;
            String GetStem() const;
            bool IsEmpty() const;

            MacOSFilePath operator/(const MacOSFilePath& rhs) const;
            MacOSFilePath& operator/=(const MacOSFilePath& rhs);
            MacOSFilePath operator/(StringView rhs) const;
            MacOSFilePath& operator/=(StringView rhs);
            MacOSFilePath operator/(AnsiStringView rhs) const;
            MacOSFilePath& operator/=(AnsiStringView rhs);

            bool operator==(const MacOSFilePath& rhs) const;
            // === Base member functions ===

#ifdef __OBJC__
        public:
            MacOSFilePath(NSString* nativePath);

            NSString* GetNativePath() const;

        private:
            NSString* mPath;
#else
        private:
            void* mPath = nullptr;
#endif // __OBJC__
        };
        using FilePath = MacOSFilePath;

        class MacOSFile : public BaseFile
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

#ifdef __OBJC__
        public:
            MacOSFile(NSString* filePath, NSFileHandle* fileHandle);
            ~MacOSFile();

        private:
            NSString* mFilePath;
            NSFileHandle* mFileHandle;
            Uint64 mCurrentOffset;
            Uint64 mSize;
#endif // __OBJC__
        };

        class MacOSFileSystem : public BaseFileSystem
        {
            // === Base member functions ===
        public:
            static bool IsExist(const FilePath& path);
            static bool IsDirectory(const FilePath& path);
            static bool IsFile(const FilePath& path);
            static Vector<String> GetList(const FilePath& directoryPath);

            static FilePath GetCurrentDirectoryPath();
            static Character GetSeparator();

            static SharedPtr<MacOSFile> OpenFile(const FilePath& path, FileAccessModeFlags accessModeFlags, bool createIfNotExist = false);
            static const char* SplitFileNameFromFullPath(const char* fullPath);
            // === Base member functions ===

            MacOSFileSystem() = delete;
            ~MacOSFileSystem() = delete;
        };
        using File = MacOSFile;
        using FileSystem = MacOSFileSystem;
    }
}

#endif // CUBE_PLATFORM_MACOS

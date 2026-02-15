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
            static bool IsExist(StringView path);
            static bool IsDirectory(StringView path);
            static bool IsFile(StringView path);
            static Vector<String> GetList(StringView directoryPath);

            static String GetCurrentDirectoryPath();
            static Character GetSeparator();

            static SharedPtr<MacOSFile> OpenFile(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist = false);
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

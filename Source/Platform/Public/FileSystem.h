#pragma once

#include "PlatformHeader.h"

#include "Checker.h"
#include "Flags.h"

namespace cube
{
    namespace platform
    {
        class CUBE_PLATFORM_EXPORT BaseFilePath
        {
        public:
            BaseFilePath() = default;
            ~BaseFilePath() = default;

            BaseFilePath(StringView path) { NOT_IMPLEMENTED() }
            BaseFilePath(AnsiStringView path) { NOT_IMPLEMENTED() }

            String ToString() const { NOT_IMPLEMENTED() return {}; }
            AnsiString ToAnsiString() const { NOT_IMPLEMENTED() return {}; }

            BaseFilePath GetParent() const { NOT_IMPLEMENTED() return {}; }
            String GetFileName() const { NOT_IMPLEMENTED() return {}; }
            String GetExtension() const { NOT_IMPLEMENTED() return {}; }
            String GetStem() const { NOT_IMPLEMENTED() return {}; }
            bool IsEmpty() const { NOT_IMPLEMENTED() return true; }

            BaseFilePath operator/(const BaseFilePath& rhs) const { NOT_IMPLEMENTED() return {}; }
            BaseFilePath& operator/=(const BaseFilePath& rhs) { NOT_IMPLEMENTED() return *this; }
            BaseFilePath operator/(StringView rhs) const { NOT_IMPLEMENTED() return {}; }
            BaseFilePath& operator/=(StringView rhs) { NOT_IMPLEMENTED() return *this; }
            BaseFilePath operator/(AnsiStringView rhs) const { NOT_IMPLEMENTED() return {}; }
            BaseFilePath& operator/=(AnsiStringView rhs) { NOT_IMPLEMENTED() return *this; }

            bool operator==(const BaseFilePath& rhs) const { NOT_IMPLEMENTED() return false; }
        };

        enum class FileAccessModeFlag
        {
            Read = 1,
            Write = 2
        };
        using FileAccessModeFlags = Flags<FileAccessModeFlag>;
        FLAGS_OPERATOR(FileAccessModeFlag);

        class CUBE_PLATFORM_EXPORT BaseFile
        {
        public:
            BaseFile() = default;
            ~BaseFile() = default;

            Uint64 GetFileSize() const { NOT_IMPLEMENTED() return 0; }
            Time GetWriteTime() const { NOT_IMPLEMENTED() return {}; }

            void SetFilePointer(Uint64 offset) { NOT_IMPLEMENTED() }
            void MoveFilePointer(Int64 distance) { NOT_IMPLEMENTED() }

            Uint64 Read(void* pReadBuffer, Uint64 bufferSizeToRead) { NOT_IMPLEMENTED() return 0; }
            void Write(void* pWriteBuffer, Uint64 bufferSize) { NOT_IMPLEMENTED() }

        protected:
            friend class BaseFileSystem;
        };

        class CUBE_PLATFORM_EXPORT BaseFileSystem
        {
        public:
            BaseFileSystem() = delete;
            ~BaseFileSystem() = delete;

            static bool IsExist(const BaseFilePath& path) { NOT_IMPLEMENTED() return false; }
            static bool IsDirectory(const BaseFilePath& path) { NOT_IMPLEMENTED() return false; }
            static bool IsFile(const BaseFilePath& path) { NOT_IMPLEMENTED() return false; }
            static Vector<String> GetList(const BaseFilePath& directoryPath) { NOT_IMPLEMENTED() return {}; }

            static BaseFilePath GetCurrentDirectoryPath() { NOT_IMPLEMENTED() return {}; }
            static Character GetSeparator() { NOT_IMPLEMENTED() return {}; }

            static SharedPtr<BaseFile> OpenFile(const BaseFilePath& path, FileAccessModeFlags accessModeFlags, bool createIfNotExist = false) { NOT_IMPLEMENTED() return nullptr; }
            static const char* SplitFileNameFromFullPath(const char* fullPath) { NOT_IMPLEMENTED() return nullptr; }
        };
    } // namespace platform
} // namespace cube

#if defined(CUBE_PLATFORM_MACOS)
#include "MacOS/MacOSFileSystem.h"
#elif defined(CUBE_PLATFORM_WINDOWS)
#include "Windows/WindowsFileSystem.h"
#endif

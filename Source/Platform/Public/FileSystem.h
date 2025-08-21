#pragma once

#include "PlatformHeader.h"

#include "Flags.h"

namespace cube
{
    namespace platform
    {
        enum class FileAccessModeFlag
        {
            Read = 1,
            Write = 2
        };
        using FileAccessModeFlags = Flags<FileAccessModeFlag>;
        FLAGS_OPERATOR(FileAccessModeFlag);

        class CUBE_PLATFORM_EXPORT File
        {
        public:
            File() = default;
            ~File() = default;

            Uint64 GetFileSize() const;
            Time GetWriteTime() const;

            void SetFilePointer(Uint64 offset);
            void MoveFilePointer(Int64 distance);

            Uint64 Read(void* pReadBuffer, Uint64 bufferSizeToRead);
            void Write(void* pWriteBuffer, Uint64 bufferSize);

        protected:
            friend class FileSystem;
        };

#define FILE_CLASS_DEFINITIONS(ChildClass) \
        Uint64 File::GetFileSize() const { return reinterpret_cast<const ChildClass*>(this)->GetFileSizeImpl(); } \
        Time File::GetWriteTime() const { return reinterpret_cast<const ChildClass*>(this)->GetWriteTimeImpl(); } \
        \
        void File::SetFilePointer(Uint64 offset) { reinterpret_cast<ChildClass*>(this)->SetFilePointerImpl(offset); } \
        void File::MoveFilePointer(Int64 distance) { reinterpret_cast<ChildClass*>(this)->MoveFilePointerImpl(distance); } \
        \
        Uint64 File::Read(void* pReadBuffer, Uint64 bufferSizeToRead) { return reinterpret_cast<ChildClass*>(this)->ReadImpl(pReadBuffer, bufferSizeToRead); } \
        void File::Write(void* pWriteBuffer, Uint64 bufferSize) { return reinterpret_cast<ChildClass*>(this)->WriteImpl(pWriteBuffer, bufferSize); }

        class CUBE_PLATFORM_EXPORT FileSystem
        {
        public:
            FileSystem() = delete;
            ~FileSystem() = delete;

            static bool IsExist(StringView path);
            static bool IsDirectory(StringView path);
            static bool IsFile(StringView path);
            static Vector<String> GetList(StringView directoryPath);

            static String GetCurrentDirectoryPath();
            static Character GetSeparator();

            static SharedPtr<File> OpenFile(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist = false);
            static const char* SplitFileNameFromFullPath(const char* fullPath);
        };

#define FILE_SYSTEM_CLASS_DEFINITIONS(ChildClass) \
        bool FileSystem::IsExist(StringView path) { return ChildClass::IsExistImpl(path); } \
        bool FileSystem::IsDirectory(StringView path) { return ChildClass::IsDirectoryImpl(path); } \
        bool FileSystem::IsFile(StringView path) { return ChildClass::IsFileImpl(path); } \
        Vector<String> FileSystem::GetList(StringView directoryPath) { return ChildClass::GetListImpl(directoryPath); } \
        \
        String FileSystem::GetCurrentDirectoryPath() { return ChildClass::GetCurrentDirectoryPathImpl(); } \
        Character FileSystem::GetSeparator() { return ChildClass::GetSeparatorImpl(); } \
        \
        SharedPtr<File> FileSystem::OpenFile(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist) { \
            return ChildClass::OpenFileImpl(path, accessModeFlags, createIfNotExist); \
        } \
        const char* FileSystem::SplitFileNameFromFullPath(const char* fullPath) { \
            return ChildClass::SplitFileNameFromFullPathImpl(fullPath); \
        }
    } // namespace platform
} // namespace cube

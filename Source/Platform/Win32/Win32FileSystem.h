#pragma once
#pragma once

#ifdef _WIN32

#include "../PlatformHeader.h"

#include <Windows.h>

#include "../FileSystem.h"

namespace cube
{
    namespace platform
    {
        //////////
        // File //
        //////////

        class PLATFORM_EXPORT Win32File : public File
        {
        public:
            Win32File(HANDLE fileHandle);
            virtual ~Win32File();

            Uint64 GetFileSizeImpl();

            void SetFilePointerImpl(Uint64 offset);
            void MoveFilePointerImpl(Int64 distance);

            Uint64 ReadImpl(void* pReadBuffer, Uint64 bufferSizeToRead);
            void WriteImpl(void* pWriteBuffer, Uint64 bufferSize);

        private:
            HANDLE mFileHandle;
        };
        FILE_DEFINITIONS(Win32File)

        ////////////////
        // FileSystem //
        ////////////////

        class Win32FileSystem : public FileSystem
        {
        public:
            Win32FileSystem() = delete;
            ~Win32FileSystem() = delete;

            static SPtr<File> OpenFileImpl(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist = false);

        private:
            static DWORD GetDwDesiredAccess(FileAccessModeFlags accessModeFlags);
        };
        FILE_SYSTEM_DEFINITIONS(Win32FileSystem)
    }
}

#endif // _WIN32

#ifdef _WIN32

#include "Win32FileSystem.h"

#include <iostream>

#include "../PlatformDebug.h"
#include "../PlatformString.h"

namespace cube
{
    namespace platform
    {
        //////////
        // File //
        //////////
        Uint64 Win32File::GetFileSizeImpl()
        {
            LARGE_INTEGER size_LI;
            BOOL res = GetFileSizeEx(mFileHandle, &size_LI);
            PLATFORM_CHECK(res, "Failed to get file size. (ErrorCode: {0})", GetLastError());

            return size_LI.QuadPart;
        }

        void Win32File::SetFilePointerImpl(Uint64 offset)
        {
            LARGE_INTEGER distance_LI;
            distance_LI.QuadPart = offset;

            BOOL res = SetFilePointerEx(mFileHandle, distance_LI, NULL, FILE_BEGIN);
            PLATFORM_CHECK(res, "Failed to set file pointer. (ErrorCode: {0})", GetLastError());
        }

        void Win32File::MoveFilePointerImpl(Int64 distance)
        {
            LARGE_INTEGER distance_LI;
            distance_LI.QuadPart = distance;

            BOOL res = SetFilePointerEx(mFileHandle, distance_LI, NULL, FILE_CURRENT);
            PLATFORM_CHECK(res, "Failed to move file pointer. (ErrorCode: {0})", GetLastError());
        }

        Uint64 Win32File::ReadImpl(void* pReadBuffer, Uint64 bufferSizeToRead)
        {
            Uint64 readBufferSize;

            BOOL res = ReadFile(mFileHandle, pReadBuffer, (DWORD)bufferSizeToRead, (LPDWORD)&readBufferSize, NULL);
            PLATFORM_CHECK(res, "Failed to read the file. (ErrorCode: {0})", GetLastError());

            return readBufferSize;
        }

        void Win32File::WriteImpl(void* pWriteBuffer, Uint64 bufferSize)
        {
            DWORD writtenSize;
            BOOL res = WriteFile(mFileHandle, pWriteBuffer, (DWORD)bufferSize, &writtenSize, nullptr);
            PLATFORM_CHECK(res, "Failed to write the file. (ErrorCode: {0})", GetLastError());
        }

        Win32File::Win32File(HANDLE fileHandle) :
            mFileHandle(fileHandle)
        {}

        Win32File::~Win32File()
        {
            CloseHandle(mFileHandle);
        }

        ////////////////
        // FileSystem //
        ////////////////

        SPtr<File> Win32FileSystem::OpenFileImpl(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist)
        {
            DWORD desiredAccess = GetDwDesiredAccess(accessModeFlags);
            PString pPath;
            String_ConvertAndAppend(pPath, path);

            HANDLE file = CreateFile(pPath.c_str(), desiredAccess, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            if(file != INVALID_HANDLE_VALUE) {
                return std::make_shared<Win32File>(file);
            }

            DWORD err = GetLastError();

            if(err == ERROR_FILE_NOT_FOUND && createIfNotExist == true) {
                file = CreateFile(pPath.c_str(), desiredAccess, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

                if(file != INVALID_HANDLE_VALUE)
                    return std::make_shared<Win32File>(file);

                err = GetLastError();
            }

            PLATFORM_ASSERTION_FAILED("Failed to open a file. ({0}) (ErrorCode: {1})", path, err);
            return nullptr;
        }

        DWORD Win32FileSystem::GetDwDesiredAccess(FileAccessModeFlags accessModeFlags)
        {
            DWORD d = 0;

            if(accessModeFlags.IsSet(FileAccessModeFlag::Read))
                d |= GENERIC_READ;

            if(accessModeFlags.IsSet(FileAccessModeFlag::Write))
                d |= GENERIC_WRITE;

            return d;
        }
    } // namespace platform
} // namespace cube

#endif // _WIN32

#ifdef CUBE_PLATFORM_WINDOWS

#include "Windows/WindowsFileSystem.h"

#include <iostream>

#include "Checker.h"
#include "Windows/WindowsString.h"

namespace cube
{
    namespace platform
    {
        Uint64 WindowsFile::GetFileSizeImpl() const
        {
            LARGE_INTEGER size_LI;
            BOOL res = GetFileSizeEx(mFileHandle, &size_LI);
            CHECK_FORMAT(res, "Failed to get file size. (ErrorCode: {0})", GetLastError());

            return size_LI.QuadPart;
        }

        void WindowsFile::SetFilePointerImpl(Uint64 offset)
        {
            LARGE_INTEGER distance_LI;
            distance_LI.QuadPart = offset;

            BOOL res = SetFilePointerEx(mFileHandle, distance_LI, NULL, FILE_BEGIN);
            CHECK_FORMAT(res, "Failed to set file pointer. (ErrorCode: {0})", GetLastError());
        }

        void WindowsFile::MoveFilePointerImpl(Int64 distance)
        {
            LARGE_INTEGER distance_LI;
            distance_LI.QuadPart = distance;

            BOOL res = SetFilePointerEx(mFileHandle, distance_LI, NULL, FILE_CURRENT);
            CHECK_FORMAT(res, "Failed to move file pointer. (ErrorCode: {0})", GetLastError());
        }

        Uint64 WindowsFile::ReadImpl(void* pReadBuffer, Uint64 bufferSizeToRead)
        {
            Uint64 readBufferSize;

            BOOL res = ReadFile(mFileHandle, pReadBuffer, (DWORD)bufferSizeToRead, (LPDWORD)&readBufferSize, NULL);
            CHECK_FORMAT(res, "Failed to read the file. (ErrorCode: {0})", GetLastError());

            return readBufferSize;
        }

        void WindowsFile::WriteImpl(void* pWriteBuffer, Uint64 bufferSize)
        {
            DWORD writtenSize;
            BOOL res = WriteFile(mFileHandle, pWriteBuffer, (DWORD)bufferSize, &writtenSize, nullptr);
            CHECK_FORMAT(res, "Failed to write the file. (ErrorCode: {0})", GetLastError());
        }

        WindowsFile::WindowsFile(HANDLE fileHandle) :
            mFileHandle(fileHandle)
        {}

        WindowsFile::~WindowsFile()
        {
            CloseHandle(mFileHandle);
        }

        SharedPtr<File> WindowsFileSystem::OpenFileImpl(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist)
        {
            DWORD desiredAccess = GetDwDesiredAccess(accessModeFlags);
            WindowsString pPath;
            String_ConvertAndAppend(pPath, path);

            HANDLE file = CreateFile(pPath.c_str(), desiredAccess, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            if (file != INVALID_HANDLE_VALUE)
            {
                return std::make_shared<WindowsFile>(file);
            }

            DWORD err = GetLastError();

            if (err == ERROR_FILE_NOT_FOUND && createIfNotExist == true)
            {
                file = CreateFile(pPath.c_str(), desiredAccess, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

                if (file != INVALID_HANDLE_VALUE) return std::make_shared<WindowsFile>(file);

                err = GetLastError();
            }

            CHECK_FORMAT(false, "Failed to open a file. ({0}) (ErrorCode: {1})", path, err);
            return nullptr;
        }

        char WindowsFileSystem::GetSeparatorImpl()
        {
            return '\\';
        }

        const char* WindowsFileSystem::SplitFileNameFromFullPathImpl(const char* fullPath)
        {
            const char* lastSeparator = strrchr(fullPath, GetSeparatorImpl());

            if (lastSeparator != nullptr)
            {
                return lastSeparator + 1;
            }

            return fullPath;
        }

        DWORD WindowsFileSystem::GetDwDesiredAccess(FileAccessModeFlags accessModeFlags)
        {
            DWORD d = 0;

            if (accessModeFlags.IsSet(FileAccessModeFlag::Read)) d |= GENERIC_READ;

            if (accessModeFlags.IsSet(FileAccessModeFlag::Write)) d |= GENERIC_WRITE;

            return d;
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS

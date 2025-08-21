#ifdef CUBE_PLATFORM_WINDOWS

#include "Windows/WindowsFileSystem.h"

#include <iostream>

#include "Checker.h"
#include "Windows/WindowsString.h"

namespace cube
{
    namespace platform
    {
        FILE_CLASS_DEFINITIONS(WindowsFile)

        WindowsFile::WindowsFile(HANDLE fileHandle) :
            mFileHandle(fileHandle)
        {}

        WindowsFile::~WindowsFile()
        {
            CloseHandle(mFileHandle);
        }

        Uint64 WindowsFile::GetFileSizeImpl() const
        {
            LARGE_INTEGER size_LI;
            BOOL res = GetFileSizeEx(mFileHandle, &size_LI);
            CHECK_FORMAT(res, "Failed to get file size. (ErrorCode: {0})", GetLastError());

            return size_LI.QuadPart;
        }

        Time WindowsFile::GetWriteTimeImpl() const
        {
            FILETIME writeTime;

            BOOL res = GetFileTime(mFileHandle, NULL, NULL, &writeTime);
            CHECK_FORMAT(res, "Failed to get file write time. (ErrorCode: {0})", GetLastError());

            Uint64 low = writeTime.dwLowDateTime;
            Uint64 high = writeTime.dwHighDateTime;
            return (high << (sizeof(DWORD) * 8)) | low;
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
            DWORD readBufferSize;

            BOOL res = ReadFile(mFileHandle, pReadBuffer, (DWORD)bufferSizeToRead, &readBufferSize, NULL);
            CHECK_FORMAT(res, "Failed to read the file. (ErrorCode: {0})", GetLastError());

            return readBufferSize;
        }

        void WindowsFile::WriteImpl(void* pWriteBuffer, Uint64 bufferSize)
        {
            DWORD writtenSize;
            BOOL res = WriteFile(mFileHandle, pWriteBuffer, (DWORD)bufferSize, &writtenSize, nullptr);
            CHECK_FORMAT(res, "Failed to write the file. (ErrorCode: {0})", GetLastError());
        }

        FILE_SYSTEM_CLASS_DEFINITIONS(WindowsFileSystem)

        bool WindowsFileSystem::IsExistImpl(StringView path)
        {
            WindowsString WinPath = String_Convert<WindowsString>(path);

            DWORD res = GetFileAttributes(WinPath.c_str());
            return res != INVALID_FILE_ATTRIBUTES;
        }

        bool WindowsFileSystem::IsDirectoryImpl(StringView path)
        {
            WindowsString WinPath = String_Convert<WindowsString>(path);

            DWORD res = GetFileAttributes(WinPath.c_str());
            return res != INVALID_FILE_ATTRIBUTES && !!(res & FILE_ATTRIBUTE_DIRECTORY);
        }

        bool WindowsFileSystem::IsFileImpl(StringView path)
        {
            WindowsString WinPath = String_Convert<WindowsString>(path);

            DWORD res = GetFileAttributes(WinPath.c_str());
            return res != INVALID_FILE_ATTRIBUTES && !(res & FILE_ATTRIBUTE_DIRECTORY);
        }

        Vector<String> WindowsFileSystem::GetListImpl(StringView directoryPath)
        {
            Vector<String> res;

            WindowsString winPath = Format<WindowsString>(WINDOWS_T("{}/*.*"), directoryPath);
            WIN32_FIND_DATA data;

            HANDLE handle = FindFirstFile(winPath.c_str(), &data);
            if (handle != INVALID_HANDLE_VALUE)
            {
                do
                {
                    res.emplace_back(String_Convert<String>(WindowsStringView(data.cFileName)));
                } while (FindNextFile(handle, &data));
            }

            return res;
        }

        String WindowsFileSystem::GetCurrentDirectoryPathImpl()
        {
            DWORD len = GetCurrentDirectory(0, NULL);
            WindowsString winPath;
            winPath.resize(len);

            if (GetCurrentDirectory(len, winPath.data()))
            {
                return String_Convert<String>(winPath);
            }
            else
            {
                DWORD err = GetLastError();
                CUBE_LOG(Error, WindowsFileSystem, "Failed to get the current directory path. (ErrorCode: {0})", err);
                return CUBE_T("");
            }
        }

        Character WindowsFileSystem::GetSeparatorImpl()
        {
            return CUBE_T('\\');
        }

        SharedPtr<File> WindowsFileSystem::OpenFileImpl(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist)
        {
            DWORD desiredAccess = GetDwDesiredAccess(accessModeFlags);
            WindowsString pPath = String_Convert<WindowsString>(path);

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

            CUBE_LOG(Warning, WindowsFileSystem, "Failed to open a file. ({0}) (ErrorCode: {1})", path, err);
            return nullptr;
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

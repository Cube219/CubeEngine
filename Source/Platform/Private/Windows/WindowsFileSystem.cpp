#ifdef CUBE_PLATFORM_WINDOWS

#include "Windows/WindowsFileSystem.h"

#include <iostream>

#include "Checker.h"
#include "Windows/WindowsString.h"

namespace cube
{
    namespace platform
    {
        // ===== WindowsFilePath =====

        WindowsFilePath::WindowsFilePath(StringView path)
        {
            mPath = String_Convert<WindowsString>(path);
            for (WindowsCharacter& ch : mPath)
            {
                if (ch == WINDOWS_T('/'))
                {
                    ch = WINDOWS_T('\\');
                }
            }
            if (mPath.size() > 1 && mPath.back() == WINDOWS_T('\\'))
            {
                mPath.pop_back();
            }
        }

        WindowsFilePath::WindowsFilePath(AnsiStringView path)
        {
            mPath = String_Convert<WindowsString>(path);
            for (WindowsCharacter& ch : mPath)
            {
                if (ch == WINDOWS_T('/'))
                {
                    ch = WINDOWS_T('\\');
                }
            }
            if (mPath.size() > 1 && mPath.back() == WINDOWS_T('\\'))
            {
                mPath.pop_back();
            }
        }

        WindowsFilePath::WindowsFilePath(WindowsStringView nativePath)
            : mPath(nativePath)
        {
            if (mPath.size() > 1 && mPath.back() == WINDOWS_T('\\'))
            {
                mPath.pop_back();
            }
        }

        String WindowsFilePath::ToString() const
        {
            String result = String_Convert<String>(WindowsStringView(mPath));
            for (Character& ch : result)
            {
                if (ch == CUBE_T('\\'))
                {
                    ch = CUBE_T('/');
                }
            }
            return result;
        }

        AnsiString WindowsFilePath::ToAnsiString() const
        {
            return String_Convert<AnsiString>(WindowsStringView(mPath));
        }

        WindowsStringView WindowsFilePath::GetNativePath() const
        {
            return WindowsStringView(mPath);
        }

        WindowsFilePath WindowsFilePath::GetParent() const
        {
            SizeType pos = mPath.rfind(WINDOWS_T('\\'));
            if (pos == WindowsString::npos || pos == 0)
            {
                if (pos == 0)
                {
                    return WindowsFilePath(WindowsStringView(WINDOWS_T("\\")));
                }
                return WindowsFilePath();
            }
            return WindowsFilePath(WindowsStringView(mPath.data(), pos));
        }

        String WindowsFilePath::GetFileName() const
        {
            SizeType pos = mPath.rfind(WINDOWS_T('\\'));
            if (pos == WindowsString::npos)
            {
                return String_Convert<String>(WindowsStringView(mPath));
            }
            return String_Convert<String>(WindowsStringView(mPath.data() + pos + 1, mPath.size() - pos - 1));
        }

        String WindowsFilePath::GetExtension() const
        {
            SizeType sepPos = mPath.rfind(WINDOWS_T('\\'));
            SizeType dotPos = mPath.rfind(WINDOWS_T('.'));
            if (dotPos == WindowsString::npos || (sepPos != WindowsString::npos && dotPos < sepPos))
            {
                return String();
            }
            return String_Convert<String>(WindowsStringView(mPath.data() + dotPos, mPath.size() - dotPos));
        }

        String WindowsFilePath::GetStem() const
        {
            SizeType sepPos = mPath.rfind(WINDOWS_T('\\'));
            SizeType nameStart = (sepPos == WindowsString::npos) ? 0 : sepPos + 1;
            SizeType dotPos = mPath.rfind(WINDOWS_T('.'));
            SizeType nameEnd = (dotPos == WindowsString::npos || dotPos < nameStart) ? mPath.size() : dotPos;
            return String_Convert<String>(WindowsStringView(mPath.data() + nameStart, nameEnd - nameStart));
        }

        bool WindowsFilePath::IsEmpty() const
        {
            return mPath.empty();
        }

        WindowsFilePath WindowsFilePath::operator/(const WindowsFilePath& rhs) const
        {
            WindowsFilePath result(*this);
            result /= rhs;
            return result;
        }

        WindowsFilePath& WindowsFilePath::operator/=(const WindowsFilePath& rhs)
        {
            if (!mPath.empty() && mPath.back() != WINDOWS_T('\\'))
            {
                mPath += WINDOWS_T('\\');
            }
            mPath += rhs.mPath;
            return *this;
        }

        WindowsFilePath WindowsFilePath::operator/(StringView rhs) const
        {
            return *this / WindowsFilePath(rhs);
        }

        WindowsFilePath& WindowsFilePath::operator/=(StringView rhs)
        {
            return *this /= WindowsFilePath(rhs);
        }

        WindowsFilePath WindowsFilePath::operator/(AnsiStringView rhs) const
        {
            return *this / WindowsFilePath(rhs);
        }

        WindowsFilePath& WindowsFilePath::operator/=(AnsiStringView rhs)
        {
            return *this /= WindowsFilePath(rhs);
        }

        bool WindowsFilePath::operator==(const WindowsFilePath& rhs) const
        {
            return mPath == rhs.mPath;
        }

        // ===== WindowsFile =====

        WindowsFile::WindowsFile(HANDLE fileHandle) :
            mFileHandle(fileHandle)
        {}

        WindowsFile::~WindowsFile()
        {
            CloseHandle(mFileHandle);
        }

        Uint64 WindowsFile::GetFileSize() const
        {
            LARGE_INTEGER size_LI;
            BOOL res = GetFileSizeEx(mFileHandle, &size_LI);
            CHECK_FORMAT(res, "Failed to get file size. (ErrorCode: {0})", GetLastError());

            return size_LI.QuadPart;
        }

        Time WindowsFile::GetWriteTime() const
        {
            FILETIME writeTime;

            BOOL res = GetFileTime(mFileHandle, NULL, NULL, &writeTime);
            CHECK_FORMAT(res, "Failed to get file write time. (ErrorCode: {0})", GetLastError());

            Uint64 low = writeTime.dwLowDateTime;
            Uint64 high = writeTime.dwHighDateTime;
            return (high << (sizeof(DWORD) * 8)) | low;
        }

        void WindowsFile::SetFilePointer(Uint64 offset)
        {
            LARGE_INTEGER distance_LI;
            distance_LI.QuadPart = offset;

            BOOL res = SetFilePointerEx(mFileHandle, distance_LI, NULL, FILE_BEGIN);
            CHECK_FORMAT(res, "Failed to set file pointer. (ErrorCode: {0})", GetLastError());
        }

        void WindowsFile::MoveFilePointer(Int64 distance)
        {
            LARGE_INTEGER distance_LI;
            distance_LI.QuadPart = distance;

            BOOL res = SetFilePointerEx(mFileHandle, distance_LI, NULL, FILE_CURRENT);
            CHECK_FORMAT(res, "Failed to move file pointer. (ErrorCode: {0})", GetLastError());
        }

        Uint64 WindowsFile::Read(void* pReadBuffer, Uint64 bufferSizeToRead)
        {
            DWORD readBufferSize;

            BOOL res = ReadFile(mFileHandle, pReadBuffer, (DWORD)bufferSizeToRead, &readBufferSize, NULL);
            CHECK_FORMAT(res, "Failed to read the file. (ErrorCode: {0})", GetLastError());

            return readBufferSize;
        }

        void WindowsFile::Write(void* pWriteBuffer, Uint64 bufferSize)
        {
            DWORD writtenSize;
            BOOL res = WriteFile(mFileHandle, pWriteBuffer, (DWORD)bufferSize, &writtenSize, nullptr);
            CHECK_FORMAT(res, "Failed to write the file. (ErrorCode: {0})", GetLastError());
        }

        bool WindowsFileSystem::IsExist(const FilePath& path)
        {
            DWORD res = GetFileAttributes(path.GetNativePath().data());
            return res != INVALID_FILE_ATTRIBUTES;
        }

        bool WindowsFileSystem::IsDirectory(const FilePath& path)
        {
            DWORD res = GetFileAttributes(path.GetNativePath().data());
            return res != INVALID_FILE_ATTRIBUTES && !!(res & FILE_ATTRIBUTE_DIRECTORY);
        }

        bool WindowsFileSystem::IsFile(const FilePath& path)
        {
            DWORD res = GetFileAttributes(path.GetNativePath().data());
            return res != INVALID_FILE_ATTRIBUTES && !(res & FILE_ATTRIBUTE_DIRECTORY);
        }

        Vector<String> WindowsFileSystem::GetList(const FilePath& directoryPath)
        {
            Vector<String> res;

            WindowsString winPath = WindowsString(directoryPath.GetNativePath()) + WINDOWS_T("\\*.*");
            WIN32_FIND_DATA data;

            HANDLE handle = FindFirstFile(winPath.c_str(), &data);
            if (handle != INVALID_HANDLE_VALUE)
            {
                do
                {
                    res.emplace_back(String_Convert<String>(WindowsStringView(data.cFileName)));
                } while (FindNextFile(handle, &data));
            }
            FindClose(handle);

            return res;
        }

        FilePath WindowsFileSystem::GetCurrentDirectoryPath()
        {
            DWORD len = GetCurrentDirectory(0, NULL);
            WindowsString winPath;
            winPath.resize(len);

            if (GetCurrentDirectory(len, winPath.data()))
            {
                return FilePath(WindowsStringView(winPath));
            }
            else
            {
                DWORD err = GetLastError();
                CUBE_LOG(Error, WindowsFileSystem, "Failed to get the current directory path. (ErrorCode: {0})", err);
                return FilePath();
            }
        }

        Character WindowsFileSystem::GetSeparator()
        {
            return CUBE_T('\\');
        }

        SharedPtr<WindowsFile> WindowsFileSystem::OpenFile(const FilePath& path, FileAccessModeFlags accessModeFlags, bool createIfNotExist)
        {
            DWORD desiredAccess = GetDwDesiredAccess(accessModeFlags);

            HANDLE file = CreateFile(path.GetNativePath().data(), desiredAccess, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

            if (file != INVALID_HANDLE_VALUE)
            {
                return std::make_shared<WindowsFile>(file);
            }

            DWORD err = GetLastError();

            if (err == ERROR_FILE_NOT_FOUND && createIfNotExist == true)
            {
                file = CreateFile(path.GetNativePath().data(), desiredAccess, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

                if (file != INVALID_HANDLE_VALUE)
                {
                    return std::make_shared<WindowsFile>(file);
                }

                err = GetLastError();
            }

            CUBE_LOG(Warning, WindowsFileSystem, "Failed to open a file. ({0}) (ErrorCode: {1})", path.ToString(), err);
            return nullptr;
        }

        const char* WindowsFileSystem::SplitFileNameFromFullPath(const char* fullPath)
        {
            const char* lastSeparator = strrchr(fullPath, GetSeparator());

            if (lastSeparator != nullptr)
            {
                return lastSeparator + 1;
            }

            return fullPath;
        }

        DWORD WindowsFileSystem::GetDwDesiredAccess(FileAccessModeFlags accessModeFlags)
        {
            DWORD d = 0;

            if (accessModeFlags.IsSet(FileAccessModeFlag::Read))
            {
                d |= GENERIC_READ;
            }

            if (accessModeFlags.IsSet(FileAccessModeFlag::Write))
            {
                d |= GENERIC_WRITE;
            }

            return d;
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_WINDOWS

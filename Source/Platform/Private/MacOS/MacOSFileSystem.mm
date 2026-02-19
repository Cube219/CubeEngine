#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSFileSystem.h"

#include <Foundation/Foundation.h>

#include "Checker.h"
#include "FileSystem.h"
#include "Logger.h"
#include "MacOS/MacOSString.h"

namespace cube
{
    namespace platform
    {
        // ===== MacOSFilePath =====

        MacOSFilePath::MacOSFilePath()
            : mPath(@"")
        {
        }

        MacOSFilePath::~MacOSFilePath()
        {
            mPath = nil;
        }

        MacOSFilePath::MacOSFilePath(const MacOSFilePath& other)
            : mPath([other.mPath copy])
        {
        }

        // TODO: Use better way instead of copying?
        MacOSFilePath::MacOSFilePath(MacOSFilePath&& other)
            : mPath([other.mPath copy])
        {
            other.mPath = nil;
        }

        MacOSFilePath& MacOSFilePath::operator=(const MacOSFilePath& other)
        {
            if (this != &other)
            {
                mPath = [other.mPath copy];
            }
            return *this;
        }

        // TODO: Use better way instead of copying?
        MacOSFilePath& MacOSFilePath::operator=(MacOSFilePath&& other)
        {
            if (this != &other)
            {
                mPath = [other.mPath copy];
                other.mPath = nil;
            }
            return *this;
        }

        MacOSFilePath::MacOSFilePath(StringView path)
        {
            MacOSString macPath = String_Convert<MacOSString>(path);
            for (MacOSCharacter& ch : macPath)
            {
                if (ch == MACOS_T('\\'))
                {
                    ch = MACOS_T('/');
                }
            }
            if (macPath.size() > 1 && macPath.back() == MACOS_T('/'))
            {
                macPath.pop_back();
            }
            mPath = [NSString stringWithUTF8String:macPath.data()];
        }

        MacOSFilePath::MacOSFilePath(AnsiStringView path)
        {
            MacOSString macPath = String_Convert<MacOSString>(path);
            for (MacOSCharacter& ch : macPath)
            {
                if (ch == MACOS_T('\\'))
                {
                    ch = MACOS_T('/');
                }
            }
            if (macPath.size() > 1 && macPath.back() == MACOS_T('/'))
            {
                macPath.pop_back();
            }
            mPath = [NSString stringWithUTF8String:macPath.data()];
        }

        String MacOSFilePath::ToString() const
        {
            return String_Convert<String>(mPath);
        }

        AnsiString MacOSFilePath::ToAnsiString() const
        {
            return AnsiString([mPath UTF8String]);
        }

        MacOSFilePath::MacOSFilePath(NSString* nativePath)
            : mPath([nativePath copy])
        {
        }

        NSString* MacOSFilePath::GetNativePath() const
        {
            return mPath;
        }

        MacOSFilePath MacOSFilePath::GetParent() const
        {
            if ([mPath length] == 0)
            {
                return MacOSFilePath();
            }
            NSString* parent = [mPath stringByDeletingLastPathComponent];
            if ([parent isEqualToString:mPath])
            {
                return MacOSFilePath();
            }
            MacOSFilePath result;
            result.mPath = parent;
            return result;
        }

        String MacOSFilePath::GetFileName() const
        {
            return String_Convert<String>([mPath lastPathComponent]);
        }

        String MacOSFilePath::GetExtension() const
        {
            NSString* ext = [mPath pathExtension];
            if ([ext length] == 0)
            {
                return String();
            }
            NSString* dotExt = [@"." stringByAppendingString:ext];
            return String_Convert<String>(dotExt);
        }

        String MacOSFilePath::GetStem() const
        {
            return String_Convert<String>([[mPath lastPathComponent] stringByDeletingPathExtension]);
        }

        bool MacOSFilePath::IsEmpty() const
        {
            return [mPath length] == 0;
        }

        MacOSFilePath MacOSFilePath::operator/(const MacOSFilePath& rhs) const
        {
            MacOSFilePath result(*this);
            result /= rhs;
            return result;
        }

        MacOSFilePath& MacOSFilePath::operator/=(const MacOSFilePath& rhs)
        {
            if ([mPath length] == 0)
            {
                mPath = [rhs.mPath copy];
            }
            else
            {
                mPath = [mPath stringByAppendingPathComponent:rhs.mPath];
            }
            return *this;
        }

        MacOSFilePath MacOSFilePath::operator/(StringView rhs) const
        {
            return *this / MacOSFilePath(rhs);
        }

        MacOSFilePath& MacOSFilePath::operator/=(StringView rhs)
        {
            return *this /= MacOSFilePath(rhs);
        }

        MacOSFilePath MacOSFilePath::operator/(AnsiStringView rhs) const
        {
            return *this / MacOSFilePath(rhs);
        }

        MacOSFilePath& MacOSFilePath::operator/=(AnsiStringView rhs)
        {
            return *this /= MacOSFilePath(rhs);
        }

        bool MacOSFilePath::operator==(const MacOSFilePath& rhs) const
        {
            return [mPath isEqualToString:rhs.mPath];
        }

        // ===== MacOSFile =====

        MacOSFile::MacOSFile(NSString* filePath, NSFileHandle* fileHandle)
            : mFilePath(filePath)
            , mFileHandle(fileHandle)
            , mCurrentOffset(0)
        { @autoreleasepool {
            NSError* error;

            // Get the file size
            if (![mFileHandle seekToEndReturningOffset:&mSize error:&error])
            {
                CUBE_LOG(Warning, MacOSFile, "Failed to get the file size. ({})", [[error localizedDescription] UTF8String]);
            }
            if (![mFileHandle seekToOffset:0 error:&error])
            {
                CUBE_LOG(Warning, MacOSFile, "Failed to restore the file pointer. ({})", [[error localizedDescription] UTF8String]);
            }
        }}

        MacOSFile::~MacOSFile()
        {
            [mFileHandle closeFile];
        }

        Uint64 MacOSFile::GetFileSize() const
        {
            return mSize;
        }

        Time MacOSFile::GetWriteTime() const
        { @autoreleasepool {
            NSError* error;

            NSDictionary* attributes = [[NSFileManager defaultManager] attributesOfItemAtPath:mFilePath error:&error];
            if (attributes == nil)
            {
                CUBE_LOG(Warning, MacOSFile, "Failed to get the write time. ({0})", [[error localizedDescription] UTF8String]);
                return Time();
            }

            NSDate* modificationDate = [attributes fileModificationDate];
            if (modificationDate != nil)
            {
                return (Uint64)[modificationDate timeIntervalSince1970];
            }
            return Time();
        }}

        void MacOSFile::SetFilePointer(Uint64 offset)
        { @autoreleasepool {
            NSError* error;
            if (![mFileHandle seekToOffset:offset error:&error])
            {
                CHECK_FORMAT(false, "Failed to set file pointer. ({})", [[error localizedDescription] UTF8String]);
            }
            else
            {
                mCurrentOffset = offset;
            }
        }}

        void MacOSFile::MoveFilePointer(Int64 distance)
        { @autoreleasepool {
            NSError* error;
            if (![mFileHandle seekToOffset:(mCurrentOffset + distance) error:&error])
            {
                CHECK_FORMAT(false, "Failed to move file pointer. ({})", [[error localizedDescription] UTF8String]);
            }
            else
            {
                mCurrentOffset += distance;
            }
        }}

        Uint64 MacOSFile::Read(void* pReadBuffer, Uint64 bufferSizeToRead)
        { @autoreleasepool {
            NSError* error;
            NSData* readData = [mFileHandle readDataUpToLength:bufferSizeToRead error:&error];
            if (readData == nil)
            {
                CHECK_FORMAT(false, "Failed to read the file. ({})", [[error localizedDescription] UTF8String]);
            }
            [readData getBytes:pReadBuffer length:readData.length];
            mCurrentOffset += readData.length;
            
            return readData.length;
        }}

        void MacOSFile::Write(void* pWriteBuffer, Uint64 bufferSize)
        { @autoreleasepool {
            NSError* error;
            if (![mFileHandle writeData:[NSData dataWithBytes:pWriteBuffer length:bufferSize] error:&error])
            {
                CHECK_FORMAT(false, "Failed to write the file. ({})", [[error localizedDescription] UTF8String]);
            }

            mCurrentOffset += bufferSize;
        }}

        bool MacOSFileSystem::IsExist(const FilePath& path)
        { @autoreleasepool {
            return [[NSFileManager defaultManager] fileExistsAtPath:path.GetNativePath()];
        }}

        bool MacOSFileSystem::IsDirectory(const FilePath& path)
        { @autoreleasepool {
            BOOL isDirectory;
            [[NSFileManager defaultManager] fileExistsAtPath:path.GetNativePath() isDirectory:&isDirectory];
            return isDirectory;
        }}

        bool MacOSFileSystem::IsFile(const FilePath& path)
        { @autoreleasepool {
            BOOL isDirectory;
            [[NSFileManager defaultManager] fileExistsAtPath:path.GetNativePath() isDirectory:&isDirectory];
            return !isDirectory;
        }}

        Vector<String> MacOSFileSystem::GetList(const FilePath& directoryPath)
        {
            Vector<String> result;
            @autoreleasepool {
                NSArray* list = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:directoryPath.GetNativePath() error:nil];

                if (list)
                {
                    for (NSString* fileOrDirectory in list)
                    {
                        result.push_back(String_Convert<String>(fileOrDirectory));
                    }
                }
            }

            return result;
        }

        FilePath MacOSFileSystem::GetCurrentDirectoryPath()
        {
            return FilePath([[NSFileManager defaultManager] currentDirectoryPath]);
        }

        Character MacOSFileSystem::GetSeparator()
        {
            return CUBE_T('/');
        }

        SharedPtr<MacOSFile> MacOSFileSystem::OpenFile(const FilePath& path, FileAccessModeFlags accessModeFlags, bool createIfNotExist)
        {
            NSString* nsPath = path.GetNativePath();

            if (createIfNotExist)
            {
                if (![[NSFileManager defaultManager] fileExistsAtPath:nsPath])
                {
                    [[NSFileManager defaultManager]
                        createFileAtPath:nsPath
                        contents:nil
                        attributes:nil
                    ];
                }
            }

            NSFileHandle* fileHandle = nil;
            const bool hasReadMode = accessModeFlags.IsSet(FileAccessModeFlag::Read);
            const bool hasWriteMode = accessModeFlags.IsSet(FileAccessModeFlag::Write);
            if (hasReadMode && hasWriteMode)
            {
                fileHandle = [NSFileHandle fileHandleForUpdatingAtPath:nsPath];
            }
            else {
                if (hasReadMode)
                {
                    fileHandle = [NSFileHandle fileHandleForReadingAtPath:nsPath];
                }
                else if (hasWriteMode)
                {
                    fileHandle = [NSFileHandle fileHandleForWritingAtPath:nsPath];
                }
            }

            if (fileHandle != nil)
            {
                return std::make_shared<MacOSFile>(nsPath, fileHandle);
            }
            else
            {
                CUBE_LOG(Warning, MacOSFileSystem, "Failed to open a file. ({0})", path.ToString());
            }
            return nullptr;
        }

        const char* MacOSFileSystem::SplitFileNameFromFullPath(const char* fullPath)
        {
            const char* lastSeparator = strrchr(fullPath, GetSeparator());

            if (lastSeparator != nullptr)
            {
                return lastSeparator + 1;
            }

            return fullPath;
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS

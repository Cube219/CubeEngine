#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSFileSystem.h"

#include "Checker.h"
#include "FileSystem.h"
#include "Logger.h"
#include "MacOS/MacOSString.h"

namespace cube
{
    namespace platform
    {
        FILE_CLASS_DEFINITIONS(MacOSFile)

        MacOSFile::MacOSFile(NSFileHandle* fileHandle) :
            mFileHandle(fileHandle),
            mCurrentOffset(0)
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

        Uint64 MacOSFile::GetFileSizeImpl() const
        {
            return mSize;
        }

        void MacOSFile::SetFilePointerImpl(Uint64 offset)
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

        void MacOSFile::MoveFilePointerImpl(Int64 distance)
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

        Uint64 MacOSFile::ReadImpl(void* pReadBuffer, Uint64 bufferSizeToRead)
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

        void MacOSFile::WriteImpl(void* pWriteBuffer, Uint64 bufferSize)
        { @autoreleasepool {
            NSError* error;
            if (![mFileHandle writeData:[NSData dataWithBytes:pWriteBuffer length:bufferSize] error:&error])
            {
                CHECK_FORMAT(false, "Failed to write the file. ({})", [[error localizedDescription] UTF8String]);
            }

            mCurrentOffset += bufferSize;
        }}

        FILE_SYSTEM_CLASS_DEFINITIONS(MacOSFileSystem)

        SharedPtr<File> MacOSFileSystem::OpenFileImpl(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist)
        {
            NSString* nsPath = ToNSString(path);

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
                [nsPath release];
                return std::make_shared<MacOSFile>(fileHandle);
            }
            else
            {
                CUBE_LOG(Warning, MacOSFileSystem, "Failed to open a file. ({0})", path);
            }
            [nsPath release];
            return nullptr;
        }

        char MacOSFileSystem::GetSeparatorImpl()
        {
            return '/';
        }

        const char* MacOSFileSystem::SplitFileNameFromFullPathImpl(const char* fullPath)
        {
            const char* lastSeparator = strrchr(fullPath, GetSeparatorImpl());

            if (lastSeparator != nullptr)
            {
                return lastSeparator + 1;
            }

            return fullPath;
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS

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

        bool MacOSFileSystem::IsExist(StringView path)
        { @autoreleasepool {
            NSString* nsPath = String_Convert<NSString*>(path);
            return [[NSFileManager defaultManager] fileExistsAtPath:nsPath];
        }}

        bool MacOSFileSystem::IsDirectory(StringView path)
        { @autoreleasepool {
            NSString* nsPath = String_Convert<NSString*>(path);
            BOOL isDirectory;
            [[NSFileManager defaultManager] fileExistsAtPath:nsPath isDirectory:&isDirectory];
            return isDirectory;
        }}

        bool MacOSFileSystem::IsFile(StringView path)
        { @autoreleasepool {
            NSString* nsPath = String_Convert<NSString*>(path);
            BOOL isDirectory;
            [[NSFileManager defaultManager] fileExistsAtPath:nsPath isDirectory:&isDirectory];
            return !isDirectory;
        }}

        Vector<String> MacOSFileSystem::GetList(StringView directoryPath)
        {
            Vector<String> result;
            @autoreleasepool {
                NSString* nsPath = String_Convert<NSString*>(directoryPath);
                NSArray* list = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:nsPath error:nil];

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

        String MacOSFileSystem::GetCurrentDirectoryPath()
        {
            String result;
            @autoreleasepool {
                NSString* path = [[NSFileManager defaultManager] currentDirectoryPath];
                result = String_Convert<String>(path);
            }

            return result;
        }

        Character MacOSFileSystem::GetSeparator()
        {
            return CUBE_T('/');
        }

        SharedPtr<MacOSFile> MacOSFileSystem::OpenFile(StringView path, FileAccessModeFlags accessModeFlags, bool createIfNotExist)
        {
            NSString* nsPath = String_Convert<NSString*>(path);

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
                CUBE_LOG(Warning, MacOSFileSystem, "Failed to open a file. ({0})", path);
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

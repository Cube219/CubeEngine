#pragma once

#ifdef CUBE_PLATFORM_MACOS

#include "PlatformHeader.h"

#include <Foundation/Foundation.h>

#include "Checker.h"

namespace cube
{
#define CHECK_MAIN_THREAD() \
    CHECK_FORMAT([NSThread isMainThread], "You must call UI-releated functions in main thread. " \
        "Consider MacOSUtility::DispatchToMainThread().");

    namespace platform
    {
        class MacOSUtility
        {
        public:
            MacOSUtility() = delete;
            ~MacOSUtility() = delete;

            static void DispatchToMainThread(std::function<void()> func)
            {
                dispatch_async(dispatch_get_main_queue(), ^{
                    func();
                });
            }

            static void DispatchToMainThreadAndWait(std::function<void()> func)
            {
                if ([NSThread isMainThread])
                {
                    func();
                }
                else
                    {
                    dispatch_sync(dispatch_get_main_queue(), ^{
                        func();
                    });
                }
            }
        };
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS

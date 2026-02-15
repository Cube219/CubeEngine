#ifdef CUBE_PLATFORM_MACOS

#include "MacOS/MacOSDebug.h"

#include <execinfo.h>
#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach-o/dyld.h>

#include "Checker.h"
#include "MacOS/MacOSPlatform.h"
#include "MacOS/MacOSString.h"
#include "MacOS/MacOSUtility.h"

@implementation CubeLoggerWindow

- (void)keyDown:(NSEvent* )event
{
    if (cube::platform::MacOSPlatform::IsApplicationClosed())
    {
        cube::platform::MacOSDebug::CloseAndDestroyLoggerWindow();
    }
}

@end

@implementation CubeLoggerTextView

- (void)keyDown:(NSEvent* ) event
{
    if (cube::platform::MacOSPlatform::IsApplicationClosed())
    {
        cube::platform::MacOSDebug::CloseAndDestroyLoggerWindow();
    }
    
    [super keyDown:event];
}

@end

@implementation CubeLoggerWindowDelegate

- (BOOL)windowShouldClose:(NSWindow* ) sender
{
    // Just call termination. Remain logic will be processed in applicationShouldTerminate.
    [NSApp terminate:nil];

    return YES;
}

@end

namespace cube
{
    namespace platform
    {
        CubeLoggerWindow* MacOSDebug::mLoggerWindow;
        CubeLoggerWindowDelegate* MacOSDebug::mLoggerWindowDelegaate;
        CubeLoggerTextView* MacOSDebug::mLoggerTextView;

        bool MacOSDebug::mIsLoggerWindowCreated = false;

        bool MacOSDebug::mIsDebugBreakSetInDebugMessageAlert = false;
        bool MacOSDebug::mIsForceTerminationSetInDebugMessageAlert = false;

        void MacOSDebug::PrintToDebugConsole(StringView str, PrintColorCategory colorCategory)
        {
            MacOSString osStr = String_Convert<MacOSString>(str);

            std::cout << osStr << std::endl;

            if (mIsLoggerWindowCreated)
            {
                osStr.push_back('\n');
                MacOSUtility::DispatchToMainThread([osStr, colorCategory] {
                    AppendLogText(String_Convert<NSString*>(osStr), colorCategory);
                });
            }
        }

        void MacOSDebug::ProcessFatalError(StringView msg)
        {
            ShowDebugMessageAlert(CUBE_T("Fatal error!"), msg);
        }

        void MacOSDebug::ProcessFailedCheck(const char* fileName, int lineNum, StringView formattedMsg)
        {
            if (!MacOSDebug::IsDebuggerAttached())
            {
                ShowDebugMessageAlert(CUBE_T("Check failed!"), formattedMsg);
            }
            else
            {
                // Sync to main thread to print messages in debug console.
                MacOSUtility::DispatchToMainThreadAndWait([]() {});
            }
        }

        constexpr int MAX_NUM_FRAMES = 128;
        constexpr int MAX_NAME_LENGTH = 1024;

        String MacOSDebug::DumpStackTrace(bool removeBeforeProjectFolderPath)
        { @autoreleasepool {
            void* callStack[MAX_NUM_FRAMES];
            int numFrames = backtrace(callStack, MAX_NUM_FRAMES);

            struct StackFrameInfo
            {
                uintptr_t address;
                U8String str;
            };
            Vector<StackFrameInfo> stackFrames(numFrames);
            for (int i = 0; i < numFrames; ++i)
            {
                stackFrames[i].address = (uintptr_t)callStack[i];
            }

            auto GetLibPathAndAddress = [](uintptr_t address, uintptr_t& outLibAddress) -> const char*
            {
                for (Uint32 i = 0; i < _dyld_image_count(); ++i)
                {
                    const mach_header* header = _dyld_get_image_header(i);
                    intptr_t slide = _dyld_get_image_vmaddr_slide(i);

                    // Cast header and calculate the segment size to determine address range
                    if (header->magic == MH_MAGIC_64)
                    {
                        load_command* cmd = (load_command*)((char*)header + sizeof(mach_header_64));
                        for (Uint32 j = 0; j < header->ncmds; ++j)
                        {
                            if (cmd->cmd == LC_SEGMENT_64)
                            {
                                segment_command_64* segCmd = (segment_command_64*)cmd;
                                uintptr_t segStart = slide + segCmd->vmaddr;
                                uintptr_t segEnd = segStart + segCmd->vmsize;

                                if (segStart <= address && address < segEnd)
                                {
                                    outLibAddress = segStart;
                                    return _dyld_get_image_name(i);
                                }
                            }
                            cmd = (load_command*)((char*)cmd + cmd->cmdsize);
                        }
                    }
                }
                return nullptr;
            };

            // Find library mapping in each stack frame
            struct MappedInfo
            {
                const char* libPath;
                Vector<StackFrameInfo*> pStackFrames;
                NSTask* task;
                NSPipe* pipe;
            };
            Map<uintptr_t, MappedInfo> mapLibAddress;
            for (int i = 0; i < numFrames; ++i)
            {
                StackFrameInfo& stackFrame = stackFrames[i];

                uintptr_t libAddress;
                const char* libPath = GetLibPathAndAddress(stackFrame.address, libAddress);
                if (libPath)
                {
                    MappedInfo& mappedInfo = mapLibAddress[libAddress];
                    mappedInfo.libPath = libPath;
                    mappedInfo.pStackFrames.push_back(&stackFrame);
                }
                else
                {
                    stackFrame.str = CUBE_U8_T("Unknown");
                }
            }

            // Collect stack frame infos using atos
            for (auto& [libAddress, mappedInfo] : mapLibAddress)
            {
                NSString* libPathStr = [NSString stringWithUTF8String:mappedInfo.libPath];
                NSString* libAddressStr = [NSString stringWithFormat:@"0x%lx", libAddress];

                mappedInfo.task = [[NSTask alloc] init];
                mappedInfo.task.launchPath = @"/usr/bin/atos";

                NSMutableArray* args = [NSMutableArray arrayWithArray:@[@"-o", libPathStr, @"-l", libAddressStr]];
                for (StackFrameInfo* pStackFrame : mappedInfo.pStackFrames)
                {
                    [args addObject:[NSString stringWithFormat:@"0x%lx", pStackFrame->address]];
                }
                mappedInfo.task.arguments = args;

                mappedInfo.pipe = [NSPipe pipe];
                mappedInfo.task.standardOutput = mappedInfo.pipe;
                [mappedInfo.task launch];
            }
            for (auto& [libAddress, mappedInfo] : mapLibAddress)
            {
                // It calls UIKit functions (I don't know why).
                MacOSUtility::DispatchToMainThreadAndWait([task = mappedInfo.task]{
                    [task waitUntilExit];
                });

                NSData* data = [[mappedInfo.pipe fileHandleForReading] readDataToEndOfFile];
                NSArray* outputArray = [
                    [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding]
                    componentsSeparatedByString:@"\n"
                ];

                for (int i = 0; i < mappedInfo.pStackFrames.size(); ++i)
                {
                    mappedInfo.pStackFrames[i]->str = [outputArray[i] UTF8String];
                }
            }

            // Skip one stack frame
            //     - MacOSDebug::DumpStackTrace()
            String res;
            for (int i = 1; i < numFrames; ++i)
            {
                res += Format(CUBE_T("    {}\n"), stackFrames[i].str);
                if (stackFrames[i].str.empty())
                {
                    stackFrames[i].str = CUBE_U8_T("    Unknown");
                }
            }

            return res;
        }}

        bool MacOSDebug::IsDebuggerAttached()
        {
            // Set up the mib (Management Information Base)
            int mib[4];
            mib[0] = CTL_KERN;
            mib[1] = KERN_PROC;
            mib[2] = KERN_PROC_PID;
            mib[3] = getpid();

            kinfo_proc info;
            info.kp_proc.p_flag = 0;
            size_t size = sizeof(info);

            if (sysctl(mib, 4, &info, &size, NULL, 0) == -1)
            {
                perror("sysctl");
                return false;
            }

            // Check for the P_TRACED flag, which indicates the process is being debugged
            return ((info.kp_proc.p_flag & P_TRACED) != 0);
        }

        void MacOSDebug::CreateAndShowLoggerWindow()
        {
            CHECK_MAIN_THREAD()
            CHECK(!mIsLoggerWindowCreated);

            NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
            
            mLoggerWindow = [[CubeLoggerWindow alloc]
                initWithContentRect:NSMakeRect(0, 0, 1280, 720)
                styleMask:style
                backing:NSBackingStoreBuffered
                defer:NO
            ];
            [mLoggerWindow setTitle:@"CubeEngine Logger"];

            NSScrollView* scrollView = [[NSScrollView alloc] initWithFrame:[[mLoggerWindow contentView] bounds]];
            [scrollView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

            mLoggerTextView = [[CubeLoggerTextView alloc] initWithFrame:[[mLoggerWindow contentView] bounds]];
            mLoggerTextView.editable = NO;
            mLoggerTextView.selectable = YES;

            [scrollView setDocumentView:mLoggerTextView];
            [scrollView setHasVerticalScroller:YES];

            [[mLoggerWindow contentView] addSubview:scrollView];

            mLoggerWindowDelegaate = [[CubeLoggerWindowDelegate alloc] init];
            [mLoggerWindow setDelegate:mLoggerWindowDelegaate];

            [mLoggerWindow makeKeyAndOrderFront:nil];
            mIsLoggerWindowCreated = true;

            // Move to top-left
            NSScreen* screen = [mLoggerWindow screen] ?: [NSScreen mainScreen];
            NSRect screenFrame = [screen visibleFrame];
            NSRect windowFrame = [mLoggerWindow frame];

            NSPoint topLeft;
            topLeft.x = screenFrame.origin.x;
            topLeft.y = NSMaxY(screenFrame) - windowFrame.size.height;

            [mLoggerWindow setFrameOrigin:topLeft];
        }

        void MacOSDebug::AppendLogText(NSString* text, PrintColorCategory colorCategory)
        {
            CHECK_MAIN_THREAD()
            CHECK(mIsLoggerWindowCreated);

            @autoreleasepool {
                NSColor* color;
                switch (colorCategory)
                {
                case PrintColorCategory::Warning:
                    color = [NSColor systemOrangeColor];
                    break;
                case PrintColorCategory::Error:
                    color = [NSColor systemRedColor];
                    break;
                case PrintColorCategory::Default:
                default:
                    color = [NSColor textColor];
                    break;
                }
                
                NSTextStorage* textStorage = mLoggerTextView.textStorage;
                NSAttributedString* attrText = [[NSAttributedString alloc]
                    initWithString:text
                        attributes:@{
                            NSForegroundColorAttributeName: color,
                            NSBackgroundColorAttributeName: [NSColor textBackgroundColor],
                            NSFontAttributeName: [NSFont fontWithName:@"Menlo" size:12]
                    }
                ];
                [textStorage appendAttributedString:attrText];

                // Scroll to bottom
                [mLoggerTextView scrollRangeToVisible:NSMakeRange(textStorage.length, 0)];
            }
        }
        

        void MacOSDebug::CloseAndDestroyLoggerWindow()
        {
            CHECK_MAIN_THREAD()

            if (mIsLoggerWindowCreated)
            {
                [mLoggerTextView release];
                [mLoggerWindow close];
                [mLoggerWindowDelegaate release];

                mIsLoggerWindowCreated = false;
            }
        }

        void MacOSDebug::ShowDebugMessageAlert(StringView title, StringView msg)
        {
            MacOSUtility::DispatchToMainThreadAndWait([title, msg]() {
                ShowDebugMessageAlert_MainThread(title, msg);
            });
            // Dispatch and wait to ensure the alert modal is closed.
            MacOSUtility::DispatchToMainThreadAndWait([]() {});

            if (mIsDebugBreakSetInDebugMessageAlert)
            {
                mIsDebugBreakSetInDebugMessageAlert = false;
                CUBE_DEBUG_BREAK
            }

            if (mIsForceTerminationSetInDebugMessageAlert)
            {
                platform::MacOSPlatform::ForceTerminateMainLoopThread();
                exit(3);
            }
        }

        void MacOSDebug::ShowDebugMessageAlert_MainThread(StringView title, StringView msg)
        {
            CHECK_MAIN_THREAD()

            @autoreleasepool {
                NSTextView* textView = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, 400, 600)];
                [textView setEditable:NO];
                [textView setSelectable:YES];

                NSAttributedString* attrText = [[NSAttributedString alloc]
                    initWithString:String_Convert<NSString*>(msg)
                    attributes:@{
                        NSForegroundColorAttributeName: [NSColor textColor],
                        NSBackgroundColorAttributeName: [NSColor textBackgroundColor],
                        NSFontAttributeName: [NSFont fontWithName:@"Menlo" size:12]
                    }
                ];
                [textView.textStorage setAttributedString: attrText];

                NSScrollView* scrollView = [[NSScrollView alloc] initWithFrame:NSMakeRect(0, 0, 400, 600)];
                [scrollView setDocumentView:textView];
                [scrollView setHasVerticalScroller:YES];
                [scrollView setHasHorizontalScroller:YES];
                // Scroll to bottom
                [textView scrollRangeToVisible:NSMakeRange(msg.size(), 0)];

                NSAlert* alert = [[NSAlert alloc] init];
                [alert setMessageText:String_Convert<NSString*>(title)];
                [alert setAccessoryView:scrollView];
                [alert setAlertStyle:NSAlertStyleCritical];
                [alert addButtonWithTitle:@"Exit"];
                [alert addButtonWithTitle:@"Ignore"];
                [alert addButtonWithTitle:@"Debug"];

                NSModalResponse response = [alert runModal];
                // Defer the logics to close the alert modal first.
                switch (response)
                {
                case NSAlertFirstButtonReturn: // Exit
                    mIsForceTerminationSetInDebugMessageAlert = true;
                    break;
                case NSAlertSecondButtonReturn: // Ignore
                    break;
                case NSAlertThirdButtonReturn: // Debug
                    mIsDebugBreakSetInDebugMessageAlert = true;
                    break;
                default:
                    break;
                }
            }
        }
    } // namespace platform
} // namespace cube

#endif // CUBE_PLATFORM_MACOS

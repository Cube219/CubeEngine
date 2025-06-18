#pragma once

#include "DX12Header.h"

#include <dxgidebug.h>

namespace cube
{
    class DX12Device;

    namespace platform
    {
        class DLib;
    } // namespace platform

    class DX12Debug
    {
    public:
        DX12Debug() = delete;
        ~DX12Debug() = delete;

        static void Initialize(Uint32& outDxgiFactoryFlags);
        static void Shutdown();

        static void InitializeD3DExceptionHandler(DX12Device& device);
        static void ShutdownD3DExceptionHandler();

    private:
        static LONG D3DVectoredExceptionHandler(EXCEPTION_POINTERS* exceptionInfo);
        static void CheckDebugMessages();

        static bool mIsInitizlied;
        static DX12Device* mDevice;
        static HANDLE mExceptionHandler;

        static SharedPtr<platform::DLib> mDXGIDebugDLib;
        static ComPtr<IDXGIDebug> mDXGIDebug;
    };
} // namespace cube

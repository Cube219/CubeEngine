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

        static void InitializeD3DDebugMessageLogging(DX12Device& device);
        static void ShutdownD3DDebugMessageLogging();

    private:
        static LONG D3DVectoredExceptionHandler(EXCEPTION_POINTERS* exceptionInfo);
        static void CheckD3DDebugMessages();

        static void D3DMessageCallback(D3D12_MESSAGE_CATEGORY Category, D3D12_MESSAGE_SEVERITY Severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* pContext);


        static void LogD3DDebugMessage(D3D12_MESSAGE_SEVERITY severity, LPCSTR pDescription);

        static bool mIsInitialized;
        static DX12Device* mDevice;
        static DWORD mMessageCallbackCookie;
        static HANDLE mExceptionHandler;

        static SharedPtr<platform::DLib> mDXGIDebugDLib;
        static ComPtr<IDXGIDebug> mDXGIDebug;
    };
} // namespace cube

#include "DX12Debug.h"

#include "Checker.h"
#include "Logger.h"
#include "Platform.h"
#include "PlatformDebug.h"

#include "DX12Device.h"

namespace cube
{
    bool DX12Debug::mIsInitialized = false;
    DX12Device* DX12Debug::mDevice;
    DWORD DX12Debug::mMessageCallbackCookie;
    HANDLE DX12Debug::mExceptionHandler;

    SharedPtr<platform::DLib> DX12Debug::mDXGIDebugDLib;
    ComPtr<IDXGIDebug> DX12Debug::mDXGIDebug;

    void DX12Debug::Initialize(Uint32& outDxgiFactoryFlags)
    {
        CHECK(mIsInitialized == false);

        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            outDxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

            CUBE_LOG(Info, DX12, "Enabled DX12 debug layer.");
        }

        mDXGIDebugDLib = platform::Platform::LoadDLib(platform::FilePath(CUBE_T("dxgidebug")));
        if (mDXGIDebugDLib)
        {
            using DXGIGetDebugInterfaceFunction = HRESULT(*)(REFIID, void**);
            DXGIGetDebugInterfaceFunction DXGIGetDebugInterfaceFunc = reinterpret_cast<DXGIGetDebugInterfaceFunction>(mDXGIDebugDLib->GetFunction(CUBE_T("DXGIGetDebugInterface")));
            if (DXGIGetDebugInterfaceFunc != nullptr)
            {
                DXGIGetDebugInterfaceFunc(IID_PPV_ARGS(&mDXGIDebug));

                CUBE_LOG(Info, DX12, "Enabled DXGI debug layer.");
            }
        }

        mIsInitialized = true;
    }

    void DX12Debug::Shutdown()
    {
        if (mIsInitialized == false)
        {
            return;
        }

        if (mDXGIDebug)
        {
            CUBE_LOG(Info, DX12, "===== Begin live objects =====");

            DXGI_DEBUG_RLO_FLAGS flags = DXGI_DEBUG_RLO_ALL;
            mDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, flags);
            mDXGIDebug = nullptr;

            CUBE_LOG(Info, DX12, "===== End live objects =====");
        }
        mDXGIDebugDLib = nullptr;
    }

    void DX12Debug::InitializeD3DDebugMessageLogging(DX12Device& device)
    {
        mDevice = &device;

        bool useMessageCallback = false;
        ComPtr<ID3D12InfoQueue> infoQueue;
        if (SUCCEEDED(device.GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue))))
        {
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

            ComPtr<ID3D12InfoQueue1> infoQueue1;
            if (SUCCEEDED(device.GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue1))))
            {
                // Use debug layer message callback if possible.
                infoQueue1->RegisterMessageCallback(D3DMessageCallback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &mMessageCallbackCookie);
            
                useMessageCallback = true;
            }
        }

        if (!useMessageCallback)
        {
            mExceptionHandler = AddVectoredExceptionHandler(1, D3DVectoredExceptionHandler);
        }
    }

    void DX12Debug::ShutdownD3DDebugMessageLogging()
    {
        if (mMessageCallbackCookie)
        {
            ComPtr<ID3D12InfoQueue1> infoQueue1;
            if (SUCCEEDED(mDevice->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue1))))
            {
                infoQueue1->UnregisterMessageCallback(mMessageCallbackCookie);
            }
        }

        if (mExceptionHandler)
        {
            RemoveVectoredExceptionHandler(mExceptionHandler);
        }
    }

    LONG DX12Debug::D3DVectoredExceptionHandler(EXCEPTION_POINTERS* exceptionInfo)
    {
        if (exceptionInfo->ExceptionRecord->ExceptionCode == _FACDXGI)
        {
            CheckD3DDebugMessages();
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        return EXCEPTION_CONTINUE_SEARCH;
    }

    void DX12Debug::CheckD3DDebugMessages()
    {
        if (mIsInitialized == false)
        {
            return;
        }

        ComPtr<ID3D12InfoQueue> infoQueue;
        if (SUCCEEDED(mDevice->GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue))))
        {
            constexpr Uint64 initialBufferSize = 1024;
            char staticBuffer[initialBufferSize];
            void* dynamicBuffer = nullptr;
            Uint64 currentBufferSize = initialBufferSize;

            int numMessages = static_cast<int>(infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter());
            for (int i = 0; i < numMessages; ++i)
            {
                SizeType messageLength;
                infoQueue->GetMessage(i, nullptr, &messageLength);
                if (messageLength > currentBufferSize)
                {
                    if (dynamicBuffer)
                    {
                        platform::Platform::Free(dynamicBuffer);
                    }
                    currentBufferSize = messageLength;
                    dynamicBuffer = platform::Platform::Allocate(currentBufferSize);
                }

                if (currentBufferSize == initialBufferSize || dynamicBuffer)
                {
                    D3D12_MESSAGE* message;
                    if (dynamicBuffer)
                    {
                        message = (D3D12_MESSAGE*)dynamicBuffer;
                    }
                    else
                    {
                        message = (D3D12_MESSAGE*)staticBuffer;
                    }

                    infoQueue->GetMessage(i, message, &messageLength);
                    LogD3DDebugMessage(message->Severity, message->pDescription);
                }
                else
                {
                    // Failed memory allocation. Use static buffer at the next message.
                    currentBufferSize = initialBufferSize;
                }
            }

            if (dynamicBuffer)
            {
                platform::Platform::Free(dynamicBuffer);
            }

            infoQueue->ClearStoredMessages();
        }
    }

    void DX12Debug::D3DMessageCallback(D3D12_MESSAGE_CATEGORY Category, D3D12_MESSAGE_SEVERITY Severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* pContext)
    {
        if (Severity <= D3D12_MESSAGE_SEVERITY_WARNING)
        {
            LogD3DDebugMessage(Severity, pDescription);
        }
    }

    void DX12Debug::LogD3DDebugMessage(D3D12_MESSAGE_SEVERITY severity, LPCSTR pDescription)
    {
        switch (severity)
        {
        case D3D12_MESSAGE_SEVERITY_CORRUPTION:
        case D3D12_MESSAGE_SEVERITY_ERROR:
            CUBE_LOG(Error, DX12, "[DX12Debug]: {0}", pDescription);

            if (platform::Debug::IsDebuggerAttached())
            {
                CUBE_DEBUG_BREAK
            }
            break;

        case D3D12_MESSAGE_SEVERITY_WARNING:
            CUBE_LOG(Warning, DX12, "[DX12Debug]: {0}", pDescription);
            break;

        case D3D12_MESSAGE_SEVERITY_INFO:
        case D3D12_MESSAGE_SEVERITY_MESSAGE:
            CUBE_LOG(Info, DX12, "[DX12Debug]: {0}", pDescription);
            break;
        }
    }
} // namespace cube

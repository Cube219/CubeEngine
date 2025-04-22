#include "DX12Debug.h"

#include "Checker.h"
#include "Logger.h"
#include "Platform.h"
#include "PlatformDebug.h"

#include "DX12Device.h"

namespace cube
{
    bool DX12Debug::mIsInitizlied = false;

    SharedPtr<platform::DLib> DX12Debug::mDXGIDebugDLib;
    ComPtr<IDXGIDebug> DX12Debug::mDXGIDebug;

    void DX12Debug::Initialize(Uint32& outDxgiFactoryFlags)
    {
        CHECK(mIsInitizlied == false);

        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            outDxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

            CUBE_LOG(Info, DX12, "Enabled DX12 debug layer.");
        }

        mDXGIDebugDLib = platform::Platform::LoadDLib(CUBE_T("dxgidebug"));
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

        mIsInitizlied = true;
    }

    void DX12Debug::Shutdown()
    {
        if (mIsInitizlied == false)
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

    void DX12Debug::CheckDebugMessages(DX12Device& device)
    {
        if (mIsInitizlied == false)
        {
            return;
        }

        ComPtr<ID3D12InfoQueue> infoQueue;
        if (SUCCEEDED(device.GetDevice()->QueryInterface(IID_PPV_ARGS(&infoQueue))))
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

                    switch (message->Severity)
                    {
                    case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                    case D3D12_MESSAGE_SEVERITY_ERROR:
                        CUBE_LOG(Error, DX12, "[DX12Debug]: {}", message->pDescription);

                        if (platform::PlatformDebug::IsDebuggerAttached())
                        {
                            platform::PlatformDebug::BreakDebug();
                        }

                        break;

                    case D3D12_MESSAGE_SEVERITY_WARNING:
                        CUBE_LOG(Warning, DX12, "[DX12Debug]: {}", message->pDescription);
                        break;

                    case D3D12_MESSAGE_SEVERITY_INFO:
                    case D3D12_MESSAGE_SEVERITY_MESSAGE:
                        CUBE_LOG(Info, DX12, "[DX12Debug]: {}", message->pDescription);
                        break;
                    }
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
} // namespace cube

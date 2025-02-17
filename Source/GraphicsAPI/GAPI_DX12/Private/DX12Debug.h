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

        static void CheckDebugMessages(DX12Device& device);

    private:
        static bool mIsInitizlied;

        static SharedPtr<platform::DLib> mDXGIDebugDLib;
        static ComPtr<IDXGIDebug> mDXGIDebug;
    };
} // namespace cube

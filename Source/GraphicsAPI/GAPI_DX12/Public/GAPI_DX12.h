#pragma once

#include "DX12Header.h"

#include "GAPI.h"

#include "DX12Device.h"

namespace cube
{
    namespace platform
    {
        class DLib;
    }

    extern "C" CUBE_DX12_EXPORT GAPI* CreateGAPI();

    class CUBE_DX12_EXPORT GAPI_DX12 : public GAPI
    {
    public:
        GAPI_DX12() = default;
        virtual ~GAPI_DX12() = default;

         virtual void Initialize(const GAPIInitInfo& initInfo) override;
         virtual void Shutdown(const ImGUIContext& imGUIInfo) override;

         virtual void OnBeforeRender() override;
         virtual void OnAfterRender() override;
         virtual void OnBeforePresent(gapi::Viewport* viewport) override;
         virtual void OnAfterPresent() override;

         virtual SharedPtr<gapi::CommandList> CreateCommandList(const gapi::CommandListCreateInfo& info) override;
         virtual SharedPtr<gapi::Fence> CreateFence(const gapi::FenceCreateInfo& info) override;
         virtual SharedPtr<gapi::Viewport> CreateViewport(const gapi::ViewportCreateInfo& info) override;

    private:
        void InitializeImGUI(const ImGUIContext& imGUIInfo);
        void ShutdownImGUI(const ImGUIContext& imGUIInfo);

        ComPtr<IDXGIFactory2> mFactory;
        ComPtr<IDXGIFactory6> mFactory6;
        Vector<DX12Device*> mDevices;
        DX12Device* mMainDevice;

        ImGUIContext mImGUIContext;
        ID3D12GraphicsCommandList* mImGUIRenderCommandList;

        Uint64 mCurrentRenderFrame;
    };
} // namespace cube

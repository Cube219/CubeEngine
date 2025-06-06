#pragma once

#include "DX12Header.h"

#include "GAPI.h"

#include "DX12Device.h"

namespace cube
{
    namespace platform
    {
        class DLib;
    } // namespace platform

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

        virtual void WaitForGPU() override;

        virtual SharedPtr<gapi::Buffer> CreateBuffer(const gapi::BufferCreateInfo& info) override;
        virtual SharedPtr<gapi::CommandList> CreateCommandList(const gapi::CommandListCreateInfo& info) override;
        virtual SharedPtr<gapi::Fence> CreateFence(const gapi::FenceCreateInfo& info) override;
        virtual SharedPtr<gapi::Pipeline> CreateGraphicsPipeline(const gapi::GraphicsPipelineCreateInfo& info) override;
        virtual SharedPtr<gapi::Pipeline> CreateComputePipeline(const gapi::ComputePipelineCreateInfo& info) override;
        virtual SharedPtr<gapi::Shader> CreateShader(const gapi::ShaderCreateInfo& info) override;
        virtual SharedPtr<gapi::ShaderVariablesLayout> CreateShaderVariablesLayout(const gapi::ShaderVariablesLayoutCreateInfo& info) override;
        virtual SharedPtr<gapi::Viewport> CreateViewport(const gapi::ViewportCreateInfo& info) override;

        virtual gapi::TimestampList GetLastTimestampList() override;
        virtual gapi::VRAMStatus GetVRAMUsage() override;

    private:
        void InitializeImGUI(const ImGUIContext& imGUIInfo);
        void ShutdownImGUI(const ImGUIContext& imGUIInfo);

        ComPtr<IDXGIFactory2> mFactory;
        ComPtr<IDXGIFactory6> mFactory6;
        Vector<DX12Device*> mDevices;
        DX12Device* mMainDevice;

        ImGUIContext mImGUIContext;
        ComPtr<ID3D12GraphicsCommandList> mImGUIRenderCommandList;

        Uint64 mCurrentRenderFrame;
    };
} // namespace cube

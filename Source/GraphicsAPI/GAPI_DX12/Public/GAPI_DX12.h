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

        virtual void SetNumGPUSync(Uint32 newNumGPUSync) override;

        virtual void OnBeforeRender() override;
        virtual void OnAfterRender() override;
        virtual void OnBeforePresent(gapi::TextureRTV* backbufferRTV) override;
        virtual void OnAfterPresent() override;

        virtual void BeginRenderingFrame() override;
        virtual void EndRenderingFrame() override;
        virtual void WaitAllGPUSync() override;

        virtual const gapi::ShaderParameterHelper& GetShaderParameterHelper() const override;

        virtual SharedPtr<gapi::Buffer> CreateBuffer(const gapi::BufferCreateInfo& info) override;
        virtual SharedPtr<gapi::CommandList> CreateCommandList(const gapi::CommandListCreateInfo& info) override;
        virtual SharedPtr<gapi::Fence> CreateFence(const gapi::FenceCreateInfo& info) override;
        virtual SharedPtr<gapi::GraphicsPipeline> CreateGraphicsPipeline(const gapi::GraphicsPipelineCreateInfo& info) override;
        virtual SharedPtr<gapi::ComputePipeline> CreateComputePipeline(const gapi::ComputePipelineCreateInfo& info) override;
        virtual SharedPtr<gapi::Sampler> CreateSampler(const gapi::SamplerCreateInfo& info) override;
        virtual SharedPtr<gapi::Shader> CreateShader(const gapi::ShaderCreateInfo& info) override;
        virtual SharedPtr<gapi::ShaderVariablesLayout> CreateShaderVariablesLayout(const gapi::ShaderVariablesLayoutCreateInfo& info) override;
        virtual SharedPtr<gapi::Texture> CreateTexture(const gapi::TextureCreateInfo& info) override;
        virtual SharedPtr<gapi::SwapChain> CreateSwapChain(const gapi::SwapChainCreateInfo& info) override;

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

        Uint64 mCurrentGPUFrame;
        Uint32 mNumGPUSync;
    };
} // namespace cube

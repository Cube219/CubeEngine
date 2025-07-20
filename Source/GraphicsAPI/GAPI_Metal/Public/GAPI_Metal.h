#pragma once

#include "MetalHeader.h"

#include "GAPI.h"

#include "MetalDevice.h"
#include "Platform.h"

@interface CubeImGUIMTKView : MTKView <MTKViewDelegate>

@end

namespace cube
{
    extern "C" CUBE_METAL_EXPORT GAPI* CreateGAPI();

    class CUBE_METAL_EXPORT GAPI_Metal : public GAPI
    {
    public:
        GAPI_Metal() = default;
        virtual ~GAPI_Metal() = default;

        virtual void Initialize(const GAPIInitInfo& initInfo) override;
        virtual void Shutdown(const ImGUIContext& imGUIInfo) override;

        virtual void SetNumGPUSync(Uint32 newNumGPUSync) override;

        virtual void OnBeforeRender() override;
        virtual void OnAfterRender() override;
        virtual void OnBeforePresent(gapi::Viewport* viewport) override;
        virtual void OnAfterPresent() override;

        virtual void BeginRenderingFrame() override;
        virtual void EndRenderingFrame() override;
        virtual void WaitAllGPUSync() override;

        virtual SharedPtr<gapi::Buffer> CreateBuffer(const gapi::BufferCreateInfo& info) override;
        virtual SharedPtr<gapi::CommandList> CreateCommandList(const gapi::CommandListCreateInfo& info) override;
        virtual SharedPtr<gapi::Fence> CreateFence(const gapi::FenceCreateInfo& info) override;
        virtual SharedPtr<gapi::GraphicsPipeline> CreateGraphicsPipeline(const gapi::GraphicsPipelineCreateInfo& info) override;
        virtual SharedPtr<gapi::ComputePipeline> CreateComputePipeline(const gapi::ComputePipelineCreateInfo& info) override;
        virtual SharedPtr<gapi::Sampler> CreateSampler(const gapi::SamplerCreateInfo& info) override;
        virtual SharedPtr<gapi::Shader> CreateShader(const gapi::ShaderCreateInfo& info) override;
        virtual SharedPtr<gapi::ShaderVariablesLayout> CreateShaderVariablesLayout(const gapi::ShaderVariablesLayoutCreateInfo& info) override;
        virtual SharedPtr<gapi::Texture> CreateTexture(const gapi::TextureCreateInfo& info) override;
        virtual SharedPtr<gapi::Viewport> CreateViewport(const gapi::ViewportCreateInfo& info) override;

        virtual gapi::TimestampList GetLastTimestampList() override;
        virtual gapi::VRAMStatus GetVRAMUsage() override;

    private:
        void InitializeImGUI(const ImGUIContext& imGUIInfo);
        void ShutdownImGUI(const ImGUIContext& imGUIInfo);

        Vector<MetalDevice*> mDevices;
        MetalDevice* mMainDevice;
        MTLRenderPassDescriptor* mRenderPassDescriptor;
        id<MTLCommandQueue> mCommandQueue;

        ImGUIContext mImGUIContext;
        CubeImGUIMTKView* mImGUIView;
    };
} // namespace cube

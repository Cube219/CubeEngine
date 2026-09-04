#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"
#include "GAPI_Resource.h"
#include "GAPI_Timestamp.h"
#include "GAPITypes.h"

namespace cube
{
    namespace gapi
    {
        class GraphicsPipeline;
        struct GraphicsPipelineCreateInfo;
        class ComputePipeline;
        struct ComputePipelineCreateInfo;
        class Sampler;
        struct SamplerCreateInfo;
        class Shader;
        struct ShaderCreateInfo;
        struct ShaderCompileResult;
        class ShaderParameterHelper;
        class Buffer;
        struct BufferCreateInfo;
        class Texture;
        struct TextureInfo;
        struct TextureCreateInfo;
        class CommandList;
        struct CommandListCreateInfo;
        class Fence;
        struct FenceCreateInfo;
        class SwapChain;
        struct SwapChainCreateInfo;

        class TextureRTV;
    } // namespace gapi

    struct ImGUIContext
    {
        void* context = nullptr;
        void* allocFunc = nullptr;
        void* freeFunc = nullptr;
        void* userData = nullptr;
    };

    struct GAPIInitInfo
    {
        Uint32 numGPUSync;
        bool enableDebugLayer = false;
        ImGUIContext imGUI;
    };

    class GAPI
    {
    public:
        GAPI() = default;
        virtual ~GAPI() = default;

        virtual void Initialize(const GAPIInitInfo& initInfo) = 0;
        virtual void Shutdown(const ImGUIContext& imGUIInfo) = 0;

        virtual void SetNumGPUSync(Uint32 newNumGPUSync) = 0;

        virtual void OnBeforeRender() = 0;
        virtual void OnAfterRender() = 0;
        virtual void OnBeforePresent(gapi::Texture* backbuffer) = 0;
        virtual void OnAfterPresent() = 0;

        virtual void BeginRenderingFrame() = 0;
        virtual void EndRenderingFrame() = 0;
        virtual void WaitAllGPUSync() = 0;

        const gapi::GAPIInfo& GetInfo() const { return mInfo; }
        virtual const gapi::ShaderParameterHelper& GetShaderParameterHelper() const = 0;

        virtual SharedPtr<gapi::Buffer> CreateBuffer(const gapi::BufferCreateInfo& info) = 0;
        virtual SharedPtr<gapi::CommandList> CreateCommandList(const gapi::CommandListCreateInfo& info) = 0;
        virtual SharedPtr<gapi::Fence> CreateFence(const gapi::FenceCreateInfo& info) = 0;
        virtual SharedPtr<gapi::GraphicsPipeline> CreateGraphicsPipeline(const gapi::GraphicsPipelineCreateInfo& info) = 0;
        virtual SharedPtr<gapi::ComputePipeline> CreateComputePipeline(const gapi::ComputePipelineCreateInfo& info) = 0;
        virtual SharedPtr<gapi::Sampler> CreateSampler(const gapi::SamplerCreateInfo& info) = 0;
        virtual SharedPtr<gapi::Shader> CreateShader(const gapi::ShaderCreateInfo& info) = 0;
        virtual SharedPtr<gapi::Texture> CreateTexture(const gapi::TextureCreateInfo& createInfo) = 0;
        virtual SharedPtr<gapi::SwapChain> CreateSwapChain(const gapi::SwapChainCreateInfo& info) = 0;

        virtual gapi::TimestampRangeList GetLastTimestampRangeList() = 0;
        virtual gapi::VRAMStatus GetVRAMUsage() = 0;

    protected:
        gapi::GAPIInfo mInfo;
    };
} // namespace cube

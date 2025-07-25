#pragma once

#include "GAPIHeader.h"

#include "CubeString.h"
#include "GAPI_Timestamp.h"

namespace cube
{
    namespace gapi
    {
        class ShaderVariablesLayout;
        struct ShaderVariablesLayoutCreateInfo;
        class GraphicsPipeline;
        struct GraphicsPipelineCreateInfo;
        class ComputePipeline;
        struct ComputePipelineCreateInfo;
        class Sampler;
        struct SamplerCreateInfo;
        class Shader;
        struct ShaderCreateInfo;
        struct ShaderCompileResult;
        class Buffer;
        struct BufferCreateInfo;
        class Texture;
        struct TextureCreateInfo;
        class CommandList;
        struct CommandListCreateInfo;
        class Fence;
        struct FenceCreateInfo;
        class Viewport;
        struct ViewportCreateInfo;
    } // namespace gapi

    enum class GAPIName
    {
        Unknown,
        DX12,
        Metal
    };
    inline const Character* GAPINameToString(GAPIName gAPIName)
    {
        switch (gAPIName)
        {
        case GAPIName::DX12:
            return CUBE_T("DX12");
        case GAPIName::Metal:
            return CUBE_T("Metal");
        case GAPIName::Unknown:
        default:
            return CUBE_T("Unknown");
        }
    }

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
        struct Info
        {
            GAPIName apiName = GAPIName::Unknown;
            bool useLeftHanded = false;
        };
    public:
        GAPI() = default;
        virtual ~GAPI() = default;

        virtual void Initialize(const GAPIInitInfo& initInfo) = 0;
        virtual void Shutdown(const ImGUIContext& imGUIInfo) = 0;

        virtual void SetNumGPUSync(Uint32 newNumGPUSync) = 0;

        virtual void OnBeforeRender() = 0;
        virtual void OnAfterRender() = 0;
        virtual void OnBeforePresent(gapi::Viewport* viewport) = 0;
        virtual void OnAfterPresent() = 0;

        virtual void BeginRenderingFrame() = 0;
        virtual void EndRenderingFrame() = 0;
        virtual void WaitAllGPUSync() = 0;

        const Info& GetInfo() const { return mInfo; }

        virtual SharedPtr<gapi::Buffer> CreateBuffer(const gapi::BufferCreateInfo& info) = 0;
        virtual SharedPtr<gapi::CommandList> CreateCommandList(const gapi::CommandListCreateInfo& info) = 0;
        virtual SharedPtr<gapi::Fence> CreateFence(const gapi::FenceCreateInfo& info) = 0;
        virtual SharedPtr<gapi::GraphicsPipeline> CreateGraphicsPipeline(const gapi::GraphicsPipelineCreateInfo& info) = 0;
        virtual SharedPtr<gapi::ComputePipeline> CreateComputePipeline(const gapi::ComputePipelineCreateInfo& info) = 0;
        virtual SharedPtr<gapi::Sampler> CreateSampler(const gapi::SamplerCreateInfo& info) = 0;
        virtual SharedPtr<gapi::Shader> CreateShader(const gapi::ShaderCreateInfo& info) = 0;
        virtual SharedPtr<gapi::ShaderVariablesLayout> CreateShaderVariablesLayout(const gapi::ShaderVariablesLayoutCreateInfo& info) = 0;
        virtual SharedPtr<gapi::Texture> CreateTexture(const gapi::TextureCreateInfo& info) = 0;
        virtual SharedPtr<gapi::Viewport> CreateViewport(const gapi::ViewportCreateInfo& info) = 0;

        virtual gapi::TimestampList GetLastTimestampList() = 0;
        virtual gapi::VRAMStatus GetVRAMUsage() = 0;

    protected:
        Info mInfo;
    };
} // namespace cube

#include "GAPI_Metal.h"

#include "Checker.h"
#include "Logger.h"

namespace cube
{
    GAPI* CreateGAPI()
    {
        return new GAPI_Metal();
    }

    void GAPI_Metal::Initialize(const GAPIInitInfo& initInfo)
    {
        CUBE_LOG(LogType::Info, Metal, "Initialize GAPI_Metal.");
    }

    void GAPI_Metal::Shutdown(const ImGUIContext& imGUIInfo)
    {
        CUBE_LOG(LogType::Info, Metal, "Shutdown GAPI_Metal.");
    }

    void GAPI_Metal::OnBeforeRender()
    {
    }

    void GAPI_Metal::OnAfterRender()
    {
    }

    void GAPI_Metal::OnBeforePresent(gapi::Viewport* viewport)
    {
    }

    void GAPI_Metal::OnAfterPresent()
    {
    }

    void GAPI_Metal::WaitForGPU()
    {
        NOT_IMPLEMENTED();
    }

    SharedPtr<gapi::Buffer> GAPI_Metal::CreateBuffer(const gapi::BufferCreateInfo& info)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    SharedPtr<gapi::CommandList> GAPI_Metal::CreateCommandList(const gapi::CommandListCreateInfo& info)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    SharedPtr<gapi::Fence> GAPI_Metal::CreateFence(const gapi::FenceCreateInfo& info)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    SharedPtr<gapi::Pipeline> GAPI_Metal::CreateGraphicsPipeline(const gapi::GraphicsPipelineCreateInfo& info)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    SharedPtr<gapi::Pipeline> GAPI_Metal::CreateComputePipeline(const gapi::ComputePipelineCreateInfo& info)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    SharedPtr<gapi::Shader> GAPI_Metal::CreateShader(const gapi::ShaderCreateInfo& info)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    SharedPtr<gapi::ShaderVariablesLayout> GAPI_Metal::CreateShaderVariablesLayout(const gapi::ShaderVariablesLayoutCreateInfo& info)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    SharedPtr<gapi::Viewport> GAPI_Metal::CreateViewport(const gapi::ViewportCreateInfo& info)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    gapi::TimestampList GAPI_Metal::GetLastTimestampList()
    {
        NOT_IMPLEMENTED();
        return {};
    }
} // namespace cube

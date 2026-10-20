#include "RayTracingDebugView.h"

#include "Allocator/FrameAllocator.h"
#include "Engine.h"
#include "RenderCore/RenderGraph.h"
#include "Renderer/Renderer.h"
#include "Scene/Scene.h"

namespace cube
{
    class RayTracingDebugViewShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(RayTracingDebugViewShaderParameterList)
            CUBE_SHADER_PARAMETER(Float4, bufferSizeAndInvSize)
            CUBE_SHADER_PARAMETER(BindlessTLAS, tlas)
            CUBE_SHADER_PARAMETER(RGTextureUAVHandle, outColorTexture)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(RayTracingDebugViewShaderParameterList);

    RayTracingDebugView::RayTracingDebugView(Renderer& renderer)
        : mRenderer(renderer)
    {
    }

    RayTracingDebugView::~RayTracingDebugView()
    {
    }

    void RayTracingDebugView::Initialize(bool supportHWRT)
    {
        mIsSupported = supportHWRT;
        if (!mIsSupported)
        {
            return;
        }

        platform::FilePath rayTracingDebugShaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("RayTracingDebug.slang");

        mRayTracingDebugViewShader = mRenderer.GetShaderManager().CreateShader(ShaderCreateInfo{
            .shaderInfo = ShaderInfo{
                .type = gapi::ShaderType::Compute,
                .entryPoint = "RayTracingDebugViewCS",
            },
            .filePaths = { &rayTracingDebugShaderFilePath, 1 },
            .debugName = CUBE_T("RayTracingDebugViewCS"),
        });
        CHECK(mRayTracingDebugViewShader);
        mRayTracingDebugViewPipelineInfo = ComputePipelineInfo{
            .shader = mRayTracingDebugViewShader,
        };
    }

    void RayTracingDebugView::Shutdown()
    {
        mRayTracingDebugViewPipelineInfo = {};
        mRayTracingDebugViewShader = nullptr;
    }

    bool RayTracingDebugView::RenderDebugView(RGBuilder& builder, const Scene& scene, RGTextureHandle colorTexture)
    {
        if (!mIsSupported)
        {
            return false;
        }

        SharedPtr<gapi::TLAS> tlas = scene.GetTLAS();
        if (!tlas)
        {
            return false;
        }

        const Uint32 width = colorTexture->GetTextureInfo().width;
        const Uint32 height = colorTexture->GetTextureInfo().height;

        RGTextureUAVHandle colorTextureUAV = builder.CreateUAV(colorTexture);
        RGAccelerationStructureHandle rgTLAS = builder.RegisterAccelerationStructure(scene.GetTLAS());

        auto blases = scene.GetBLASes();
        FrameVector<RGAccelerationStructureHandle> rgBLASes;
        rgBLASes.reserve(blases.size());
        for (SharedPtr<gapi::BLAS>& blas : blases)
        {
            rgBLASes.push_back(builder.RegisterAccelerationStructure(blas));
        }

        auto shaderParameters = builder.CreateShaderParameterList<RayTracingDebugViewShaderParameterList>();
        shaderParameters->bufferSizeAndInvSize = Float4(width, height, 1.0f / width, 1.0f / height);
        shaderParameters->tlas = BindlessTLAS{ .id = tlas->GetBindlessId() };
        shaderParameters->outColorTexture = colorTextureUAV;

        SharedPtr<ComputePipeline> rayTracingDebugViewPipeline = mRenderer.GetPipelineManager().GetOrCreateComputePipeline(ComputePipelineCreateInfo{
            .pipelineInfo = mRayTracingDebugViewPipelineInfo,
            .debugName = CUBE_T("RayTracingDebugViewPipeline"),
        });

        builder.AddPass(CUBE_T("RayTracingDebugView"),
            rayTracingDebugViewPipeline,
            shaderParameters,
            [width, height](gapi::CommandList& commandList)
            {
                commandList.DispatchThreads(width, height, 1);
            },
            [rgTLAS, rgBLASes](RGBuilder& builder)
            {
                builder.UseResource(rgTLAS, gapi::ResourceAccessFlag::ReadAccelerationStructure, gapi::ResourceSyncFlag::Compute);
                for (RGAccelerationStructureHandle blas : rgBLASes)
                {
                    builder.UseResource(blas, gapi::ResourceAccessFlag::ReadAccelerationStructure, gapi::ResourceSyncFlag::Compute);
                }
            });

        return true;
    }
} // namespace cube

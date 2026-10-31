#pragma once

#include "EngineHeader.h"

#include "RenderCore/Pipeline.h"
#include "RenderCore/RenderGraphTypes.h"

namespace cube
{
    namespace gapi
    {
        class RayTracingPipeline;
    } // namespace gapi
    class Renderer;
    class RGBuilder;
    class Scene;
    class Shader;

    class RayTracingDebugView
    {
    public:
        RayTracingDebugView(Renderer& renderer);
        ~RayTracingDebugView();
        
        void Initialize(bool supportHWRT, bool supportRayTracingPipeline);
        void Shutdown();

        bool RenderDebugView(RGBuilder& builder, const Scene& scene, RGTextureHandle colorTexture);

    private:
        bool RenderDebugViewRTPipeline(RGBuilder& builder, const Scene& scene, RGTextureHandle colorTexture);
        bool RenderDebugViewInline(RGBuilder& builder, const Scene& scene, RGTextureHandle colorTexture);

        Renderer& mRenderer;

        bool mIsSupported;
        bool mUseRayTracingPipeline;

        SharedPtr<Shader> mRayTracingDebugViewRayGenShader;
        SharedPtr<Shader> mRayTracingDebugViewClosestHitShader;
        SharedPtr<Shader> mRayTracingDebugViewMissShader;

        SharedPtr<Shader> mRayTracingDebugViewInlineShader;
        ComputePipelineInfo mRayTracingDebugViewInlinePipelineInfo;
    };
} // namespace cube

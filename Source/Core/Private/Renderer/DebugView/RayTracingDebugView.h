#pragma once

#include "CoreHeader.h"

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
        
        void Initialize(bool supportHWRT);
        void Shutdown();

        bool RenderDebugView(RGBuilder& builder, const Scene& scene, RGTextureHandle colorTexture);

    private:
        Renderer& mRenderer;

        bool mIsSupported;

        SharedPtr<Shader> mRayTracingDebugViewShader;
        ComputePipelineInfo mRayTracingDebugViewPipelineInfo;
    };
} // namespace cube

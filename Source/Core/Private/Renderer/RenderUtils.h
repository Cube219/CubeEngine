#pragma once

#include "CoreHeader.h"

#include "Pipeline.h"

namespace cube
{
    class Renderer;
    class RenderGraph;
    class Shader;

    class RenderUtils
    {
    public:
        RenderUtils(Renderer& renderer);
        ~RenderUtils() = default;

        void Initialize();
        void Shutdown();

        void CopyTexturePS(RGBuilder& builder, RGTextureHandle src, RGTextureHandle dst);

    private:
        Renderer& mRenderer;

        SharedPtr<Shader> mFullScreenVS;

        SharedPtr<Shader> mCopyTexturePS;
        GraphicsPipelineInfo mCopyTexturePSPipelineInfo;
    };
} // namespace cube

#pragma once

#include "CoreHeader.h"

#include "Pipeline.h"
#include "Renderer/RenderGraphTypes.h"

namespace cube
{
    class Shader;

    // Must match in Tonemap.slang
    enum class TonemapMode
    {
        Linear,
        PBRNeutral,

        Num
    };

    class Tonemap
    {
    public:
        Tonemap(Renderer& renderer);
        ~Tonemap() = default;

        void Initialize();
        void Shutdown();

        void OnLoopImGUIContent();

        void Execute(RGBuilder& builder, RGTextureHandle src, RGTextureHandle dst);

    private:
        Renderer& mRenderer;

        TonemapMode mMode = TonemapMode::Linear;
        bool mUseGammaCorrect = true;

        SharedPtr<Shader> mTonemappingShader;
        ComputePipelineInfo mTonemappingPipelineInfo;
    };
} // namespace cube

#include "Tonemap.h"

#include "RenderGraph.h"
#include "imgui.h"

#include "Renderer/ShaderParameter.h"

namespace cube
{
    class TonemapShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(TonemapShaderParameterList)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, srcTexture)
            CUBE_SHADER_PARAMETER(RGTextureUAVHandle, dstTexture)
            CUBE_SHADER_PARAMETER(Uint32, mode)
            CUBE_SHADER_PARAMETER(bool, useGammaCorrect)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(TonemapShaderParameterList);

    Tonemap::Tonemap(Renderer& renderer)
        : mRenderer(renderer)
    {
    }

    void Tonemap::Initialize()
    {
        platform::FilePath tonemapFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("Tonemap.slang");

        mTonemappingShader = mRenderer.GetShaderManager().CreateShader({
            .shaderInfo = {
                .type = gapi::ShaderType::Compute,
                .language = gapi::ShaderLanguage::Slang,
                .entryPoint = "TonemappingCS"
            },
            .filePaths = { &tonemapFilePath, 1 },
            .debugName = CUBE_T("TonemappingCS")
        });
        CHECK(mTonemappingShader);

        mTonemappingPipelineInfo = {
            .shader = mTonemappingShader
        };
    }
    
    void Tonemap::Shutdown()
    {
        mTonemappingPipelineInfo = {};
        mTonemappingShader = nullptr;
    }

    void Tonemap::OnLoopImGUIContent()
    {
        ImGui::SeparatorText("Tonemap");

        static auto GetTonemapModeStr = [](TonemapMode mode) -> const char*
        {
            switch (mode)
            {
            case TonemapMode::Linear: return "Linear";
            case TonemapMode::PBRNeutral: return "PBR Neutral";
            case TonemapMode::Num: return "Num";
            }
            return "";
        };

        ImGui::SetNextItemWidth(160);
        if (ImGui::BeginCombo("Mode", GetTonemapModeStr(mMode)))
        {
            for (Uint32 i = 0; i < static_cast<Uint32>(TonemapMode::Num); ++i)
            {
                const bool selected = static_cast<Uint32>(mMode) == i;
                const TonemapMode currentMode = static_cast<TonemapMode>(i);
                if (ImGui::Selectable(GetTonemapModeStr(currentMode), selected))
                {
                    mMode = currentMode;
                }

                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
        ImGui::Checkbox("Gamma Correct", &mUseGammaCorrect);
    }

    void Tonemap::Execute(RGBuilder& builder, RGTextureHandle src, RGTextureHandle dst)
    {
        RGTextureSRVHandle srcSRV = builder.CreateSRV(src);
        RGTextureUAVHandle dstUAV = builder.CreateUAV(dst);

        const Uint32 width = src->GetTextureInfo().width;
        const Uint32 height = src->GetTextureInfo().height;
        CHECK(width == dst->GetTextureInfo().width && height == dst->GetTextureInfo().height);

        auto params = builder.CreateShaderParameterList<TonemapShaderParameterList>();
        params->Get()->srcTexture = srcSRV;
        params->Get()->dstTexture = dstUAV;
        params->Get()->mode = static_cast<Uint32>(mMode);
        params->Get()->useGammaCorrect = mUseGammaCorrect;

        SharedPtr<ComputePipeline> tonemappingPipeline = mRenderer.GetPipelineManager().GetOrCreateComputePipeline({
            .pipelineInfo = mTonemappingPipelineInfo,
            .debugName = CUBE_T("Tonemapping Pipeline")
        });

        builder.AddPass(CUBE_T("Tonemapping"),
            tonemappingPipeline,
            params,
            [width, height](gapi::CommandList& commandList)
            {
                commandList.DispatchThreads(width, height, 1);
            },
            true
        );
    }
} // namespace cube

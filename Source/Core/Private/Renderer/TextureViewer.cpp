#include "TextureViewer.h"

#include "imgui.h"

#include "Allocator/FrameAllocator.h"
#include "CubeMath.h"
#include "Engine.h"
#include "RenderGraph.h"
#include "Renderer.h"
#include "Shader.h"

namespace cube
{
    class CopyToTextureViewerShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(CopyToTextureViewerShaderParameterList)
            CUBE_SHADER_PARAMETER(Vector4, dstSizeAndInvSize)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, srcTexture2D)
            CUBE_SHADER_PARAMETER(RGTextureUAVHandle, dstTexture)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(CopyToTextureViewerShaderParameterList);

    TextureViewer::TextureViewer(Renderer& renderer)
        : mRenderer(renderer)
    {
    }

    void TextureViewer::Initialize()
    {
        {
            platform::FilePath textureViewerShaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("TextureViewer.slang");

            mCopyToTextureViewer2DShader = mRenderer.GetShaderManager().CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Compute,
                    .entryPoint = "CopyToTextureViewer2DCS"
                },
                .filePaths = { &textureViewerShaderFilePath, 1 },
                .debugName = CUBE_T("CopyToTextureViewer2DCS")
            });
            CHECK(mCopyToTextureViewer2DShader);

            mCopyToTextureViewer2DPipeline = mRenderer.GetShaderManager().CreateComputePipeline({
                .pipelineInfo = {
                    .shader = mCopyToTextureViewer2DShader
                },
                .debugName = CUBE_T("CopyToTextureViewer2D Pipeline")
            });
        }
    }

    void TextureViewer::Shutdown()
    {
        mCopyToTextureViewer2DPipeline = nullptr;
        mCopyToTextureViewer2DShader = nullptr;

        mCopiedTextureSRV = nullptr;
        mCopiedTexture = nullptr;
    }

    void TextureViewer::OnLoopImGUI()
    {
        if (!mShow)
        {
            return;
        }

        if (ImGui::Begin("Texture Viewer", &mShow))
        {
            if (mIsCopied)
            {
                ImGui::Text("Name: %s", mName.c_str());
                ImGui::Text("Captured rendering frame: %llu", mRenderingFrame);
                switch (mTextureInfo.type)
                {
                case gapi::TextureType::Texture2D:
                {
                    ImGui::Text("Type: Texture2D");
                    ImGui::Text("Dimensions: %d x %d", mTextureInfo.width, mTextureInfo.height);
                    ImGui::Text("MipLevel: %d", mSubresourceRange.firstMipLevel);
                    break;
                }
                default:
                    break;
                }

                const float bottomTextLineSize = ImGui::GetTextLineHeightWithSpacing();
                const ImVec2 contentRegion = ImGui::GetContentRegionAvail();
                const ImVec2 canvasExtent = ImVec2(contentRegion.x, contentRegion.y - bottomTextLineSize);
                ImGui::BeginChild("##TextureCanvas", canvasExtent, ImGuiChildFlags_Borders);

                const ImVec2 canvasOrigin = ImGui::GetCursorScreenPos();
                const ImVec2 canvasSize = ImGui::GetContentRegionAvail();

                ImGui::InvisibleButton("##TextureCanvasHit", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
                const bool isHovered = ImGui::IsItemHovered();
                const bool isActive = ImGui::IsItemActive();

                ImGuiIO& io = ImGui::GetIO();

                if (isHovered && io.MouseWheel != 0.0f)
                {
                    const float oldZoom = mZoom;
                    float newZoom = oldZoom * std::pow(1.1f, io.MouseWheel);
                    newZoom = Math::Min(Math::Max(newZoom, 0.1f), 32.0f);

                    if (std::abs(newZoom - oldZoom) > std::numeric_limits<float>::epsilon())
                    {
                        const float canvasCenterX = canvasOrigin.x + canvasSize.x * 0.5f;
                        const float canvasCenterY = canvasOrigin.y + canvasSize.y * 0.5f;
                        const float imageCenterX = canvasCenterX + mPanOffsetX;
                        const float imageCenterY = canvasCenterY + mPanOffsetY;
                        const float scale = newZoom / oldZoom;
                        const float newImageCenterX = io.MousePos.x - (io.MousePos.x - imageCenterX) * scale;
                        const float newImageCenterY = io.MousePos.y - (io.MousePos.y - imageCenterY) * scale;
                        mPanOffsetX = newImageCenterX - canvasCenterX;
                        mPanOffsetY = newImageCenterY - canvasCenterY;
                        mZoom = newZoom;
                    }
                }

                if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f))
                {
                    mPanOffsetX += io.MouseDelta.x;
                    mPanOffsetY += io.MouseDelta.y;
                }

                const float scaledWidth = static_cast<float>(mCopiedTextureWidth) * mZoom;
                const float scaledHeight = static_cast<float>(mCopiedTextureHeight) * mZoom;
                const ImVec2 imageMin(
                    canvasOrigin.x + (canvasSize.x - scaledWidth) * 0.5f + mPanOffsetX,
                    canvasOrigin.y + (canvasSize.y - scaledHeight) * 0.5f + mPanOffsetY
                );
                const ImVec2 imageMax(imageMin.x + scaledWidth, imageMin.y + scaledHeight);

                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerNearest);
                drawList->AddImage(mCopiedTextureSRV->GetImTextureID(), imageMin, imageMax);
                drawList->AddCallback(ImDrawCallback_ResetRenderState);

                ImGui::EndChild();

                if (isHovered && scaledWidth > 0.0f && scaledHeight > 0.0f)
                {
                    const ImVec2 mousePos = io.MousePos;
                    const float u = (mousePos.x - imageMin.x) / scaledWidth;
                    const float v = (mousePos.y - imageMin.y) / scaledHeight;
                    if (u >= 0.0f && u < 1.0f && v >= 0.0f && v < 1.0f)
                    {
                        const int pixelX = static_cast<int>(u * static_cast<float>(mTextureInfo.width));
                        const int pixelY = static_cast<int>(v * static_cast<float>(mTextureInfo.height));
                        ImGui::Text("Pixel (%d, %d) / UV (%.4f, %.4f)", pixelX, pixelY, u, v);
                    }
                    else
                    {
                        ImGui::Text("Pixel / UV");
                    }
                }
                else
                {
                    ImGui::Text("Pixel / UV");
                }
                // TODO: Show pixel information
                // TODO: RGBA mask / color range
            }
            else
            {
                ImGui::Text("None of texture has been set.");
            }
        }
        ImGui::End();
    }

    void TextureViewer::SetTexture(RGBuilder& builder, RGTextureHandle texture, gapi::SubresourceRangeInput subresourceRange)
    {
        const gapi::TextureInfo& textureInfo = texture->GetTextureInfo();
        if (textureInfo.type != gapi::TextureType::Texture2D)
        {
            NOT_IMPLEMENTED();
        }

        CreateNewCopiedTextureIfNeeded(textureInfo.width, textureInfo.height);

        mName = String_Convert<AnsiString>(texture->GetDebugName());
        mRenderingFrame = mRenderer.GetCurrentRenderingFrame();
        mTextureInfo = textureInfo;
        mSubresourceRange = subresourceRange;

        RGTextureSRVHandle srcSRV = builder.CreateSRV(texture, subresourceRange.firstMipLevel, 1);
        RGTextureHandle dstTex = builder.RegisterTexture(mCopiedTexture);
        RGTextureUAVHandle dstUAV = builder.CreateUAV(dstTex);

        RGShaderParameterListHandle<CopyToTextureViewerShaderParameterList> params = builder.CreateShaderParameterList<CopyToTextureViewerShaderParameterList>();
        params->Get()->dstSizeAndInvSize = Vector4(
            static_cast<float>(mCopiedTextureWidth), static_cast<float>(mCopiedTextureHeight),
            1.0f / static_cast<float>(mCopiedTextureWidth), 1.0f / static_cast<float>(mCopiedTextureHeight)
        );
        params->Get()->srcTexture2D = srcSRV;
        params->Get()->dstTexture = dstUAV;

        builder.AddPass(
            Format<FrameString>(CUBE_T("TextureViewer CopyToTexture - {0}"), mName), mCopyToTextureViewer2DPipeline,
            RGBuilder::MakeParameterListArray(params),
            [width = mCopiedTextureWidth, height = mCopiedTextureHeight](gapi::CommandList& commandList){
                commandList.DispatchThreads(width, height, 1);
            }
        );

        mIsCopied = true;
    }

    void TextureViewer::CreateNewCopiedTextureIfNeeded(Uint32 width, Uint32 height)
    {
        if (mCopiedTexture && mCopiedTextureWidth == width && mCopiedTextureHeight == height)
        {
            return;
        }

        mCopiedTextureSRV = nullptr;
        mCopiedTexture = nullptr;
        mCopiedTextureWidth = width;
        mCopiedTextureHeight = height;

        mCopiedTexture = mRenderer.GetGAPI().CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .textureInfo = {
                .format = gapi::ElementFormat::RGBA8_UNorm,
                .type = gapi::TextureType::Texture2D,
                .flags = gapi::TextureFlag::UAV,
                .width = width,
                .height = height
            },
            .debugName = CUBE_T("TextureViewer CopiedTexture")
        });
        mCopiedTextureSRV = mCopiedTexture->CreateSRV({});
    }
} // namespace cube

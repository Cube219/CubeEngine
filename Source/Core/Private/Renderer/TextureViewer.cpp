#include "TextureViewer.h"

#include "imgui.h"

#include "Allocator/FrameAllocator.h"
#include "CubeMath.h"
#include "Engine.h"
#include "Platform.h"
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

    class CopyToTextureViewerCubeShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(CopyToTextureViewerCubeShaderParameterList)
            CUBE_SHADER_PARAMETER(Vector4, srcSizeAndInvSize)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, srcTextureCube)
            CUBE_SHADER_PARAMETER(RGTextureUAVHandle, dstTexture)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(CopyToTextureViewerCubeShaderParameterList);

    class TextureViewerFetchInfoShaderParameterList : public ShaderParameterList
    {
        CUBE_BEGIN_SHADER_PARAMETER_LIST(TextureViewerFetchInfoShaderParameterList)
            CUBE_SHADER_PARAMETER(Vector4, sizeAndInvSize)
            CUBE_SHADER_PARAMETER(RGTextureSRVHandle, canvasTexture)
            CUBE_SHADER_PARAMETER(int, positionToReadX)
            CUBE_SHADER_PARAMETER(int, positionToReadY)
            CUBE_SHADER_PARAMETER(RGBufferUAVHandle, readbackBuffer)
        CUBE_END_SHADER_PARAMETER_LIST
    };
    CUBE_REGISTER_SHADER_PARAMETER_LIST(TextureViewerFetchInfoShaderParameterList);

    TextureViewer::TextureViewer(Renderer& renderer)
        : mRenderer(renderer)
    {
    }

    void TextureViewer::Initialize(Uint32 numGPUSync)
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

            mCopyToTextureViewer2DPipelineInfo = {
                .shader = mCopyToTextureViewer2DShader
            };
        }

        {
            platform::FilePath textureViewerShaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("TextureViewerCube.slang");

            mCopyToTextureViewerCubeShader = mRenderer.GetShaderManager().CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Compute,
                    .entryPoint = "CopyToTextureViewerCubeCS"
                },
                .filePaths = { &textureViewerShaderFilePath, 1 },
                .debugName = CUBE_T("CopyToTextureViewerCube CS")
            });
            CHECK(mCopyToTextureViewerCubeShader);

            mCopyToTextureViewerCubePipelineInfo = {
                .shader = mCopyToTextureViewerCubeShader
            };
        }

        {
            platform::FilePath textureViewerShaderFilePath = Engine::GetShaderDirectoryPath() / CUBE_T("TextureViewer2.slang");

            mFetchInfoShader = mRenderer.GetShaderManager().CreateShader({
                .shaderInfo = {
                    .type = gapi::ShaderType::Compute,
                    .entryPoint = "TextureViewerFetchInfoCS"
                },
                .filePaths = { &textureViewerShaderFilePath, 1 },
                .debugName = CUBE_T("TextureViewerFetchInfoCS")
            });
            CHECK(mFetchInfoShader);

            mFetchInfoPipelineInfo = {
                .shader = mFetchInfoShader
            };
        }

        mReadbackBuffers.resize(numGPUSync);
        for (Uint32 i = 0; i < mReadbackBuffers.size(); ++i)
        {
            ReadbackBuffer& readbackBuffer = mReadbackBuffers[i];
            readbackBuffer.buffer = mRenderer.GetGAPI().CreateBuffer({
                .usage = gapi::ResourceUsage::GPUtoCPU,
                .bufferInfo = {
                    .type = gapi::BufferType::Typed,
                    .size = ReadbackBuffer::totalSize,
                    .stride = ReadbackBuffer::stride,
                    .flags = gapi::BufferFlag::UAV
                },
                .debugName = Format<FrameString>(CUBE_T("TextureViewer Readback Buffer {0}"), i)
            });
            readbackBuffer.pData = platform::Platform::Allocate(ReadbackBuffer::totalSize);
        }

        mCurrentFrameIndex = 0;

        mCanvasTexture = nullptr;
        mCanvasTextureSize = { 0, 0 };
        mCanvasMipLevel = 0;
    }

    void TextureViewer::Shutdown()
    {
        for (ReadbackBuffer& readbackBuffer : mReadbackBuffers)
        {
            platform::Platform::Free(readbackBuffer.pData);
        }
        mReadbackBuffers.clear();

        mFetchInfoPipelineInfo = {};
        mFetchInfoShader = nullptr;
        mCopyToTextureViewerCubePipelineInfo = {};
        mCopyToTextureViewerCubeShader = nullptr;
        mCopyToTextureViewer2DPipelineInfo = {};
        mCopyToTextureViewer2DShader = nullptr;

        mCanvasTextureSRV = nullptr;
        mCanvasTexture = nullptr;

        mCopiedTexture = nullptr;
    }

    void TextureViewer::OnLoopImGUI()
    {
        if (!mShow)
        {
            return;
        }

        ImGui::SetNextWindowPos({ 400, 160 }, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize({ 600, 630 }, ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Texture Viewer", &mShow))
        {
            if (mCopiedTexture)
            {
                const gapi::TextureInfo& textureInfo = mCopiedTexture->GetInfo();

                ImGui::Text("Name: %s", mCopiedTextureName.c_str());
                ImGui::Text("Captured rendering frame: %llu", mCopiedRenderingFrame);
                switch (textureInfo.type)
                {
                case gapi::TextureType::Texture2D:
                {
                    ImGui::Text("Type: Texture2D");
                    ImGui::Text("Dimensions: %d x %d", textureInfo.width, textureInfo.height);
                    ImGui::Text("MipLevels: %d", textureInfo.mipLevels);
                    break;
                }
                case gapi::TextureType::TextureCube:
                {
                    ImGui::Text("Type: TextureCube");
                    ImGui::Text("Dimensions: %d x %d", textureInfo.width, textureInfo.height);
                    ImGui::Text("MipLevels: %d", textureInfo.mipLevels);
                    break;
                }
                default:
                    break;
                }
                ImGui::SetNextItemWidth(120);
                ImGui::SliderInt("MipLevel", &mMipLevel, 0, textureInfo.mipLevels - 1);

                const float bottomTextLineSize = ImGui::GetTextLineHeightWithSpacing();
                const ImVec2 contentRegion = ImGui::GetContentRegionAvail();
                const ImVec2 canvasExtent = ImVec2(contentRegion.x, contentRegion.y - bottomTextLineSize * 2);
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
                    newZoom = Math::Min(Math::Max(newZoom, 0.02f), 32.0f);

                    if (std::abs(newZoom - oldZoom) > std::numeric_limits<float>::epsilon())
                    {
                        const Float2 canvasCenter = { canvasOrigin.x + canvasSize.x * 0.5f, canvasOrigin.y + canvasSize.y * 0.5f };
                        const Float2 imageCenter = canvasCenter + mPanOffset;
                        const Float2 mousePos = { io.MousePos.x, io.MousePos.y };
                        const float scale = newZoom / oldZoom;
                        const Float2 newImageCenter = mousePos - (mousePos - imageCenter) * scale;
                        mPanOffset = newImageCenter - canvasCenter;
                        mZoom = newZoom;
                    }
                }

                if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f))
                {
                    mPanOffset += Float2{ io.MouseDelta.x, io.MouseDelta.y };
                }

                const float scaledWidth = static_cast<float>(mCanvasTextureSize.x) * mZoom;
                const float scaledHeight = static_cast<float>(mCanvasTextureSize.y) * mZoom;
                const Float2 imageMin(
                    canvasOrigin.x + (canvasSize.x - scaledWidth) * 0.5f + mPanOffset.x,
                    canvasOrigin.y + (canvasSize.y - scaledHeight) * 0.5f + mPanOffset.y
                );
                const Float2 imageMax(imageMin.x + scaledWidth, imageMin.y + scaledHeight);

                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->AddCallback(ImGui::GetPlatformIO().DrawCallback_SetSamplerNearest);
                drawList->AddImage(mCanvasTextureSRV->GetImTextureID(), { imageMin.x, imageMin.y }, { imageMax.x, imageMax.y });
                drawList->AddCallback(ImDrawCallback_ResetRenderState);

                ImGui::EndChild();

                if (isHovered && scaledWidth > 0.0f && scaledHeight > 0.0f)
                {
                    ImGUIPixelInfo({ io.MousePos.x, io.MousePos.y }, imageMin, imageMax);
                }
                else
                {
                    ImGUIPixelInfo({ -1, -1 }, imageMin, imageMax);
                }
                // TODO: Select array index.
                // TODO: RGBA mask / color range.
            }
            else
            {
                ImGui::Text("None of texture has been set.");
            }
        }
        ImGui::End();
    }

    void TextureViewer::MoveToNextFrame()
    {
        mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mReadbackBuffers.size();

        ReadbackBuffer& currentReadbackBuffer = GetCurrentReadbackBuffer();
        void* mappedPtr = currentReadbackBuffer.buffer->Map();
        memcpy(currentReadbackBuffer.pData, mappedPtr, ReadbackBuffer::totalSize);
        currentReadbackBuffer.buffer->Unmap();

        ProcessReadbackInfo();
    }

    void TextureViewer::SetTexture(RGBuilder& builder, RGTextureHandle texture)
    {
        const gapi::TextureInfo& textureInfo = texture->GetTextureInfo();
        if (textureInfo.type != gapi::TextureType::Texture2D && textureInfo.type != gapi::TextureType::TextureCube)
        {
            NOT_IMPLEMENTED();
        }

        CreateNewCanvasTextureIfNeeded(textureInfo);

        mCopiedTextureName = String_Convert<AnsiString>(texture->GetDebugName());
        mCopiedRenderingFrame = mRenderer.GetCurrentRenderingFrame();

        mCopiedTexture = mRenderer.GetGAPI().CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .textureInfo = textureInfo,
            .debugName = CUBE_T("TextureViewer CopiedTexture")
        });

        RGTextureHandle copiedTexture = builder.RegisterTexture(mCopiedTexture);
        builder.AddPass(Format<FrameString>(CUBE_T("TextureViewer CopyToTexture - {0}"), mCopiedTextureName),
            [src = texture, dst = copiedTexture](gapi::CommandList& commandList)
            {
                commandList.CopyTexture(src->GetGAPITexture(), dst->GetGAPITexture());
            }, [src = texture, dst = copiedTexture](RGBuilder& builder)
            {
                builder.UseResource(src, {}, gapi::ResourceStateFlag::CopySrc);
                builder.UseResource(dst, {}, gapi::ResourceStateFlag::CopyDst);
            },
            false, true
        );

        DrawToCanvasTextureIfNeeded(builder, true);
    }

    void TextureViewer::Update(RGBuilder& builder)
    {
        if (!mShow || !mCopiedTexture)
        {
            return;
        }

        DrawToCanvasTextureIfNeeded(builder);
        FetchInfo(builder);
    }

    void TextureViewer::ImGUIPixelInfo(Float2 mousePos, Float2 imageMin, Float2 imageMax)
    {
        CHECK(mCopiedTexture);
        const gapi::TextureInfo& textureInfo = mCopiedTexture->GetInfo();

        bool pixelPrinted = false;
        bool colorPrinted = false;

        if (mousePos.x >= 0 && mousePos.y >= 0)
        {
            const float imageWidth = imageMax.x - imageMin.x;
            const float imageHeight = imageMax.y - imageMin.y;
            const float u = (mousePos.x - imageMin.x) / imageWidth;
            const float v = (mousePos.y - imageMin.y) / imageHeight;
            if (u >= 0.0f && u < 1.0f && v >= 0.0f && v < 1.0f)
            {
                const float bottomTextLineSize = ImGui::GetTextLineHeightWithSpacing();

                mPixel = {
                    static_cast<int>(u * static_cast<float>(mCanvasTextureSize.x)),
                    static_cast<int>(v * static_cast<float>(mCanvasTextureSize.y))
                };

                const Uint32 canvasMipWidth = std::max(1u, mCanvasTextureSize.x >> mMipLevel);
                const Uint32 canvasMipHeight = std::max(1u, mCanvasTextureSize.y >> mMipLevel);

                const int pixelMipX = static_cast<int>(u * static_cast<float>(canvasMipWidth));
                const int pixelMipY = static_cast<int>(v * static_cast<float>(canvasMipHeight));
                const float mipU = (static_cast<float>(pixelMipX) + 0.5f) / static_cast<float>(canvasMipWidth);
                const float mipV = (static_cast<float>(pixelMipY) + 0.5f) / static_cast<float>(canvasMipHeight);

                if (textureInfo.type == gapi::TextureType::TextureCube)
                {
                    const Uint32 faceWidth = textureInfo.width;
                    const Uint32 faceHeight = textureInfo.height;

                    static const Uint32 cubeMultiplierX[6] = { 2, 0, 1, 1, 1, 3 };
                    static const Uint32 cubeMultiplierY[6] = { 1, 1, 0, 2, 1, 1 };
                    int faceIndex = -1;
                    for (int i = 0; i < 6; ++i)
                    {
                        const Uint32 faceMinX = faceWidth * cubeMultiplierX[i];
                        const Uint32 faceMinY = faceHeight * cubeMultiplierY[i];
                        if ((static_cast<int>(faceMinX) <= mPixel.x && mPixel.x < static_cast<int>(faceMinX + faceWidth))
                            && (static_cast<int>(faceMinY) <= mPixel.y && mPixel.y < static_cast<int>(faceMinY + faceHeight)))
                        {
                            faceIndex = i;
                            break;
                        }
                    }

                    if (faceIndex != -1)
                    {
                        const Uint32 faceMinX = faceWidth * cubeMultiplierX[faceIndex];
                        const Uint32 faceMinY = faceHeight * cubeMultiplierY[faceIndex];
                        const Uint32 faceX = mPixel.x - faceMinX;
                        const Uint32 faceY = mPixel.y - faceMinY;
                        const float faceU = (static_cast<float>(faceX) + 0.5f) / static_cast<float>(faceWidth);
                        const float faceV = (static_cast<float>(faceY) + 0.5f) / static_cast<float>(faceHeight);
                        static const char* faceStr[6] = { "+X", "-X", "+Y", "-Y", "+Z", "-Z" };

                        const Uint32 faceMipWidth = std::max(1u, faceWidth >> mMipLevel);
                        const Uint32 faceMipHeight = std::max(1u, faceHeight >> mMipLevel);
                        const int faceMipX = static_cast<int>(faceU * static_cast<float>(faceMipWidth));
                        const int faceMipY = static_cast<int>(faceV * static_cast<float>(faceMipHeight));
                        const float faceMipU = (static_cast<float>(faceMipX) + 0.5f) / static_cast<float>(faceMipWidth);
                        const float faceMipV = (static_cast<float>(faceMipY) + 0.5f) / static_cast<float>(faceMipHeight);

                        ImGui::Text("Pixel (%d, %d, %s) / UV (%.4f, %.4f)", faceMipX, faceMipY, faceStr[faceIndex], faceMipU, faceMipV);
                        pixelPrinted = true;
                    }
                }
                else
                {
                    ImGui::Text("Pixel (%d, %d) / UV (%.4f, %.4f)", pixelMipX, pixelMipY, mipU, mipV);
                    pixelPrinted = true;
                }

                if (pixelPrinted)
                {
                    ImVec4 color = { mReadbackInfo.color.x, mReadbackInfo.color.y, mReadbackInfo.color.z, mReadbackInfo.color.w };
                    ImGui::Text("R(%.4f) / G(%.4f) / B(%.4f) / A(%.4f)", color.x, color.y, color.z, color.w);
                    ImGui::SameLine();
                    ImGui::ColorButton("##PixelColor", *(ImVec4*)&color, ImGuiColorEditFlags_None, ImVec2(bottomTextLineSize - 5, bottomTextLineSize - 5));
                    colorPrinted = true;
                }
            }
        }

        if (!pixelPrinted)
        {
            ImGui::Text("Pixel / UV");
            mPixel = { -1, -1 };
        }

        if (!colorPrinted)
        {
            ImGui::Text("R / G / B / A");
        }
    }

    void TextureViewer::FetchInfo(RGBuilder& builder)
    {
        CHECK(mCanvasTexture);

        RGTextureHandle canvasTexture = builder.RegisterTexture(mCanvasTexture);
        RGTextureSRVHandle canvasSRV = builder.CreateSRV(canvasTexture);
        RGBufferHandle readbackBuffer = builder.RegisterBuffer(GetCurrentReadbackBuffer().buffer);
        RGBufferUAVHandle readbackUAV = builder.CreateUAV(readbackBuffer, ReadbackBuffer::format);

        auto params = builder.CreateShaderParameterList<TextureViewerFetchInfoShaderParameterList>();
        params->Get()->sizeAndInvSize = Vector4(
            static_cast<float>(mCanvasTextureSize.x), static_cast<float>(mCanvasTextureSize.y),
            1.0f / static_cast<float>(mCanvasTextureSize.x), 1.0f / static_cast<float>(mCanvasTextureSize.y)
        );
        params->Get()->canvasTexture = canvasSRV;
        params->Get()->positionToReadX = mPixel.x;
        params->Get()->positionToReadY = mPixel.y;
        params->Get()->readbackBuffer = readbackUAV;

        SharedPtr<ComputePipeline> fetchInfoPipeline = mRenderer.GetPipelineManager().GetOrCreateComputePipeline({
            .pipelineInfo = mFetchInfoPipelineInfo,
            .debugName = CUBE_T("TextureViewerFetchInfoCS Pipeline")
        });
        builder.AddPass(
            Format<FrameString>(CUBE_T("TextureViewer FetchInfo - {0}"), mCopiedTextureName),
            fetchInfoPipeline,
            RGBuilder::MakeParameterListArray(params),
            [](gapi::CommandList& commandList)
            {
                commandList.DispatchThreads(1, 1, 1);
            }
        );
    }

    void TextureViewer::CreateNewCanvasTextureIfNeeded(const gapi::TextureInfo& info)
    {
        Uint32 newWidth = info.width;
        Uint32 newHeight = info.height;
        if (info.type == gapi::TextureType::TextureCube)
        {
            newWidth *= 4;
            newHeight *= 3;
        }

        if (mCanvasTexture && mCanvasTextureSize == Uint2{ newWidth, newHeight })
        {
            return;
        }

        mCanvasTextureSRV = nullptr;
        mCanvasTexture = nullptr;
        mCanvasTextureSize = { newWidth, newHeight };
        mCanvasMipLevel = 0;

        mCanvasTexture = mRenderer.GetGAPI().CreateTexture({
            .usage = gapi::ResourceUsage::GPUOnly,
            .textureInfo = {
                .format = gapi::ElementFormat::RGBA8_UNorm,
                .type = gapi::TextureType::Texture2D,
                .flags = gapi::TextureFlag::UAV,
                .width = newWidth,
                .height = newHeight
            },
            .debugName = CUBE_T("TextureViewer CanvasTexture")
        });
        mCanvasTextureSRV = mCanvasTexture->CreateSRV({});
    }

    void TextureViewer::DrawToCanvasTextureIfNeeded(RGBuilder& builder, bool force)
    {
        CHECK(mCopiedTexture);
        CHECK(mCanvasTexture);
        const gapi::TextureInfo textureInfo = mCopiedTexture->GetInfo();

        mMipLevel = std::min(mMipLevel, static_cast<int>(textureInfo.mipLevels) - 1);

        if (!force && mMipLevel == static_cast<int>(mCanvasMipLevel))
        {
            return;
        }

        RGTextureHandle src = builder.RegisterTexture(mCopiedTexture);
        RGTextureSRVHandle srcSRV = builder.CreateSRV(src, mMipLevel, 1);
        RGTextureHandle dstTex = builder.RegisterTexture(mCanvasTexture);
        RGTextureUAVHandle dstUAV = builder.CreateUAV(dstTex);

        if (textureInfo.type == gapi::TextureType::Texture2D)
        {
            auto params = builder.CreateShaderParameterList<CopyToTextureViewerShaderParameterList>();
            params->Get()->dstSizeAndInvSize = Vector4(
                static_cast<float>(mCanvasTextureSize.x), static_cast<float>(mCanvasTextureSize.y),
                1.0f / static_cast<float>(mCanvasTextureSize.x), 1.0f / static_cast<float>(mCanvasTextureSize.y)
            );
            params->Get()->srcTexture2D = srcSRV;
            params->Get()->dstTexture = dstUAV;

            SharedPtr<ComputePipeline> copyToTextureViewer2DPipeline = mRenderer.GetPipelineManager().GetOrCreateComputePipeline({
                .pipelineInfo = mCopyToTextureViewer2DPipelineInfo,
                .debugName = CUBE_T("CopyToTextureViewer2D Pipeline")
            });
            builder.AddPass(
                Format<FrameString>(CUBE_T("TextureViewer CopyToCanvas - {0}"), mCopiedTextureName), copyToTextureViewer2DPipeline,
                RGBuilder::MakeParameterListArray(params),
                [size = mCanvasTextureSize](gapi::CommandList& commandList){
                    commandList.DispatchThreads(size.x, size.y, 1);
                }
            );
        }
        else if (textureInfo.type == gapi::TextureType::TextureCube)
        {
            const Uint32 srcWidth = textureInfo.width;
            const Uint32 srcHeight = textureInfo.height;

            auto params = builder.CreateShaderParameterList<CopyToTextureViewerCubeShaderParameterList>();
            params->Get()->srcSizeAndInvSize = Vector4(
                static_cast<float>(srcWidth), static_cast<float>(srcHeight),
                1.0f / static_cast<float>(srcWidth), 1.0f / static_cast<float>(srcHeight)
            );
            params->Get()->srcTextureCube = srcSRV;
            params->Get()->dstTexture = dstUAV;

            SharedPtr<ComputePipeline> copyToTextureViewerCubePipeline = mRenderer.GetPipelineManager().GetOrCreateComputePipeline({
                .pipelineInfo = mCopyToTextureViewerCubePipelineInfo,
                .debugName = CUBE_T("CopyToTextureViewerCube Pipeline")
            });
            builder.AddPass(
                Format<FrameString>(CUBE_T("TextureViewer CopyToCanvas - {0}"), mCopiedTextureName), copyToTextureViewerCubePipeline,
                RGBuilder::MakeParameterListArray(params),
                [width = srcWidth, height = srcHeight](gapi::CommandList& commandList){
                    commandList.DispatchThreads(width, height, 6);
                }
            );
        }

        mCanvasMipLevel = mMipLevel;
    }

    void TextureViewer::ProcessReadbackInfo()
    {
        Byte* data = reinterpret_cast<Byte*>(GetCurrentReadbackBuffer().pData);
        memcpy(&mReadbackInfo.color.x, data, sizeof(float));
        memcpy(&mReadbackInfo.color.y, data + 4, sizeof(float));
        memcpy(&mReadbackInfo.color.z, data + 8, sizeof(float));
        memcpy(&mReadbackInfo.color.w, data + 12, sizeof(float));
    }
} // namespace cube

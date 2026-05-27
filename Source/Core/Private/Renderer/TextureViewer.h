#pragma once

#include "CoreHeader.h"

#include "Pipeline.h"
#include "Renderer/RenderGraphTypes.h"

namespace cube
{
    class ComputePipeline;
    class Renderer;
    class RGBuilder;
    class Shader;

    namespace gapi
    {
        class Buffer;
        class Texture;
        class TextureSRV;
    } // namespace gapi

    class TextureViewer
    {
    public:
        TextureViewer(Renderer& renderer);
        ~TextureViewer() = default;

        void Initialize(Uint32 numGPUSync);
        void Shutdown();

        void OnLoopImGUI();

        void MoveToNextFrame();

        void Show() { mShow = true; }
        void SetTexture(RGBuilder& builder, RGTextureHandle texture);

        void Update(RGBuilder& builder);

    private:
        void ImGUIPixelInfo(Float2 mousePos, Float2 imageMin, Float2 imageMax);
        void FetchInfo(RGBuilder& builder);

        void CreateNewCanvasTextureIfNeeded(const gapi::TextureInfo& info);
        void DrawToCanvasTextureIfNeeded(RGBuilder& builder, bool force = false);

        void ProcessReadbackInfo();

        Renderer& mRenderer;

        SharedPtr<Shader> mCopyToTextureViewer2DShader;
        ComputePipelineInfo mCopyToTextureViewer2DPipelineInfo;
        SharedPtr<Shader> mCopyToTextureViewerCubeShader;
        ComputePipelineInfo mCopyToTextureViewerCubePipelineInfo;
        SharedPtr<Shader> mFetchInfo2DShader;
        ComputePipelineInfo mFetchInfo2DPipelineInfo;
        SharedPtr<Shader> mFetchInfoCubeShader;
        ComputePipelineInfo mFetchInfoCubePipelineInfo;

        bool mShow = false;
        Uint32 mCurrentFrameIndex = 0;

        Uint64 mCopiedRenderingFrame;
        AnsiString mCopiedTextureName;
        SharedPtr<gapi::Texture> mCopiedTexture;

        Uint2 mCanvasTextureSize;
        Uint32 mCanvasMipLevel;
        Uint4 mCanvasRGBAMask;
        Float2 mCanvasColorRange;
        SharedPtr<gapi::Texture> mCanvasTexture;
        SharedPtr<gapi::TextureSRV> mCanvasTextureSRV;

        float mZoom = 1.0f;
        Float2 mPanOffset = { 0.0f, 0.0f };
        Int2 mPixel = { -1, -1 };
        int mMipLevel = 0;
        bool mMaskR = true;
        bool mMaskG = true;
        bool mMaskB = true;
        bool mMaskA = true;
        float mRangeMin = 0.0f;
        float mRangeMax = 1.0f;

        // TODO: Use info queue to get data immediately?
        struct ReadbackBuffer
        {
            constexpr static gapi::ElementFormat format = gapi::ElementFormat::R32_Float;
            constexpr static Uint32 stride = 4;
            constexpr static Uint32 numElement = 4;
            constexpr static Uint32 totalSize = numElement * stride;

            SharedPtr<gapi::Buffer> buffer;
            void* pData = nullptr;
        };
        ReadbackBuffer& GetCurrentReadbackBuffer() { return mReadbackBuffers[mCurrentFrameIndex]; }
        Vector<ReadbackBuffer> mReadbackBuffers;
        struct ReadbackInfo
        {
            Float4 color;
        };
        ReadbackInfo mReadbackInfo;
    };
} // namespace cube

#pragma once

#include "CoreHeader.h"

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
        void SetTexture(RGBuilder& builder, RGTextureHandle texture, gapi::SubresourceRangeInput subresourceRange);

        void FetchInfo(RGBuilder& builder);

    private:
        void CreateNewCopiedTextureIfNeeded(Uint32 width, Uint32 height);
        void ProcessReadbackInfo();

        Renderer& mRenderer;

        SharedPtr<Shader> mCopyToTextureViewer2DShader;
        SharedPtr<ComputePipeline> mCopyToTextureViewer2DPipeline;
        SharedPtr<Shader> mFetchInfoShader;
        SharedPtr<ComputePipeline> mFetchInfoPipeline;

        bool mShow = false;

        Uint32 mCopiedTextureWidth;
        Uint32 mCopiedTextureHeight;
        SharedPtr<gapi::Texture> mCopiedTexture;
        SharedPtr<gapi::TextureSRV> mCopiedTextureSRV;

        Uint32 mCurrentFrameIndex = 0;

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

        float mZoom = 1.0f;
        float mPanOffsetX = 0.0f;
        float mPanOffsetY = 0.0f;
        int mPixelX = -1;
        int mPixelY = -1;

        bool mIsCopied = false;
        AnsiString mName;
        Uint64 mRenderingFrame;
        gapi::TextureInfo mTextureInfo;
        gapi::SubresourceRangeInput mSubresourceRange;
    };
} // namespace cube

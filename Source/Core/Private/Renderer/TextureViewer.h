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

        void Initialize();
        void Shutdown();

        void OnLoopImGUI();

        void Show() { mShow = true; }
        void SetTexture(RGBuilder& builder, RGTextureHandle texture, gapi::SubresourceRangeInput subresourceRange);

    private:
        void CreateNewCopiedTextureIfNeeded(Uint32 width, Uint32 height);

        Renderer& mRenderer;

        SharedPtr<Shader> mCopyToTextureViewer2DShader;
        SharedPtr<ComputePipeline> mCopyToTextureViewer2DPipeline;

        bool mShow = false;

        Uint32 mCopiedTextureWidth;
        Uint32 mCopiedTextureHeight;
        SharedPtr<gapi::Texture> mCopiedTexture;
        SharedPtr<gapi::TextureSRV> mCopiedTextureSRV;

        float mZoom = 1.0f;
        float mPanOffsetX = 0.0f;
        float mPanOffsetY = 0.0f;

        bool mIsCopied = false;
        AnsiString mName;
        Uint64 mRenderingFrame;
        gapi::TextureInfo mTextureInfo;
        gapi::SubresourceRangeInput mSubresourceRange;
    };
} // namespace cube

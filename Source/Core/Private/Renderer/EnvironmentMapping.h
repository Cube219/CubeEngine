#pragma once

#include "CoreHeader.h"

#include "Renderer/RenderGraphTypes.h"

namespace cube
{
    class GraphicsPipeline;
    class Renderer;
    class RGBuilder;
    class Shader;
    class TextureResource;

    class EnvironmentMapping
    {
    public:
        EnvironmentMapping(Renderer& renderer);
        ~EnvironmentMapping() = default;

        void Initialize(bool enable);
        void Shutdown();

        void LoadResources();
        void ClearReaources();

        bool IsSupported() const { return mIBLTexture != nullptr; }
        bool IsEnabled() const { return mIsEnabled; }

        void SetEnable(bool newEnable);
        void RecreateGraphicsPipelines();

        void DrawSkybox(RGBuilder& builder);

        // Return black cube texture if disabled.
        RGTextureSRVHandle GetDiffuseIrradianceMap(RGBuilder& builder) const;

    private:
        Renderer& mRenderer;

        bool mIsEnabled;

        SharedPtr<TextureResource> mIBLTexture;
        SharedPtr<Shader> mSkyboxVS;
        SharedPtr<Shader> mSkyboxPS;
        SharedPtr<GraphicsPipeline> mSkyboxPipeline;

        SharedPtr<gapi::Texture> mDiffuseIrradianceMap;
    };
} // namespace cube

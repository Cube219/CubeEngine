#pragma once

#include "CoreHeader.h"

#include "EnvironmentMapping.h"
#include "Light.h"

namespace cube
{
    class GAPI;
    class Renderer;
    class TextureResource;

    // Must match PointLightInfo in Light.slang.
    struct PointLightInfo
    {
        Float3 position;
        float pad0;
        Float3 intensity;
        float pad1;
    };

    struct RectLightInfo
    {
        Float3 position;
        float pad0;
        Float3 direction;
        float pad1;
        Float2 rectSize;
        float pad2;
        float pad3;
        Float3 intensity;
        float pad4;
    };

    class LightManager
    {
    public:
        LightManager(Renderer& renderer);
        ~LightManager() = default;

        void Initialize();
        void Shutdown();

        void LoadResources();
        void ClearResources();

        void OnLoopImGUIContent();

        EnvironmentMapping& GetEnvironmentMapping() { return mEnvironmentMapping; }

        void UpdateLightInfoBuffers(RGBuilder& builder);

        void BindLightShaderParameterList(RGBuilder& builder);
        void UnbindLightShaderParameterList(RGBuilder& builder);

    private:
        void LoadLTCTexture();

        void AddPointLight(const Float3& position, const Float3& intensity);
        void AddRectLight(const Float3& position, const Float3& direction, const Float2& rectSize, const Float3& intensity);

        Renderer& mRenderer;

        SharedPtr<TextureResource> mLTCTexture1;
        SharedPtr<TextureResource> mLTCTexture2;

        DirectionalLight mDirectionalLight;

        static constexpr Uint32 MAX_POINT_LIGHTS = 16;
        static constexpr Uint32 MAX_RECT_LIGHTS = 16;

        Vector<PointLight> mPointLights;
        Uint32 mNumActivePointLights = 0;
        Vector<RectLight> mRectLights;
        Uint32 mNumActiveRectLights = 0;

        SharedPtr<gapi::Buffer> mPointLightInfoGPUBuffer;
        SharedPtr<gapi::Buffer> mRectLightInfoGPUBuffer;

        EnvironmentMapping mEnvironmentMapping;
    };
} // namespace cube

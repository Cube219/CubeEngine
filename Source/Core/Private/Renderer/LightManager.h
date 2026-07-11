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
        void AddPointLight(const Float3& position, const Float3& intensity);

        Renderer& mRenderer;

        DirectionalLight mDirectionalLight;

        static constexpr Uint32 MAX_POINT_LIGHTS = 16;

        Vector<PointLight> mPointLights;
        Uint32 mNumActivePointLights = 0;

        SharedPtr<gapi::Buffer> mPointLightInfoGPUBuffer;

        EnvironmentMapping mEnvironmentMapping;
    };
} // namespace cube

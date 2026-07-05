#pragma once

#include "CoreHeader.h"

#include "EnvironmentMapping.h"

namespace cube
{
    class GAPI;
    class Renderer;
    class TextureResource;

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

        void BindLightShaderParameterList(RGBuilder& builder);
        void UnbindLightShaderParameterList(RGBuilder& builder);

    private:
        Renderer& mRenderer;

        bool mIsDirectionalLightEnabled;
        Vector3 mDirectionalLightDirection;
        Vector3 mDirectionalLightIntensity;

        EnvironmentMapping mEnvironmentMapping;
    };
} // namespace cube

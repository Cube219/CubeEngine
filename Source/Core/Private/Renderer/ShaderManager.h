#pragma once

#include "CoreHeader.h"

#include "Material.h"

namespace cube
{
    class GAPI;
    class Renderer;
    struct ShaderCreateInfo;
    class Shader;

    class ShaderManager
    {
    public:
        ShaderManager(Renderer& renderer);
        ~ShaderManager() = default;

        void Initialize(GAPI* gapi, bool useDebugMode);
        void Shutdown();

        bool IsUsingDebugMode() const { return mUseDebugMode; }

        SharedPtr<Shader> CreateShader(const ShaderCreateInfo& createInfo);
        void FreeShader(Shader* shader);

        void RecompileShaders(bool forceAll = false);

        MaterialShaderManager& GetMaterialShaderManager() { return mMaterialShaderManager; }

    private:
        friend class Renderer;

        Renderer& mRenderer;
        GAPI* mGAPI;

        bool mUseDebugMode; // Modified in Renderer directly

        Set<Shader*> mCreatedShaders;

        MaterialShaderManager mMaterialShaderManager;
    };
} // namespace cube

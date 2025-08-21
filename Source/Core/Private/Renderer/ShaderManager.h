#pragma once

#include "CoreHeader.h"

namespace cube
{
    struct ShaderCreateInfo;
    class Shader;
    class GAPI;

    namespace gapi
    {
        class Shader;
    } // namespace gapi

    class ShaderManager
    {
    public:
        ShaderManager() = default;
        ~ShaderManager() = default;

        void Initialize(GAPI* gapi);
        void Shutdown();

        SharedPtr<Shader> CreateShader(const ShaderCreateInfo& createInfo);
        void FreeShader(Shader* shader);

    private:
        GAPI* mGAPI;

        Set<Shader*> mCreatedShaders;
    };
} // namespace cube

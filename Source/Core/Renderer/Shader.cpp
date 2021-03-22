#include "Shader.h"

#include "RendererManager.h"

#include "../Resource/ResourceManager.h"

namespace cube
{
    RPtr<Shader> Shader::Load(StringView path)
    {
        RPtr<Shader> shader = ResourceManager::LoadResource<Shader>(path);

        return shader;
    }

    void Shader::_LoadShader(const rapi::ShaderCreateInfo& shaderCreateInfo)
    {
        auto& api = RendererManager::GetRenderAPI();
        mGPUShader = api.CreateShader(shaderCreateInfo);
    }
} // namespace cube

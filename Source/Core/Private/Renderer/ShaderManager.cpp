#include "ShaderManager.h"

#include "Checker.h"
#include "Shader.h"

namespace cube
{
    void ShaderManager::Initialize(GAPI* gapi)
    {
        mGAPI = gapi;
    }

    void ShaderManager::Shutdown()
    {
        CHECK_FORMAT(mCreatedShaders.size() == 0, "Not all shaders are freeed!");
    }

    SharedPtr<Shader> ShaderManager::CreateShader(const ShaderCreateInfo& createInfo)
    {
        SharedPtr<Shader> shader = std::make_shared<Shader>(*this, createInfo);
        mCreatedShaders.insert(shader.get());

        return shader;
    }

    void ShaderManager::FreeShader(Shader* shader)
    {
        auto it = mCreatedShaders.find(shader);
        CHECK(it != mCreatedShaders.end());

        mCreatedShaders.erase(it);
    }
} // namespace cube

#include "Shader.h"

#include "Engine.h"
#include "FileSystem.h"
#include "PathHelper.h"
#include "Renderer.h"

namespace cube
{
    Shader::Shader(ShaderManager& manager, const ShaderCreateInfo& createInfo) :
        mManager(manager)
    {
        SharedPtr<platform::File> shaderFile = platform::FileSystem::OpenFile(createInfo.filePath, platform::FileAccessModeFlag::Read);
        CHECK(shaderFile);
        Uint64 shaderFileSize = shaderFile->GetFileSize();

        Blob shaderCode(shaderFileSize);
        Uint64 readSize = shaderFile->Read(shaderCode.GetData(), shaderFileSize);
        CHECK(readSize <= shaderFileSize);

        mGAPIShader = Engine::GetRenderer()->GetGAPI().CreateShader(
        {
            .type = createInfo.type,
            .language = createInfo.language,
            .fileName = PathHelper::GetFileNameFromPath(createInfo.filePath),
            .path = createInfo.filePath,
            .code = shaderCode,
            .entryPoint = createInfo.entryPoint,
            .withDebugSymbol = true, // TODO: Add option in render ui after implement shader recompilation
            .debugName = createInfo.debugName
        });
        CHECK(mGAPIShader);
    }

    Shader::~Shader()
    {
        mGAPIShader = nullptr;

        mManager.FreeShader(this);
    }
} // namespace cube

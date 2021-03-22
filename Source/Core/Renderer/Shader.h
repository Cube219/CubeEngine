#pragma once

#include "../CoreHeader.h"

#include "Core/Resource/Resource.h"

#include "RenderAPIs/RenderAPI/Interface/Shader.h"

namespace cube
{
    class CORE_EXPORT Shader : public Resource
    {
    public:
        static RPtr<Shader> Load(StringView path);

    public:
        Shader() = default;
        ~Shader() = default;

        rapi::ShaderType GetType() const { return mType; }

        void _LoadShader(const rapi::ShaderCreateInfo& shaderCreateInfo);

    private:
        rapi::ShaderType mType;

        SPtr<rapi::Shader> mGPUShader;
    };
} // namespace cube

#pragma once

#include "CoreHeader.h"

#include "GAPI_Shader.h"

namespace cube
{
    struct ShaderCreateInfo
    {
        gapi::ShaderType type;
        gapi::ShaderLanguage language;

        StringView filePath;

        AnsiStringView entryPoint;

        StringView debugName;
    };

    class ShaderManager;

    class Shader
    {
    public:
        Shader(ShaderManager& manager, const ShaderCreateInfo& createInfo);
        ~Shader();

        SharedPtr<gapi::Shader> GetGAPIShader() const { return mGAPIShader; }

    private:
        friend class ShaderManager;

        ShaderManager& mManager;

        SharedPtr<gapi::Shader> mGAPIShader;
    };

    struct GraphisPipelineCreateInfo
    {};

    class GraphicsPipeline
    {
    public:
    };

    struct ComputePipelineCreateInfo
    {};

    class ComputePipeline
    {
    public:
    };
} // namespace cube

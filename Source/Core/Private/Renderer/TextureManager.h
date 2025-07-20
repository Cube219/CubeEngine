#pragma once

#include "CoreHeader.h"
#include "GAPI_Buffer.h"
#include "Renderer/RenderTypes.h"

namespace cube
{
    class GAPI;
    class TextureResource;

    namespace gapi
    {
        class CommandList;
        class ComputePipeline;
        class Shader;
        class ShaderVariablesLayout;
        class Texture;
    } // namespace gapi

    class TextureManager
    {
    public:
        TextureManager() = default;
        ~TextureManager() = default;

        void Initialize(GAPI* gapi, Uint32 numGPUSync);
        void Shutdown();

        void MoveNextFrame();

        void GenerateMipmaps(SharedPtr<gapi::Texture> texture);

    private:
        GAPI* mGAPI;

        Uint32 mCurrentIndex;

        SharedPtr<gapi::ShaderVariablesLayout> mGenerateMipmapsShaderVariablesLayout;
        SharedPtr<gapi::Shader> mGenerateMipmapsShader;
        SharedPtr<gapi::ComputePipeline> mGenerateMipmapsPipeline;

        // TODO: Adjust padding when copy to gpu buffer
        struct alignas(256) GenerateMipmapsBufferData
        {
            BindlessResource srcTexture;
            BindlessResource dstTexture;
        };
        struct GenerateMipmapBuffer
        {
            GenerateMipmapsBufferData data;
            SharedPtr<gapi::Buffer> buffer;
            Byte* bufferPointer;
        };
        Vector<Vector<GenerateMipmapBuffer>> mGenerateMipmapsBufferList;

        SharedPtr<gapi::CommandList> mCommandList;
    };
} // namespace cube

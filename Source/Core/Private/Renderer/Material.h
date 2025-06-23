#pragma once

#include "CoreHeader.h"

#include "Renderer/RenderTypes.h"
#include "Vector.h"

namespace cube
{
    namespace gapi
    {
        class Sampler;
        class Buffer;
        class Texture;
    } // namespace gapi

    class Material
    {
    public:
        Material(StringView debugName);
        ~Material();

        void SetBaseColor(Vector4 color);
        void SetBaseColorTexture(SharedPtr<gapi::Texture> texture);

        void SetSampler(int samplerIndex);

        SharedPtr<gapi::Buffer> GetMaterialBuffer() const { return mBuffer; }

    private:
        void UpdateBufferData();

        SharedPtr<gapi::Texture> mBaseColorTexture;

        // TODO: Adjust padding when copy to gpu buffer
        struct alignas(256) BufferData
        {
            Vector4 baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
            int useBaseColorTexture = false;
            BindlessResource baseColorTexture;
            int padding;
            BindlessResource sampler; 
        };
        BufferData mBufferData;
        SharedPtr<gapi::Buffer> mBuffer;
        Byte* mBufferDataPointer;
    };
} // namespace cube

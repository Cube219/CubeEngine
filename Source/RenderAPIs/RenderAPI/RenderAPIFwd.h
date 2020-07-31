#pragma once

namespace cube
{
    namespace rapi
    {
        // Interface/Buffer.h
        struct BufferCreateInfo;
        class Buffer;
        struct VertexBufferCreateInfo;
        class VertexBuffer;
        struct IndexBufferCreateInfo;
        class IndexBuffer;
        struct ConstantBufferCreateInfo;
        class ConstantBuffer;
        struct StructuredBufferCreateInfo;
        class StructuredBuffer;

        // Interface/DeviceContext.h
        class DeviceContext;

        // Interface/Resource.h
        struct ResourceCreateInfo;
        class Resource;

        // Interface/Texture.h
        struct TextureCreateInfo;
        class Texture;
        struct Texture2DCreateInfo;
        class Texture2D;
        struct Texture2DArrayCreateInfo;
        class Texture2DArray;
        struct Texture3DCreateInfo;
        class Texture3D;
        struct TextureCubeCreateInfo;
        class TextureCube;
    } // namespace rapi
} // namespace cube

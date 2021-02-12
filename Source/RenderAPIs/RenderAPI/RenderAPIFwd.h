#pragma once

namespace cube
{
    namespace rapi
    {
        // Interface/BlendState.h
        struct BlendState;

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

        // Interface/DepthStencilState.h
        struct DepthStencilState;

        // Interface/DeviceContext.h
        class DeviceContext;

        // Interface/InputLayout.h
        struct InputLayout;

        // Interface/PipelineState.h
        struct GraphicsPipelineStateCreateInfo;
        class GraphicsPipelineState;

        // Interface/RasterizerState.h
        struct RasterizerState;

        // Interface/Resource.h
        struct ResourceCreateInfo;
        class Resource;

        // Interface/Shader.h
        class Shader;

        // Interface/ShaderVariables.h
        struct ShaderVariableInfo;
        class ShaderVariables;
        struct ShaderVariablesLayoutCreateInfo;
        class ShaderVariablesLayout;

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
